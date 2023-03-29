//
// Created by Tobias on 17.07.2021.
//

#include <pico/stdlib.h.>
#include <stdio.h>
#include <pico/multicore.h>
#include <iostream>

#include "../include/SerialController.h"


void SerialController::Init(LedController* led) {
	ledController = led;

}

void SerialController::Update() {

	updateSerialInput();

	updateFunctionType();

	callFunctions();

	serialWrite();
}

void SerialController::updateSerialInput() {
	serialOutput = "";

	if(!multicore_fifo_rvalid())
		return;

	string* tempInput = (string*)multicore_fifo_pop_blocking();
	serialInput = *tempInput;

	delete tempInput;

	newData = true;
}

void SerialController::updateFunctionType() {
	if(!newData)
		return;

	uint8_t commaCount = getCommaCount(serialInput);
	int index;


	if(serialInput.find("GetVersion(") == 0){
		serialInput = serialInput.substr(11, serialInput.length() - 1);
		functionType = functionType::GetVersion;
	}
	else if(serialInput.find("Clear(") == 0){
		functionType = functionType::Clear;
		serialInput = serialInput.substr(6, serialInput.length() - 1);
	}
	else if(serialInput.find("StopAllActions(") == 0){
		functionType = functionType::StopAllActions;
		serialInput = serialInput.substr(15, serialInput.length() - 1);
	}
	else if(serialInput.find("FinishAllActions(") == 0){
		functionType = functionType::FinishAllAction;
		serialInput = serialInput.substr(17, serialInput.length() - 1);
	}
	else if(serialInput.find("GetLedCount(") == 0){
		functionType = functionType::GetLedCount;
		serialInput = serialInput.substr(13, serialInput.length() - 1);
	}
	else if(serialInput.find("SetLedCount(") == 0){
		serialInput = serialInput.substr(12, serialInput.length()-1);

		if(commaCount != 0){
			functionType = functionType::None;
			return;
		}
		functionType = functionType::SetLedCount;
		uint16_1 = StringToUint16_t(serialInput);
	}
	else if(serialInput.find("GetLedColor(") == 0){
		serialInput = serialInput.substr(12, serialInput.length() - 1);
		if(commaCount != 0){
			functionType = functionType::None;
			return;
		}

		functionType = functionType::GetLedColor;
		uint16_1 = StringToUint16_t(serialInput);
	}
	else if(serialInput.find("SetStaticColor(") == 0){
		serialInput = serialInput.substr(15, serialInput.length() - 1);

		if(commaCount == 2)
		{
			functionType = functionType::SetStaticColor1;
			rgb_1 = StringToRgb(serialInput);
		}
		else if(commaCount == 3){
			functionType = functionType::SetStaticColor2;

			index = serialInput.find(',');
			uint16_1 = StringToUint16_t(serialInput.substr(0, index));
			serialInput.erase(0, index+1);

			rgb_1 = StringToRgb(serialInput);
		} else if(commaCount == 4){
			functionType = functionType::SetStaticColor3;

			index = serialInput.find(',');
			uint16_1 = StringToUint16_t(serialInput.substr(0, index));
			serialInput.erase(0, index+1);

			index = serialInput.find(',');
			uint16_2 = StringToUint16_t(serialInput.substr(0, index));
			serialInput.erase(0, index+1);

			rgb_1 = StringToRgb(serialInput);
		}
		else{
			functionType = functionType::None;
		}
	}
	else if(serialInput.find("IsFading(") == 0){
		functionType = functionType::IsFading;
		serialInput = serialInput.substr(9, serialInput.length() - 1);
	}
	else if(serialInput.find("FadeToStaticColor(") == 0){
		serialInput = serialInput.substr(18, serialInput.length() - 1);

		if(commaCount == 3)
		{
			functionType = functionType::FadeToStaticColor1;

			index = serialInput.find(',');
			uint16_1 = StringToUint16_t(serialInput.substr(0, index));
			serialInput.erase(0, index+1);

			rgb_1 = StringToRgb(serialInput);
		}
		else if(commaCount == 4){
			functionType = functionType::FadeToStaticColor2;

			index = serialInput.find(',');
			uint16_1 = StringToUint16_t(serialInput.substr(0, index));
			serialInput.erase(0, index+1);

			index = serialInput.find(',');
			uint16_2 = StringToUint16_t(serialInput.substr(0, index));
			serialInput.erase(0, index+1);

			rgb_1 = StringToRgb(serialInput);
		} else if(commaCount == 5){
			functionType = functionType::FadeToStaticColor3;

			index = serialInput.find(',');
			uint16_1 = StringToUint16_t(serialInput.substr(0, index));
			serialInput.erase(0, index+1);

			index = serialInput.find(',');
			uint16_2 = StringToUint16_t(serialInput.substr(0, index));
			serialInput.erase(0, index+1);

			index = serialInput.find(',');
			uint16_3 = StringToUint16_t(serialInput.substr(0, index));
			serialInput.erase(0, index+1);

			rgb_1 = StringToRgb(serialInput);
		}
		else{
			functionType = functionType::None;
		}
	}
	else if(serialInput.find("StopFade(") == 0){
		functionType = functionType::StopFade;
		serialInput = serialInput.substr(9, serialInput.length() - 1);
	}
	else if(serialInput.find("SetGradient(") == 0){
		serialInput = serialInput.substr(12, serialInput.length() - 1);

		if(commaCount == 5){
			functionType = functionType::SetGradient1;

			rgb_1 = StringToRgb(serialInput);


			index = serialInput.find(',');
			serialInput.erase(0, index+1);
			index = serialInput.find(',');
			serialInput.erase(0, index+1);
			index = serialInput.find(',');
			serialInput.erase(0, index+1);

			rgb_2 = StringToRgb(serialInput);
		}
		else if(commaCount == 7){
			functionType = functionType::SetGradient2;

			index = serialInput.find(',');
			uint16_1 = StringToUint16_t(serialInput.substr(0, index));
			serialInput.erase(0, index+1);

			index = serialInput.find(',');
			uint16_2 = StringToUint16_t(serialInput.substr(0, index));
			serialInput.erase(0, index+1);

			rgb_1 = StringToRgb(serialInput);


			index = serialInput.find(',');
			serialInput.erase(0, index+1);
			index = serialInput.find(',');
			serialInput.erase(0, index+1);
			index = serialInput.find(',');
			serialInput.erase(0, index+1);

			rgb_2 = StringToRgb(serialInput);
		} else{
			functionType::None;
		}
	}
	else if(serialInput.find("FadeToGradient(") == 0){
		serialInput = serialInput.substr(15, serialInput.length() - 1);

		if(commaCount == 6){
			functionType = functionType::FadeToGradient1;

			index = serialInput.find(',');
			uint16_1 = StringToUint16_t(serialInput.substr(0, index));
			serialInput.erase(0, index+1);

			rgb_1 = StringToRgb(serialInput);


			index = serialInput.find(',');
			serialInput.erase(0, index+1);
			index = serialInput.find(',');
			serialInput.erase(0, index+1);
			index = serialInput.find(',');
			serialInput.erase(0, index+1);

			rgb_2 = StringToRgb(serialInput);
		} else if(commaCount == 8){
			functionType = functionType::FadeToGradient2;

			index = serialInput.find(',');
			uint16_1 = StringToUint16_t(serialInput.substr(0, index));
			serialInput.erase(0, index+1);

			index = serialInput.find(',');
			uint16_2 = StringToUint16_t(serialInput.substr(0, index));
			serialInput.erase(0, index+1);

			index = serialInput.find(',');
			uint16_3= StringToUint16_t(serialInput.substr(0, index));
			serialInput.erase(0, index+1);

			rgb_1 = StringToRgb(serialInput);


			index = serialInput.find(',');
			serialInput.erase(0, index+1);
			index = serialInput.find(',');
			serialInput.erase(0, index+1);
			index = serialInput.find(',');
			serialInput.erase(0, index+1);

			rgb_2 = StringToRgb(serialInput);
		} else{
			functionType = functionType::None;
		}
	}
    else if(serialInput.find("IsPulsing(") == 0){
        functionType = functionType::IsPulsing;
        serialInput = serialInput.substr(10, serialInput.length() - 1);
    }
    else if(serialInput.find("StopPulse(") == 0){
        functionType = functionType::StopPulse;
        serialInput = serialInput.substr(10, serialInput.length() - 1);
    }
    else if(serialInput.find("FinishPulseUp(") == 0){
        functionType = functionType::FinishPulseUp;
        serialInput = serialInput.substr(14, serialInput.length() - 1);
    }
    else if(serialInput.find("FinishPulseDown(") == 0){
        functionType = functionType::FinishPulseDown;
        serialInput = serialInput.substr(16, serialInput.length() - 1);
    }
    else if(serialInput.find("StartPulse(") == 0){
        serialInput = serialInput.substr(11, serialInput.length() - 1);
        if(commaCount == 8){
            functionType = functionType::StartPulse1;

            index = serialInput.find(',');
            uint16_1 = StringToUint16_t(serialInput.substr(0, index));
            serialInput.erase(0, index+1);

            index = serialInput.find(',');
            uint16_2 = StringToUint16_t(serialInput.substr(0, index));
            serialInput.erase(0, index+1);

            index = serialInput.find(',');
            uint16_3= StringToUint16_t(serialInput.substr(0, index));
            serialInput.erase(0, index+1);

            index = serialInput.find(',');
            uint16_4= StringToUint16_t(serialInput.substr(0, index));
            serialInput.erase(0, index+1);

            index = serialInput.find(',');
            uint16_5= StringToUint16_t(serialInput.substr(0, index));
            serialInput.erase(0, index+1);


            if (serialInput.find("true") == 0){
                bool_1 = true;
                serialInput.erase(0, 5);
            }
            else if(serialInput.find("false") == 0){
                bool_1 = false;
                serialInput.erase(0, 6);
            } else{
                functionType = functionType::None;
            }

            rgb_1 = StringToRgb(serialInput);
        } else if(commaCount == 6){
            functionType = functionType::StartPulse2;

            index = serialInput.find(',');
            uint16_1 = StringToUint16_t(serialInput.substr(0, index));
            serialInput.erase(0, index+1);

            index = serialInput.find(',');
            uint16_2 = StringToUint16_t(serialInput.substr(0, index));
            serialInput.erase(0, index+1);

            index = serialInput.find(',');
            uint16_3= StringToUint16_t(serialInput.substr(0, index));
            serialInput.erase(0, index+1);

            index = serialInput.find(',');
            uint16_4= StringToUint16_t(serialInput.substr(0, index));
            serialInput.erase(0, index+1);

            rgb_1 = StringToRgb(serialInput);
        } else{
            functionType = functionType::None;
        }
    }
	else if(serialInput.find("IsCycling(") == 0){
		functionType = functionType::IsCycling;
		serialInput = serialInput.substr(10, serialInput.length() - 1);
	}
	else if(serialInput.find("StopCycle(") == 0){
		functionType = functionType::StopCycle;
		serialInput = serialInput.substr(10, serialInput.length() - 1);
	}
	else if(serialInput.find("FinishCycle(") == 0){
		functionType = functionType::FinishCycle;
		serialInput = serialInput.substr(12, serialInput.length() - 1);
	}
	else if(serialInput.find("StartCycle(") == 0){
		serialInput = serialInput.substr(11, serialInput.length() - 1);

		if(commaCount != 1){
			functionType = functionType::None;
			return;
		}

		functionType = functionType::StartCycle;

		index = serialInput.find(',');
		uint16_1 = StringToUint16_t(serialInput.substr(0, index));
		serialInput.erase(0, index+1);

		if (serialInput.find("true") != -1){
			bool_1 = true;
		}
		else if(serialInput.find("false") != -1){
			bool_1 = false;
		}
		else{
			functionType = functionType::None;
		}
	}
	else{
		functionType = functionType::None;
	}
}

