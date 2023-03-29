#pragma once

#ifdef SERIALLEDCPPLIB_EXPORTS
#define SERIALLEDCPPLIB_API __declspec(dllexport)
#else
#define SERIALLEDCPPLIB_API __declspec(dllimport)
#endif

#include <Windows.h>
#include <iostream>
#include <string>
#include <thread>
#include <boost/lockfree/queue.hpp>

namespace SerialLedCppLib {

	class SERIALLEDCPPLIB_API LedController {
	public:
		LedController();
		~LedController();

		enum SerialError {
			None,
			PortError,
			UnexpectedError,
			NotConnected,
			Timeouted,
			TransmitionFailed,
			NothingToRead,
			NothingToWrite,
			HandshakeFailed
		};



		//General
		bool Connect(std::string port, uint16_t ledCount, SerialError& err);
		bool Connect(std::string port, uint16_t ledCount);
		bool AutoConnect(uint16_t ledCount);
		bool Disconnect(SerialError& err);
		bool Disconnect();
		bool IsConnected();
		uint16_t GetVersion();

		bool Clear(SerialError& err);
		bool Clear();
		void ClearAsync();
		
		bool StopAllActions(SerialError& err);
		bool StopAllActions();
		void StopAllActionsAsync();

		//TODO: Blocking machen
		bool FinishAllActions(SerialError& err);
		bool FinishAllActions();
		void FinishAllActionsAsync();

		uint16_t GetLedCount(SerialError& err);
		uint16_t GetLedCount();

		uint32_t GetLedColor(uint16_t ledNumber, SerialError& err);
		uint32_t GetLedColor(uint16_t ledNumber);

		////SetStaticColor
		bool SetStaticColor(uint8_t red, uint8_t green, uint8_t blue, SerialError& err);
		bool SetStaticColor(uint8_t red, uint8_t green, uint8_t blue);
		void SetStaticColorAsync(uint8_t red, uint8_t green, uint8_t blue);
		
		bool SetStaticColor(uint16_t ledNumber, uint8_t red, uint8_t green, uint8_t blue, SerialError& err);
		bool SetStaticColor(uint16_t ledNumber, uint8_t red, uint8_t green, uint8_t blue);
		void SetStaticColorAsync(uint16_t ledNumber, uint8_t red, uint8_t green, uint8_t blue);

		bool SetStaticColor(uint16_t ledNumberFrom, uint16_t count, uint8_t red, uint8_t green, uint8_t blue, SerialError& err);
		bool SetStaticColor(uint16_t ledNumberFrom, uint16_t count, uint8_t red, uint8_t green, uint8_t blue);
		void SetStaticColorAsync(uint16_t ledNumberFrom, uint16_t count, uint8_t red, uint8_t green, uint8_t blue);

		////FadeToStaticColor
		bool FadeToStaticColor(uint16_t period, uint8_t red, uint8_t green, uint8_t blue, SerialError& err);
		bool FadeToStaticColor(uint16_t period, uint8_t red, uint8_t green, uint8_t blue);
		void FadeToStaticColorAsync(uint16_t period, uint8_t red, uint8_t green, uint8_t blue);

		bool FadeToStaticColor(uint16_t period, uint16_t ledNumber, uint8_t red, uint8_t green, uint8_t blue, SerialError& err);
		bool FadeToStaticColor(uint16_t period, uint16_t ledNumber, uint8_t red, uint8_t green, uint8_t blue);
		void FadeToStaticColorAsync(uint16_t period, uint16_t ledNumber, uint8_t red, uint8_t green, uint8_t blue);

		bool FadeToStaticColor(uint16_t period, uint16_t ledNumberFrom, uint16_t count, uint8_t red, uint8_t green, uint8_t blue, SerialError& err);
		bool FadeToStaticColor(uint16_t period, uint16_t ledNumberFrom, uint16_t count, uint8_t red, uint8_t green, uint8_t blue);
		void FadeToStaticColorAsync(uint16_t period, uint16_t ledNumberFrom, uint16_t count, uint8_t red, uint8_t green, uint8_t blue);

        
        bool IsFading(SerialError& err);
        bool IsFading();
		bool StopFade(SerialError& err);
		bool StopFade();
		void StopFadeAsync();

		////Gradient
		bool SetGradient(uint8_t redStart, uint8_t greenStart, uint8_t blueStart, uint8_t redEnd, uint8_t greenEnd, uint8_t blueEnd, SerialError& err);
		bool SetGradient(uint8_t redStart, uint8_t greenStart, uint8_t blueStart, uint8_t redEnd, uint8_t greenEnd, uint8_t blueEnd);
		void SetGradientAsync(uint8_t redStart, uint8_t greenStart, uint8_t blueStart, uint8_t redEnd, uint8_t greenEnd, uint8_t blueEnd);

		bool SetGradient(uint16_t ledNumberFrom, uint16_t count, uint8_t redStart, uint8_t greenStart, uint8_t blueStart, uint8_t redEnd, uint8_t greenEnd, uint8_t blueEnd, SerialError& err);
		bool SetGradient(uint16_t ledNumberFrom, uint16_t count, uint8_t redStart, uint8_t greenStart, uint8_t blueStart, uint8_t redEnd, uint8_t greenEnd, uint8_t blueEnd);
		void SetGradientAsync(uint16_t ledNumberFrom, uint16_t count, uint8_t redStart, uint8_t greenStart, uint8_t blueStart, uint8_t redEnd, uint8_t greenEnd, uint8_t blueEnd);

