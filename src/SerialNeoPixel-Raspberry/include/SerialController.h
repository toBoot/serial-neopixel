//
// Created by Tobias on 17.07.2021.
//

#ifndef SERIALNEOPIXEL_ARDUINO_SERIALCONTROLLER_H
#define SERIALNEOPIXEL_ARDUINO_SERIALCONTROLLER_H

#include <string>

#include "LedController.h"

using namespace std;

class SerialController {
public:
    void Init(LedController* led);
    void Update();

private:
    LedController* ledController;

    string serialInput;
    string serialOutput;

    bool newData = false;

    void updateSerialInput();

    void updateFunctionType();

    void callFunctions();

    void serialWrite();

    uint16_t uint16_1, uint16_2, uint16_3, uint16_4, uint16_5;
    RGB rgb_1, rgb_2;
    bool bool_1;




    void serialStash(string content);
    void serialStash(uint16_t content);
    void serialStash(RGB content);
    void serialStash(bool content);


    static uint16_t StringToUint16_t(string input);
    static RGB StringToRgb(string input);
    static uint8_t getCommaCount(string input);


    enum functionType{
        None,
        GetVersion,
        Clear,
        StopAllActions,
        FinishAllAction,
        GetLedCount,
        SetLedCount,
        GetLedColor,
        SetStaticColor1,
        SetStaticColor2,
        SetStaticColor3,
        IsFading,
        StopFade,
        FadeToStaticColor1,
        FadeToStaticColor2,
        FadeToStaticColor3,
        SetGradient1,
        SetGradient2,
        FadeToGradient1,
        FadeToGradient2,
        IsPulsing,
        StopPulse,
        FinishPulseUp,
        FinishPulseDown,
        StartPulse1,
        StartPulse2,
        IsCycling,
        StopCycle,
        FinishCycle,
        StartCycle,
    } functionType;
};

#endif //SERIALNEOPIXEL_ARDUINO_SERIALCONTROLLER_H
