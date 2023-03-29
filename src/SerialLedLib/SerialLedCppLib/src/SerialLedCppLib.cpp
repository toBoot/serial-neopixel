#include "SerialLedCppLib.h"
#include <codecvt>
#include <chrono>

namespace SerialLedCppLib {

	LedController::LedController() {}


	LedController::~LedController() {
		if (IsConnected())
			Disconnect();
	}

	void LedController::rpiThreadFunction(
		bool* rpiThreadActive,
		boost::lockfree::queue<struct LedController::commandValues*>* commandQueue,
		boost::lockfree::queue<struct LedController::returnValues*>* returnQueue,
		HANDLE* rpiHandle,
		COMSTAT* rpiStatus,
		DWORD* rpiErrors,
		LedController* led){

		*rpiThreadActive = true;

		while (!returnQueue->empty()) {
			struct LedController::returnValues* rV = NULL;
			if(returnQueue->pop(rV))
				delete rV;
		}

		if (commandQueue->empty()) {
			returnQueue->push(new struct LedController::returnValues(LedController::SerialError::NothingToWrite, ""));
			return;
		}

		if (!led->IsConnected()) {
			returnQueue->push(new struct LedController::returnValues(LedController::SerialError::NotConnected, ""));
			return;
		}

		LedController::commandValues* cmd = NULL;
		
		while (!commandQueue->empty()) {
			//ReadExisting
			DWORD bytes_read;
			char inc_msg[1];
			
			ClearCommError(*rpiHandle, rpiErrors, rpiStatus);
			while (rpiStatus->cbInQue > 0) {
				if (!ReadFile(*rpiHandle, inc_msg, 1, &bytes_read, NULL))
					break;
				ClearCommError(*rpiHandle, rpiErrors, rpiStatus);
			}

			if (!commandQueue->pop(cmd)) {
				if (cmd != NULL)
					delete cmd;
				continue;
			}
			//commandStack->pop();
			DWORD bytes_sent;
			unsigned int data_sent_length;

			if (cmd->CommandString != "") {

				data_sent_length = cmd->CommandString.length();

				if (!WriteFile(*rpiHandle, (void*)cmd->CommandString.c_str(), data_sent_length, &bytes_sent, NULL)) {
					ClearCommError(*rpiHandle, rpiErrors, rpiStatus);
					returnQueue->push(new struct LedController::returnValues(LedController::SerialError::NotConnected, ""));
					return;
				}

				if (cmd->PutOnErrorStack) {
					LedController::returnValues* rV = new struct LedController::returnValues(LedController::SerialError::UnexpectedError, "");
					rV->Answer = led->readRpi(rV->Err);
					returnQueue->push(rV);
				}
			}

			delete cmd;

			//Sleep(10);
		}
		
		*rpiThreadActive = false;
	}

	std::string LedController::readRpi(SerialError& err) {
		if (!IsConnected()) {
			err = SerialError::NotConnected;
			return "";
		}

		DWORD bytes_read;
		char inc_msg[1];
		std::string serialBuffer;
		

		bool began = false;
		std::chrono::high_resolution_clock::time_point end_time = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(timeout);

		while(std::chrono::duration_cast<std::chrono::milliseconds>(end_time - std::chrono::high_resolution_clock::now()).count() > 0){
			
			ClearCommError(rpiHandle, &rpiErrors, &rpiStatus);
			if (rpiStatus.cbInQue <= 0)
				continue;


			if (ReadFile(rpiHandle, inc_msg, 1, &bytes_read, NULL)) {
				if (inc_msg[0] == front_delimiter) {
					serialBuffer = "";
					began = true;
				}

				if (began) {
					if (inc_msg[0] == end_delimiter) {
						//TODO: vlt. braucht man sleep doch
						//Sleep(1)
						ClearCommError(rpiHandle, &rpiErrors, &rpiStatus);
						if (rpiStatus.cbInQue <= 0) {
							err = SerialError::None;
							return serialBuffer.substr(0, serialBuffer.length() - 1);
						}
						began = false;
						continue;
					}

					if (inc_msg[0] != front_delimiter)
						serialBuffer.append(inc_msg, 1);
				}

			}
			else {
				err = SerialError::NotConnected;
				return "";
			}
		}
		if (serialBuffer != "") {
			err = SerialError::None;
			return serialBuffer.substr(0, serialBuffer.length() - 1);
		}
		else {
			err = SerialError::TransmitionFailed;
			return "";
		}
	}




