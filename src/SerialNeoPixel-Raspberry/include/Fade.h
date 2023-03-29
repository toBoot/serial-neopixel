//
// Created by Tobias on 22.07.2021.
//

#ifndef SERIALNEOPIXEL_RASPBERRY_FADE_H
#define SERIALNEOPIXEL_RASPBERRY_FADE_H


#include "RGB.h"


class Fade {
public:
//    void Start();
    bool GetEnded();
    void SetEnded(bool b);
    void SetPeriod(int p);
    void SetColors(RGB StartRgb, RGB EndRgb);
    bool GetNext(RGB &rgb, uint32_t time);
    void ForceNext(RGB &rgb);

    void SetStep(int s);
    int GetStep();

    void SetTimeStart(uint32_t t);
    uint32_t GetTimeStart();

private:

    RGB startRgb, endRgb;
    bool ended;


    int steps = 255;
    int period, step;
    uint32_t timeStart;

};


#endif //SERIALNEOPIXEL_ARDUINO_FADE_H