void SerialController::callFunctions() {
	if(!newData)
		return;


	switch (functionType) {
		case functionType::None:
			newData = false;
			return;
		case functionType::GetVersion:
			serialStash((string)"Version: 45");
			break;
		case functionType::Clear:
			ledController->Clear();
			serialStash(true);
			break;
		case functionType::StopAllActions:
			ledController->StopAllActions();
			serialStash(true);
			break;
		case functionType::FinishAllAction:
			ledController->FinishAllActions();
			serialStash(true);
			break;
		case functionType::GetLedCount:
			serialStash(ledController->GetLedCount());
			break;
		case functionType::SetLedCount:
			serialStash(ledController->SetLedCount(uint16_1));
			break;
		case functionType::GetLedColor:
			serialStash(ledController->GetLedColor(uint16_1));
			break;
		case functionType::SetStaticColor1:
			serialStash(ledController->SetStaticColor(rgb_1));
			break;
		case functionType::SetStaticColor2:
			serialStash(ledController->SetStaticColor(uint16_1, rgb_1));
			break;
		case functionType::SetStaticColor3:
			serialStash(ledController->SetStaticColor(uint16_1, uint16_2, rgb_1));
			break;
		case functionType::IsFading:
			serialStash(ledController->IsFading());
			break;
		case functionType::StopFade:
			ledController->StopFade();
			serialStash(true);
			break;
		case functionType::FadeToStaticColor1:
			serialStash(ledController->FadeToStaticColor(uint16_1, rgb_1));
			break;
		case functionType::FadeToStaticColor2:
			serialStash(ledController->FadeToStaticColor(uint16_1, uint16_2, rgb_1));
			break;
		case functionType::FadeToStaticColor3:
			serialStash(ledController->FadeToStaticColor(uint16_1, uint16_2, uint16_3, rgb_1));
			break;
		case functionType::SetGradient1:
			serialStash(ledController->SetGradient(rgb_1, rgb_2));
			break;
		case functionType::SetGradient2:
			serialStash(ledController->SetGradient(uint16_1, uint16_2, rgb_1, rgb_2));
			break;
		case functionType::FadeToGradient1:
			serialStash(ledController->FadeToGradient(uint16_1, rgb_1,rgb_2));
			break;
		case functionType::FadeToGradient2:
			serialStash(ledController->FadeToGradient(uint16_1, uint16_2, uint16_3, rgb_1,rgb_2));
			break;
        case functionType::IsPulsing:
            serialStash(ledController->IsPulsing());
            break;
        case functionType::StopPulse:
            ledController->StopPulse();
            serialStash(true);
            break;
        case functionType::FinishPulseUp:
            ledController->FinishPulseUp();
            serialStash(true);
            break;
        case functionType::FinishPulseDown:
            ledController->FinishPulseDown();
            serialStash(true);
            break;
        case functionType::StartPulse1:
            serialStash(ledController->StartPulse(uint16_1, uint16_2, uint16_3, uint16_4, uint16_5, bool_1, rgb_1));
            break;
        case functionType::StartPulse2:
            serialStash(ledController->StartPulse(uint16_1, uint16_2, uint16_3, uint16_4, rgb_1));
            break;
		case functionType::IsCycling:
			serialStash(ledController->IsCycling());
			break;
		case functionType::StopCycle:
			ledController->StopCycle();
			serialStash(true);
			break;
		case functionType::FinishCycle:
			ledController->FinishCycle();
			serialStash(true);
			break;
		case functionType::StartCycle:
			ledController->StartCycle(uint16_1, bool_1);
			serialStash(true);
			break;
		default:
			break;
	}
	newData = false;
}