	std::string LedController::stringCommand(std::string command, SerialError& err) {
		if (!IsConnected()) {
			err = SerialError::NotConnected;
			return "";
		}

		if (command == "") {
			err = SerialError::NothingToWrite;
			return "";
		}

		if (rpiThreadActive && rpiThread.joinable())
			rpiThread.join();

		//TODO: Keine Ahnung ob das legal ist, je nachdem wie push funktioniert
		//commandStack.push(commandValues(command, true));
		commandQueue.push(new commandValues(command, true));

		//TODO: Keine Ahnung ob der Funktionszeiger legal ist
		rpiThread = std::thread(rpiThreadFunction, &rpiThreadActive, &commandQueue, &returnQueue, &rpiHandle, &rpiStatus, &rpiErrors, this);

		rpiThread.join();

		returnValues* rV = NULL;

		if (!returnQueue.pop(rV)) {
			if (rV != NULL)
				delete rV;

			err = SerialError::UnexpectedError;
			return "";
		}

		err = rV->Err;
		std::string Answer = rV->Answer;
		delete rV;
		return Answer;
	}

	bool LedController::boolCommand(std::string command, SerialError& err) {

		std::string answer = stringCommand(command, err);

		if (err != SerialError::None)
			return false;

		if (answer == "true")
			return true;
		else if (answer == "false")
			return false;
		else {
			err = SerialError::TransmitionFailed;
			return false;
		}
	}

	uint16_t LedController::intCommand(std::string command, SerialError& err) {
		std::string answer = stringCommand(command, err);

		if (err != SerialError::None)
			return 0;

		uint16_t returnInt = 0;


		if (sscanf_s(answer.c_str(), "%hu", &returnInt, answer.length())) 
			return returnInt;

		err = SerialError::TransmitionFailed;
		return 0;
	}

	uint32_t LedController::colorCommand(std::string command, SerialError& err) {
		std::string answer = stringCommand(command, err);

		if (err != SerialError::None)
			return 0;


		uint8_t r = 0, g = 0, b = 0;


		if (sscanf_s(answer.substr(0, 3).c_str(), "%hhu", &r, 3)) {
			if (sscanf_s(answer.substr(4, 3).c_str(), "%hhu", &g, 3)) {
				if (sscanf_s(answer.substr(4, 3).c_str(), "%hhu", &b, 3)) {
					return r*1000000+g*1000+b;
				}
			}
		}

		err = SerialError::TransmitionFailed;
		return 0;
	}

	void LedController::asyncCommand(std::string command) {

		if (!IsConnected() || command == "")
			return;

		if (!commandQueue.push(new commandValues(command)))
			std::cout << "AHHH";

		if (!rpiThreadActive) {
			if (rpiThread.joinable())
				rpiThread.join();
			rpiThread = std::thread(rpiThreadFunction, &rpiThreadActive, &commandQueue, &returnQueue, &rpiHandle, &rpiStatus, &rpiErrors, this);

		}
	}






	


	LedController::SerialError LedController::handshake() {
		if (!IsConnected())
			return SerialError::NotConnected;


		SerialError err = SerialError::UnexpectedError;
		std::string answer = stringCommand(">GetVersion();", err);

		if (err != SerialError::None)
			return err;

		if(answer == "" || answer.length() < 9)
			return SerialError::HandshakeFailed;

		answer = answer.substr(9, answer.length() - 9);

		version = std::stoul(answer.c_str());

		if (version > 0)
			return SerialError::None;
		else
			return SerialError::HandshakeFailed;
	}