		////FadeToGradient
		bool FadeToGradient(uint16_t period, uint8_t redStart, uint8_t greenStart, uint8_t blueStart, uint8_t redEnd, uint8_t greenEnd, uint8_t blueEnd, SerialError& err);
		bool FadeToGradient(uint16_t period, uint8_t redStart, uint8_t greenStart, uint8_t blueStart, uint8_t redEnd, uint8_t greenEnd, uint8_t blueEnd);
		void FadeToGradientAsync(uint16_t period, uint8_t redStart, uint8_t greenStart, uint8_t blueStart, uint8_t redEnd, uint8_t greenEnd, uint8_t blueEnd);
		
		bool FadeToGradient(uint16_t period, uint16_t ledNumberFrom, uint16_t count, uint8_t redStart, uint8_t greenStart, uint8_t blueStart, uint8_t redEnd, uint8_t greenEnd, uint8_t blueEnd, SerialError& err);
		bool FadeToGradient(uint16_t period, uint16_t ledNumberFrom, uint16_t count, uint8_t redStart, uint8_t greenStart, uint8_t blueStart, uint8_t redEnd, uint8_t greenEnd, uint8_t blueEnd);
		void FadeToGradientAsync(uint16_t period, uint16_t ledNumberFrom, uint16_t count, uint8_t redStart, uint8_t greenStart, uint8_t blueStart, uint8_t redEnd, uint8_t greenEnd, uint8_t blueEnd);

		////Pulse
		bool IsPulsing(SerialError& err);
		bool IsPulsing();

		bool StopPulse(SerialError& err);
		bool StopPulse();
		void StopPulseAsync();

		//TODO: FinishPulseUp und FinishPulseDown Bocking machen -> Event Handling wird benötigt
		bool FinishPulseUp(SerialError& err);
		bool FinishPulseUp();
		void FinishPulseUpAsync();

		bool FinishPulseDown(SerialError& err);
		bool FinishPulseDown();
		void FinishPulseDownAsync();

		bool StartPulse(uint16_t periodUp, uint16_t periodHoldUp, uint16_t periodDown, uint16_t periodHoldDown, uint16_t pulseCount, bool finishPulseUp, uint8_t red, uint8_t green, uint8_t blue, SerialError& err);
		bool StartPulse(uint16_t periodUp, uint16_t periodHoldUp, uint16_t periodDown, uint16_t periodHoldDown, uint16_t pulseCount, bool finishPulseUp, uint8_t red, uint8_t green, uint8_t blue);
		void StartPulseAsync(uint16_t periodUp, uint16_t periodHoldUp, uint16_t periodDown, uint16_t periodHoldDown, uint16_t pulseCount, bool finishPulseUp, uint8_t red, uint8_t green, uint8_t blue);

		bool StartPulse(uint16_t periodUp, uint16_t periodHoldUp, uint16_t periodDown, uint16_t periodHoldDown, uint8_t red, uint8_t green, uint8_t blue, SerialError& err);
		bool StartPulse(uint16_t periodUp, uint16_t periodHoldUp, uint16_t periodDown, uint16_t periodHoldDown, uint8_t red, uint8_t green, uint8_t blue);
		void StartPulseAsync(uint16_t periodUp, uint16_t periodHoldUp, uint16_t periodDown, uint16_t periodHoldDown, uint8_t red, uint8_t green, uint8_t blue);



		////Cycle
		bool IsCycling(SerialError& err);
		bool IsCycling();

		bool StartCycle(uint16_t period, bool direction, SerialError& err);
		bool StartCycle(uint16_t period, bool direction);
		void StartCycleAsync(uint16_t period, bool direction);

		bool StopCycle(SerialError& err);
		bool StopCycle();
		void StopCycleAsync();

		bool FinishCycle(SerialError& err);
		bool FinishCycle();
		void FinishCycleAsync();

	private:
		HANDLE rpiHandle;
		COMSTAT rpiStatus;
		DWORD rpiErrors;

		struct returnValues {
			returnValues() {
				Err = SerialError::UnexpectedError;
				Answer = "";
			}
			returnValues(SerialError err, std::string answer) {
				Err = err;
				Answer = answer;
			}
			SerialError Err;
			std::string Answer;
		};

		struct commandValues {
			commandValues() {
				CommandString = "";
				PutOnErrorStack = false;
			}
			commandValues(std::string commandString, bool putOnErrorStack)
			{
				CommandString = commandString;
				PutOnErrorStack = putOnErrorStack;
			}

			commandValues(std::string commandString)
			{
				CommandString = commandString;
				PutOnErrorStack = false;
			}

			std::string CommandString;
			bool PutOnErrorStack;
		};


		//TODO Wenige Elemente als Standard
		boost::lockfree::queue<commandValues*> commandQueue{ 5 };
		boost::lockfree::queue<returnValues*> returnQueue{ 1 };

		static void rpiThreadFunction(
			bool*,
			boost::lockfree::queue<commandValues*>*,
			boost::lockfree::queue<returnValues*>*,
			HANDLE*,
			COMSTAT*,
			DWORD*,
			LedController*);

		std::thread rpiThread;
		bool rpiThreadActive = false;

		uint16_t version = 0;


		const int timeout = 500;
		const char front_delimiter = '>';
		const char end_delimiter = '\n';

		std::string readRpi(SerialError& err);

		LedController::SerialError handshake();

		std::string stringCommand(std::string command, SerialError& err);
		bool boolCommand(std::string command, SerialError& err);
		uint16_t intCommand(std::string command, SerialError& err);
		uint32_t colorCommand(std::string command, SerialError& err);
		void asyncCommand(std::string command);
	};

}