void SerialController::serialWrite() {
	if(serialOutput == "")
		return;

	puts(('>' + serialOutput).c_str());
}

void SerialController::serialStash(string content) {
	serialOutput = content;
}

void SerialController::serialStash(uint16_t content) {
	serialOutput = to_string(content);
}

void SerialController::serialStash(RGB content) {

	if(content.R < 10)
		serialOutput += "00" + to_string(content.R) + ',';
	else if (content.R < 100)
		serialOutput += "0" + to_string(content.R) + ',';
	else
		serialOutput += to_string(content.R) + ',';

	if(content.G < 10)
		serialOutput += "00" + to_string(content.G) + ',';
	else if (content.G < 100)
		serialOutput += "0" + to_string(content.G) + ',';
	else
		serialOutput += to_string(content.G) + ',';


	if(content.B < 10)
		serialOutput += "00" + to_string(content.B);
	else if (content.B < 100)
		serialOutput += "0" + to_string(content.B);
	else
		serialOutput += to_string(content.B);
}

void SerialController::serialStash(bool content) {
	if(content)
		serialOutput = "true";
	else
		serialOutput = "false";
}

RGB SerialController::StringToRgb(string input) {
	int index = input.find(',');

	if(index == -1)
		return RGB(0,0,0);

	uint8_t r = stoi(input.substr(0,index));
	input.erase(0,index + 1);

	index = input.find(',');
	if(index == -1)
		return RGB(0,0,0);

	uint8_t g = stoi(input.substr(0,index));
	input.erase(0,index + 1);

	if(input.find(')') > input.find(','))
		index = input.find(',');
	else
		index = input.find(')');

	uint8_t b = stoi(input.substr(0,index));

	return RGB(r, g, b);
}

uint16_t SerialController::StringToUint16_t(string input) {
	uint16_t output = stoi(input);
	return output;
}

uint8_t SerialController::getCommaCount(string input) {
	int index = input.find(',');
	uint8_t commaCount = 0;

	while (index != -1){
		input = input.substr(index + 1, input.length());
		commaCount++;
		index = input.find(',');
	}
	return commaCount;
}