	//Public Functions
	//TODO: Warning nachrichten mit return Values ersetzen
	//TODO: Bei Packetverlust muss die Bibliothek auch funktionieren (buffer l�schen nach jedem befehl)
	bool LedController::Connect(std::string port, uint16_t ledCount, SerialError& err){
		if (IsConnected()) {
			err = SerialError::None;
			return true;
		}

		rpiHandle = CreateFileA(static_cast<LPCSTR>(port.c_str()),
			GENERIC_READ | GENERIC_WRITE,
			0,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

		if (rpiHandle == INVALID_HANDLE_VALUE) {
			err = SerialError::PortError;
			return false;
		}
		else {
			DCB dcbSerialParams = { 0 };

			if (!GetCommState(rpiHandle, &dcbSerialParams)) {
				err = SerialError::PortError;
				return false;
				//std::cout << "Warning: Failed to get current serial params\n";
			}
			else {
				dcbSerialParams.BaudRate = 115200;
				dcbSerialParams.ByteSize = 8;
				dcbSerialParams.StopBits = ONESTOPBIT;
				dcbSerialParams.Parity = NOPARITY;
				dcbSerialParams.fDtrControl = DTR_CONTROL_ENABLE;
			}

			if (!SetCommState(rpiHandle, &dcbSerialParams)) {
				err = SerialError::PortError;
				return false;
			}
			else {
				PurgeComm(rpiHandle, PURGE_RXCLEAR | PURGE_TXCLEAR);
				err = SerialError::None;
			}
		}

		//ReadExisting
		DWORD bytes_read;
		char inc_msg[1];
		ClearCommError(rpiHandle, &rpiErrors, &rpiStatus);
		while (rpiStatus.cbInQue > 0) {
			if(!ReadFile(rpiHandle, inc_msg, 1, &bytes_read, NULL))
				break;
			ClearCommError(rpiHandle, &rpiErrors, &rpiStatus);
		}

		err = handshake();

		if (err != SerialError::None || !boolCommand(">SetLedCount("+ std::to_string(ledCount) + ");", err)){
			Disconnect();
			return false;
		}
		else {
			return true;
		}
	}

	bool LedController::Connect(std::string port, uint16_t ledCount) {
		SerialError err;
		return Connect(port, ledCount, err);
	}


	bool LedController::AutoConnect(uint16_t ledCount) {
		
		wchar_t lpTargetPath[500];

		std::string portString;
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		std::wstring wide;
		LPCWSTR port;

		for (int i = 0; i <= 255; i++)
		{

			portString = "COM" + std::to_string(i);
			wide = converter.from_bytes(portString);
			port = wide.c_str();
			DWORD test = QueryDosDevice(port, lpTargetPath, 500);

			if (test != 0 && Connect(portString, ledCount)) {
				return true;
			}
		}
		return false;
	}


	bool LedController::Disconnect(SerialError& err) {
		if (IsConnected()) {

			if (rpiThreadActive && rpiThread.joinable())
				rpiThread.join();

			CloseHandle(rpiHandle);
			err = SerialError::None;
			return true;
		}
		else {
			err = SerialError::NotConnected;
			return false;
		}
		
	}
	bool LedController::Disconnect() {
		SerialError err;
		return Disconnect(err);
	}



	bool LedController::IsConnected() {
		DCB dcbSerialParams = {0};
		return GetCommState(rpiHandle, &dcbSerialParams);
	}

	uint16_t LedController::GetVersion() {
		return version;
	}






	bool LedController::Clear(SerialError& err) {
		return boolCommand(">Clear();", err);
	}
	bool LedController::Clear() {
		SerialError err;
		return Clear(err);
	}
	void LedController::ClearAsync() {
		asyncCommand(">Clear();");
	}



	bool LedController::StopAllActions(SerialError& err) {
		return boolCommand(">StopAllActions();", err);
	}
	bool LedController::StopAllActions() {
		SerialError err;
		return StopAllActions(err);
	}
	void LedController::StopAllActionsAsync() {
		asyncCommand(">StopAllActions();");
	}



	bool LedController::FinishAllActions(SerialError& err) {
		return boolCommand(">FinishAllActions();", err);
	}
	bool LedController::FinishAllActions() {
		SerialError err;
		return StopAllActions(err);
	}
	void LedController::FinishAllActionsAsync() {
		asyncCommand(">StopAllActions();");
	}




	uint16_t LedController::GetLedCount(SerialError& err) {
		return intCommand(">GetLedCount();", err);
	}
	uint16_t LedController::GetLedCount() {
		SerialError err;
		return GetLedCount(err);
	}
	


	uint32_t LedController::GetLedColor(uint16_t ledNumber, SerialError& err) {
		return colorCommand(">GetLedColor(" + std::to_string(ledNumber) + ");", err);
	}
	uint32_t LedController::GetLedColor(uint16_t ledNumber) {
		SerialError err;
		return GetLedColor(ledNumber, err);
	}




	////SetStaticColor
	bool LedController::SetStaticColor(uint8_t red, uint8_t green, uint8_t blue, SerialError& err) {
		return boolCommand(">SetStaticColor("
			+ std::to_string(red)
			+ "," 
			+ std::to_string(green) 
			+ "," 
			+ std::to_string(blue) 
			+ ");", err);
	}
	bool LedController::SetStaticColor(uint8_t red, uint8_t green, uint8_t blue) {
		SerialError err;
		return SetStaticColor(red, green, blue, err);
	}
	void LedController::SetStaticColorAsync(uint8_t red, uint8_t green, uint8_t blue) {
		asyncCommand(">SetStaticColor("
			+ std::to_string(red)
			+ ","
			+ std::to_string(green)
			+ ","
			+ std::to_string(blue)
			+ ");");
	}


	bool LedController::SetStaticColor(uint16_t ledNumber, uint8_t red, uint8_t green, uint8_t blue, SerialError& err) {
		return boolCommand(">SetStaticColor("
			+ std::to_string(ledNumber)
			+ ","
			+ std::to_string(red)
			+ ","
			+ std::to_string(green)
			+ ","
			+ std::to_string(blue)
			+ ");", err);
	}
	bool LedController::SetStaticColor(uint16_t ledNumber, uint8_t red, uint8_t green, uint8_t blue) {
		SerialError err;
		return SetStaticColor(ledNumber, red, green, blue, err);
	}
	void LedController::SetStaticColorAsync(uint16_t ledNumber, uint8_t red, uint8_t green, uint8_t blue) {
		asyncCommand(">SetStaticColor("
			+ std::to_string(ledNumber)
			+ ","
			+ std::to_string(red)
			+ ","
			+ std::to_string(green)
			+ ","
			+ std::to_string(blue)
			+ ");");
	}


	bool LedController::SetStaticColor(uint16_t ledNumberFrom, uint16_t count, uint8_t red, uint8_t green, uint8_t blue, SerialError& err) {
		return boolCommand(">SetStaticColor("
			+ std::to_string(ledNumberFrom)
			+ ","
			+ std::to_string(count)
			+ ","
			+ std::to_string(red)
			+ ","
			+ std::to_string(green)
			+ "," 
			+ std::to_string(blue)
			+ ");", err);
	}
	bool LedController::SetStaticColor(uint16_t ledNumberFrom, uint16_t count, uint8_t red, uint8_t green, uint8_t blue) {
		SerialError err;
		return SetStaticColor(ledNumberFrom, count, red, green, blue, err);
	}
	void LedController::SetStaticColorAsync(uint16_t ledNumberFrom, uint16_t count, uint8_t red, uint8_t green, uint8_t blue) {
		asyncCommand(">SetStaticColor("
			+ std::to_string(ledNumberFrom)
			+ ","
			+ std::to_string(count)
			+ ","
			+ std::to_string(red)
			+ ","
			+ std::to_string(green)
			+ ","
			+ std::to_string(blue)
			+ ");");
	}

    
    
    bool LedController::IsFading(SerialError& err){
        return boolCommand(">IsFading();", err);
    }
    
    bool LedController::IsFading(){
        SerialError err;
        return IsFading(err);
    }



	////FadeToStaticColor
	bool LedController::FadeToStaticColor(uint16_t period, uint8_t red, uint8_t green, uint8_t blue, SerialError& err) {
		return boolCommand(">FadeToStaticColor("
			+ std::to_string(period)
			+ ","
			+ std::to_string(red)
			+ ","
			+ std::to_string(green)
			+ ","
			+ std::to_string(blue)
			+ ");", err);
	}
	bool LedController::FadeToStaticColor(uint16_t period, uint8_t red, uint8_t green, uint8_t blue) {
		SerialError err;
		return FadeToStaticColor(period, red, green, blue, err);
	}
	void LedController::FadeToStaticColorAsync(uint16_t period, uint8_t red, uint8_t green, uint8_t blue) {
		asyncCommand(">FadeToStaticColor("
			+ std::to_string(period)
			+ ","
			+ std::to_string(red)
			+ ","
			+ std::to_string(green)
			+ ","
			+ std::to_string(blue)
			+ ");");
	}


	bool LedController::FadeToStaticColor(uint16_t period, uint16_t ledNumber, uint8_t red, uint8_t green, uint8_t blue, SerialError& err) {
		return boolCommand(">FadeToStaticColor("
			+ std::to_string(period)
			+ ","
			+ std::to_string(ledNumber)
			+ ","
			+ std::to_string(red)
			+ ","
			+ std::to_string(green)
			+ ","
			+ std::to_string(blue)
			+ ");", err);
	}
	bool LedController::FadeToStaticColor(uint16_t period, uint16_t ledNumber, uint8_t red, uint8_t green, uint8_t blue) {
		SerialError err;
		return FadeToStaticColor(period, ledNumber, red, green, blue, err);
	}
	void LedController::FadeToStaticColorAsync(uint16_t period, uint16_t ledNumber, uint8_t red, uint8_t green, uint8_t blue) {
		asyncCommand(">FadeToStaticColor("
			+ std::to_string(period)
			+ ","
			+ std::to_string(ledNumber)
			+ ","
			+ std::to_string(red)
			+ ","
			+ std::to_string(green)
			+ ","
			+ std::to_string(blue)
			+ ");");
	}




	bool LedController::FadeToStaticColor(uint16_t period, uint16_t ledNumberFrom, uint16_t count, uint8_t red, uint8_t green, uint8_t blue, SerialError& err) {
		return boolCommand(">FadeToStaticColor("
			+ std::to_string(period)
			+ ","
			+ std::to_string(ledNumberFrom)
			+ ","
			+ std::to_string(count)
			+ ","
			+ std::to_string(red)
			+ ","
			+ std::to_string(green)
			+ ","
			+ std::to_string(blue)
			+ ");", err);
	}
	bool LedController::FadeToStaticColor(uint16_t period, uint16_t ledNumberFrom, uint16_t count, uint8_t red, uint8_t green, uint8_t blue) {
		SerialError err;
		return FadeToStaticColor(period, ledNumberFrom, count, red, green, blue, err);
	}
	void LedController::FadeToStaticColorAsync(uint16_t period, uint16_t ledNumberFrom, uint16_t count, uint8_t red, uint8_t green, uint8_t blue) {
		asyncCommand(">FadeToStaticColor("
			+ std::to_string(period)
			+ ","
			+ std::to_string(ledNumberFrom)
			+ ","
			+ std::to_string(count)
			+ ","
			+ std::to_string(red)
			+ ","
			+ std::to_string(green)
			+ ","
			+ std::to_string(blue)
			+ ");");
	}




	bool LedController::StopFade(SerialError& err) {
		return boolCommand(">StopFade();", err);
	}
	bool LedController::StopFade() {
		SerialError err;
		return StopFade(err);
	}
	void LedController::StopFadeAsync() {
		asyncCommand(">StopFade();");
	}



	bool LedController::SetGradient(uint8_t redStart, uint8_t greenStart, uint8_t blueStart, uint8_t redEnd, uint8_t greenEnd, uint8_t blueEnd, SerialError& err) {
		return boolCommand(">SetGradient("
			+ std::to_string(redStart)
			+ ","
			+ std::to_string(greenStart)
			+ ","
			+ std::to_string(blueStart)
			+ ","
			+ std::to_string(redEnd)
			+ ","
			+ std::to_string(greenEnd)
			+ ","
			+ std::to_string(blueEnd)
			+ ");", err);
	}
	bool LedController::SetGradient(uint8_t redStart, uint8_t greenStart, uint8_t blueStart, uint8_t redEnd, uint8_t greenEnd, uint8_t blueEnd) {
		SerialError err;
		return SetGradient(redStart, greenStart, blueStart, redEnd, greenEnd, blueEnd, err);
	}
	void LedController::SetGradientAsync(uint8_t redStart, uint8_t greenStart, uint8_t blueStart, uint8_t redEnd, uint8_t greenEnd, uint8_t blueEnd) {
		asyncCommand(">SetGradient("
			+ std::to_string(redStart)
			+ ","
			+ std::to_string(greenStart)
			+ ","
			+ std::to_string(blueStart)
			+ ","
			+ std::to_string(redEnd)
			+ ","
			+ std::to_string(greenEnd)
			+ ","
			+ std::to_string(blueEnd)
			+ ");");
	}





	bool LedController::SetGradient(uint16_t ledNumberFrom, uint16_t count, uint8_t redStart, uint8_t greenStart, uint8_t blueStart, uint8_t redEnd, uint8_t greenEnd, uint8_t blueEnd, SerialError& err) {
		return boolCommand(">SetGradient("
			+ std::to_string(ledNumberFrom)
			+ ","
			+ std::to_string(count)
			+ ","
			+ std::to_string(redStart)
			+ ","
			+ std::to_string(greenStart)
			+ ","
			+ std::to_string(blueStart)
			+ ","
			+ std::to_string(redEnd)
			+ ","
			+ std::to_string(greenEnd)
			+ ","
			+ std::to_string(blueEnd)
			+ ");", err);
	}
	bool LedController::SetGradient(uint16_t ledNumberFrom, uint16_t count, uint8_t redStart, uint8_t greenStart, uint8_t blueStart, uint8_t redEnd, uint8_t greenEnd, uint8_t blueEnd) {
		SerialError err;
		return SetGradient(ledNumberFrom, count, redStart, greenStart, blueStart, redEnd, greenEnd, blueEnd, err);
	}
	void LedController::SetGradientAsync(uint16_t ledNumberFrom, uint16_t count, uint8_t redStart, uint8_t greenStart, uint8_t blueStart, uint8_t redEnd, uint8_t greenEnd, uint8_t blueEnd) {
		asyncCommand(">SetGradient("
			+ std::to_string(ledNumberFrom)
			+ ","
			+ std::to_string(count)
			+ ","
			+ std::to_string(redStart)
			+ ","
			+ std::to_string(greenStart)
			+ ","
			+ std::to_string(blueStart)
			+ ","
			+ std::to_string(redEnd)
			+ ","
			+ std::to_string(greenEnd)
			+ ","
			+ std::to_string(blueEnd)
			+ ");");
	}





	bool LedController::FadeToGradient(uint16_t period, uint8_t redStart, uint8_t greenStart, uint8_t blueStart, uint8_t redEnd, uint8_t greenEnd, uint8_t blueEnd, SerialError& err) {
		return boolCommand(">FadeToGradient("
			+ std::to_string(period)
			+ ","
			+ std::to_string(redStart)
			+ ","
			+ std::to_string(greenStart)
			+ ","
			+ std::to_string(blueStart)
			+ ","
			+ std::to_string(redEnd)
			+ ","
			+ std::to_string(greenEnd)
			+ ","
			+ std::to_string(blueEnd)
			+ ");", err);
	}
	bool LedController::FadeToGradient(uint16_t period, uint8_t redStart, uint8_t greenStart, uint8_t blueStart, uint8_t redEnd, uint8_t greenEnd, uint8_t blueEnd) {
		SerialError err;
		return FadeToGradient(period, redStart, greenStart, blueStart, redEnd, greenEnd, blueEnd, err);
	}
	void LedController::FadeToGradientAsync(uint16_t period, uint8_t redStart, uint8_t greenStart, uint8_t blueStart, uint8_t redEnd, uint8_t greenEnd, uint8_t blueEnd) {
		asyncCommand(">FadeToGradient("
			+ std::to_string(period)
			+ ","
			+ std::to_string(redStart)
			+ ","
			+ std::to_string(greenStart)
			+ ","
			+ std::to_string(blueStart)
			+ ","
			+ std::to_string(redEnd)
			+ ","
			+ std::to_string(greenEnd)
			+ ","
			+ std::to_string(blueEnd)
			+ ");");
	}




	bool LedController::FadeToGradient(uint16_t period, uint16_t ledNumberFrom, uint16_t count, uint8_t redStart, uint8_t greenStart, uint8_t blueStart, uint8_t redEnd, uint8_t greenEnd, uint8_t blueEnd, SerialError& err) {
		return boolCommand(">FadeToGradient("
			+ std::to_string(period)
			+ ","
			+ std::to_string(ledNumberFrom)
			+ ","
			+ std::to_string(count)
			+ ","
			+ std::to_string(redStart)
			+ ","
			+ std::to_string(greenStart)
			+ ","
			+ std::to_string(blueStart)
			+ ","
			+ std::to_string(redEnd)
			+ ","
			+ std::to_string(greenEnd)
			+ ","
			+ std::to_string(blueEnd)
			+ ");", err);
	}
	bool LedController::FadeToGradient(uint16_t period, uint16_t ledNumberFrom, uint16_t count, uint8_t redStart, uint8_t greenStart, uint8_t blueStart, uint8_t redEnd, uint8_t greenEnd, uint8_t blueEnd) {
		SerialError err;
		return FadeToGradient(period, ledNumberFrom, count, redStart, greenStart, blueStart, redEnd, greenEnd, blueEnd, err);
	}
	void LedController::FadeToGradientAsync(uint16_t period, uint16_t ledNumberFrom, uint16_t count, uint8_t redStart, uint8_t greenStart, uint8_t blueStart, uint8_t redEnd, uint8_t greenEnd, uint8_t blueEnd) {
		asyncCommand(">FadeToGradient("
			+ std::to_string(period)
			+ ","
			+ std::to_string(ledNumberFrom)
			+ ","
			+ std::to_string(count)
			+ ","
			+ std::to_string(redStart)
			+ ","
			+ std::to_string(greenStart)
			+ ","
			+ std::to_string(blueStart)
			+ ","
			+ std::to_string(redEnd)
			+ ","
			+ std::to_string(greenEnd)
			+ ","
			+ std::to_string(blueEnd)
			+ ");");
	}


	
	bool LedController::IsPulsing(SerialError& err) {
		return boolCommand(">IsPulsing();", err);
	}
	bool LedController::IsPulsing() {
		SerialError err;
		return IsPulsing(err);
	}



	bool LedController::StopPulse(SerialError& err) {
		return boolCommand(">StopPulse();", err);
	}

	bool LedController::StopPulse() {
		SerialError err;
		return StopPulse(err);
	}

	void LedController::StopPulseAsync() {
		asyncCommand(">StopPulse();");
	}

	
	bool LedController::FinishPulseUp(SerialError & err) {
		return boolCommand(">FinishPulseUp();", err);
	}
	bool LedController::FinishPulseUp() {
		SerialError err;
		return FinishPulseUp(err);
	}
	void LedController::FinishPulseUpAsync() {
		asyncCommand(">FinishPulseUp();");
	}

	bool LedController::FinishPulseDown(SerialError& err) {
		return boolCommand(">FinishPulseDown();", err);
	}
	bool LedController::FinishPulseDown() {
		SerialError err;
		return FinishPulseDown(err);
	}
	void LedController::FinishPulseDownAsync() {
		asyncCommand(">FinishPulseDown();");
	}


	bool LedController::StartPulse(uint16_t periodUp, uint16_t periodHoldUp, uint16_t periodDown, uint16_t periodHoldDown, uint16_t pulseCount, bool finishPulseUp, uint8_t red, uint8_t green, uint8_t blue, SerialError& err) {
		std::string finishPulseUpString;
		if (finishPulseUp)
			finishPulseUpString = "true";
		else
			finishPulseUpString = "false";
		
		return boolCommand(">StartPulse("
			+ std::to_string(periodUp)
			+ ","
			+ std::to_string(periodHoldUp)
			+ ","
			+ std::to_string(periodDown)
			+ ","
			+ std::to_string(periodHoldDown)
			+ ","
			+ std::to_string(pulseCount)
			+ ","
			+ finishPulseUpString
			+ ","
			+ std::to_string(red)
			+ ","
			+ std::to_string(green)
			+ ","
			+ std::to_string(blue)
			+ ");", err);
	}

	bool LedController::StartPulse(uint16_t periodUp, uint16_t periodHoldUp, uint16_t periodDown, uint16_t periodHoldDown, uint16_t pulseCount, bool finishPulseUp, uint8_t red, uint8_t green, uint8_t blue) {
		SerialError err;
		return StartPulse(periodUp, periodHoldUp, periodDown, periodHoldDown, pulseCount, finishPulseUp, red, green, blue, err);
	}
	void LedController::StartPulseAsync(uint16_t periodUp, uint16_t periodHoldUp, uint16_t periodDown, uint16_t periodHoldDown, uint16_t pulseCount, bool finishPulseUp, uint8_t red, uint8_t green, uint8_t blue) {
		std::string finishPulseUpString;
		if (finishPulseUp)
			finishPulseUpString = "true";
		else
			finishPulseUpString = "false";
		
		asyncCommand(">StartPulse("
			+ std::to_string(periodUp)
			+ ","
			+ std::to_string(periodHoldUp)
			+ ","
			+ std::to_string(periodDown)
			+ ","
			+ std::to_string(periodHoldDown)
			+ ","
			+ std::to_string(pulseCount)
			+ ","
			+ finishPulseUpString
			+ ","
			+ std::to_string(red)
			+ ","
			+ std::to_string(green)
			+ ","
			+ std::to_string(blue)
			+ ");");
	}


	bool LedController::StartPulse(uint16_t periodUp, uint16_t periodHoldUp, uint16_t periodDown, uint16_t periodHoldDown, uint8_t red, uint8_t green, uint8_t blue, SerialError& err) {
		return StartPulse(periodUp, periodHoldUp, periodDown, periodHoldDown, 0, true, red, green, blue, err);
	}
	bool LedController::StartPulse(uint16_t periodUp, uint16_t periodHoldUp, uint16_t periodDown, uint16_t periodHoldDown, uint8_t red, uint8_t green, uint8_t blue) {
		SerialError err;
		return StartPulse(periodUp, periodHoldUp, periodDown, periodHoldDown, 0, true, red, green, blue, err);
	}
	void LedController::StartPulseAsync(uint16_t periodUp, uint16_t periodHoldUp, uint16_t periodDown, uint16_t periodHoldDown, uint8_t red, uint8_t green, uint8_t blue) {
		StartPulseAsync(periodUp, periodHoldDown, periodDown, periodHoldDown, 0, true, red, green, blue);
	}


	bool LedController::IsCycling(SerialError& err) {
		return boolCommand(">IsCycling();", err);
	}
	bool LedController::IsCycling() {
		SerialError err;
		return IsCycling(err);
	}



	bool LedController::StartCycle(uint16_t period, bool direction, SerialError& err) {
		std::string w = ">StartCycle("
			+ std::to_string(period)
			+ ",";

		if (direction)
			w.append("true);");
		else
			w.append("false);");

		return boolCommand(w, err);
	}
	bool LedController::StartCycle(uint16_t period, bool direction) {
		SerialError err;
		return StartCycle(period, direction, err);
	}
	void LedController::StartCycleAsync(uint16_t period, bool direction) {
		std::string w = ">StartCycle("
			+ std::to_string(period)
			+ ",";

		if (direction)
			w.append("true);");
		else
			w.append("false);");

		asyncCommand(w);
	}



	bool LedController::StopCycle(SerialError& err) {
		return boolCommand(">StopCycle();", err);
	}
	bool LedController::StopCycle() {
		SerialError err;
		return StopCycle(err);
	}
	void LedController::StopCycleAsync() {
		asyncCommand(">StopCycle();");
	}



	bool LedController::FinishCycle(SerialError& err) {
		return boolCommand(">FinishCycle();", err);
	}
	bool LedController::FinishCycle() {
		SerialError err;
		return FinishCycle(err);
	}
	void LedController::FinishCycleAsync() {
		asyncCommand(">FinishCycle();");
	}
}