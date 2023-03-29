//
// Created by Tobias on 17.07.2021.
//

#ifndef SERIALNEOPIXEL_RASPBERRY_LEDCONTROLLER_H
#define SERIALNEOPIXEL_RASPBERRY_LEDCONTROLLER_H


#include <Adafruit_NeoPixel.hpp>

#include "RGB.h"
#include "Fade.h"


class LedController {
public:

    void Init(uint16_t ledCount);

//    void Init(uint8_t ledPin, neoPixelType pixelType);
    void Update();

    void Clear();

    void StopAllActions();

    void FinishAllActions();

    uint16_t GetLedCount();

    bool SetLedCount(uint16_t count);

    RGB GetLedColor(uint16_t ledNumber);


    bool SetStaticColor(RGB rgb);

    bool SetStaticColor(uint16_t ledNumber, RGB rgb);

    bool SetStaticColor(uint16_t ledNumberFrom, uint16_t count, RGB rgb);

    bool IsFading();

    void StopFade();

    bool FadeToStaticColor(uint16_t period, RGB rgb);

    bool FadeToStaticColor(uint16_t period, uint16_t ledNumber, RGB rgb);

    bool FadeToStaticColor(uint16_t period, uint16_t ledNumberFrom, uint16_t count, RGB rgb);

    bool SetGradient(RGB rgbStart, RGB rgbEnd);

    bool SetGradient(uint16_t ledNumberFrom, uint16_t count, RGB rgbStart, RGB rgbEnd);

    bool FadeToGradient(uint16_t period, RGB rgbStart, RGB rgbEnd);

    bool FadeToGradient(uint16_t period, uint16_t ledNumberFrom, uint16_t count, RGB rgbStart, RGB rgbEnd);


    bool IsPulsing();

    void StopPulse();

    void FinishPulseUp();

    void FinishPulseDown();

    bool StartPulse(uint16_t periodUp, uint16_t periodHoldUp, uint16_t periodDown, uint16_t periodHoldDown, uint16_t pulseCount, bool finishPulseUp, RGB rgb);

    bool StartPulse(uint16_t periodUp, uint16_t periodHoldUp, uint16_t periodDown, uint16_t periodHoldDown, RGB rgb);

    bool IsCycling();

    void StopCycle();

    void FinishCycle();

    void StartCycle(uint16_t period, bool direction);


private:

    enum EffectType {
        None,
        Cycle,
        Pulse
    } effect;

    Adafruit_NeoPixel led;

    Fade fade;


    //Dieses rgbArray soll das Zentrale rgb array sein. Wenn kein Effekt aktiv ist, wird dieser von einer
    //update Funktion zum RGB Strip gepusht. Falls ein Effekt aktiv ist, wird dieser auf das Array angewandt.

    RGB *rgbArray;


    /// <summary>
    ///
    /// </summary>
    /// <param name="ledNumber, one based"></param>
    /// <param name="red"></param>
    /// <param name="green"></param>
    /// <param name="blue"></param>
    void setPixelColor(uint16_t ledNumber, RGB rgb);

    /// <summary>
    ///
    /// </summary>
    /// <param name="number, one based"></param>
    /// <returns></returns>
    bool isLedNumber(uint16_t number);

    bool toUpdate = false;
    uint32_t lastUpdated;

    bool setStaticColor(RGB rgb);

    bool setStaticColor(uint16_t ledNumber, RGB rgb);

    bool setStaticColor(uint16_t ledNumberFrom, uint16_t count, RGB rgb);


    bool fadeToStaticColor(uint16_t period, RGB rgb);
    bool fadeToStaticColor(uint16_t period, uint16_t ledNumber, RGB rgb);
    bool fadeToStaticColor(uint16_t period, uint16_t ledNumberFrom, uint16_t count, RGB rgb);


    //Fade stuff
    void updateFade();

    RGB      *fadeOrigRgbArray;
    RGB      *fadeDestRgbArray;
    uint16_t *fadePeriodArray;
    uint8_t  *fadeStepArray;
    uint32_t *fadeStartTimeArray;
    bool     *fadeEndedArray;


    void pushRgbArrayToStrip();

    //Pulse stuff
    void updatePulseEffect();

    uint16_t pulseCountLeft = 0;
    bool     pulseFinishUp;
    uint16_t pulsePeriodUp;
    uint16_t pulsePeriodDown;
    uint16_t pulsePeriodHoldUp;
    uint16_t pulsePeriodHoldDown;
    uint32_t pulseStartTime;
    RGB      pulseRgb;


    //Cycle stuff
    void cycleFadeUpdate();

    uint8_t *cycleFadeStepArray;


    void updateCycleEffect();

    uint16_t cyclePeriod;
    bool     lastCycle = false;
    bool     cycleSmoothAnimation;
    uint32_t cycleStartTime;
    uint16_t cyclePosition;
    bool     cycleDirection;
};

#endif //SERIALNEOPIXEL_ARDUINO_LEDCONTROLLER_H
