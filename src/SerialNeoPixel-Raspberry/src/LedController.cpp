//
// Created by Tobias on 17.07.2021.
//
#include <pico/stdlib.h.>
#include <stdio.h>
#include <string>
#include <cmath>

#include "../include/LedController.h"

void LedController::Init(uint16_t ledCount){

	led.updateLength(ledCount);
	led.updateType(NEO_GRB + NEO_KHZ800);
	led.setPin(0);
	led.begin();

	delete [] fadeOrigRgbArray;
	fadeOrigRgbArray = new RGB [GetLedCount()];

	delete [] fadeDestRgbArray;
	fadeDestRgbArray = new RGB [GetLedCount()];

	delete [] fadePeriodArray;
	fadePeriodArray = new uint16_t[GetLedCount()];

	delete [] fadeStepArray;
	fadeStepArray = new uint8_t [GetLedCount()];

	delete [] fadeStartTimeArray;
	fadeStartTimeArray = new uint32_t [GetLedCount()];

	delete [] fadeEndedArray;
	fadeEndedArray = new bool[GetLedCount()];

	delete[] cycleFadeStepArray;
	cycleFadeStepArray = new uint8_t[GetLedCount()];

	delete [] rgbArray;
	rgbArray = new RGB[GetLedCount()];



	for (uint16_t i = 0; i < GetLedCount(); i++) {
		fadeStepArray[i] = 0;
		fadeStartTimeArray[i] = 0;
		fadeEndedArray[i] = true;
	}

	lastUpdated = to_ms_since_boot(get_absolute_time());
}


void LedController::Update() {
    switch (effect) {
        case None:
            updateFade();
            if(toUpdate)
                pushRgbArrayToStrip();
            break;
        case Cycle:
            updateFade();
            updateCycleEffect();
            break;
        case Pulse:
            updateFade();
            updatePulseEffect();
            if(toUpdate)
                pushRgbArrayToStrip();
            break;
    }

    if(toUpdate){
        led.show();
        toUpdate = false;
	}
}

uint16_t LedController::GetLedCount(){
	return led.numPixels();
}

bool LedController::SetLedCount(uint16_t count) {
	if(count == 0 || count > 1024)
		return false;

	led.updateLength(count);
	return true;
}

RGB LedController::GetLedColor(uint16_t ledNumber) {
	if(isLedNumber(ledNumber))
        return rgbArray[ledNumber-1];
	else
		return RGB(0,0,0);
}


bool LedController::isLedNumber(uint16_t number) {
	if (number > GetLedCount() || number == 0)
		return false;

	return true;
}


void LedController::setPixelColor(uint16_t ledNumber, RGB rgb) {
    if(isLedNumber(ledNumber)){
        rgbArray[ledNumber-1] = rgb;
    }
}


void LedController::Clear() {
    for(int i = 0; i < GetLedCount(); ++i){
        rgbArray[i] = RGB(0,0,0);
    }
	StopAllActions();
	toUpdate = true;
}

void LedController::StopAllActions() {
	StopFade();
    StopPulse();
	StopCycle();
}

void LedController::FinishAllActions() {
	FinishPulseUp();
	FinishCycle();
}






//region setStaticColor private functions

bool LedController::setStaticColor(RGB rgb) {
	for (uint16_t i = 1; i <= GetLedCount(); i++) {
		setPixelColor(i, rgb);

		if(!fadeEndedArray[i - 1])
			fadeEndedArray[i - 1] = true;
	}
	toUpdate = true;
	return true;
}

bool LedController::setStaticColor(uint16_t ledNumber, RGB rgb) {
	if (!isLedNumber(ledNumber))
		return false;


	setPixelColor(ledNumber, rgb);
	if(!fadeEndedArray[ledNumber - 1])
		fadeEndedArray[ledNumber - 1] = true;


	toUpdate = true;
	return true;
}

bool LedController::setStaticColor(uint16_t ledNumberFrom, uint16_t count, RGB rgb) {
	if (!isLedNumber(ledNumberFrom) || !isLedNumber(ledNumberFrom + count - 1))
		return false;

	for (uint16_t i = ledNumberFrom; i < ledNumberFrom + count; i++) {
		setPixelColor(i, rgb);
		if(!fadeEndedArray[i - 1])
			fadeEndedArray[i - 1] = true;
	}
	toUpdate = true;
	return true;
}
//endregion





//region SetStaticColor Public functions
bool LedController::SetStaticColor(RGB rgb) {
    StopPulse();
    return setStaticColor(rgb);
}

bool LedController::SetStaticColor(uint16_t ledNumber, RGB rgb) {
    StopPulse();
    return setStaticColor(ledNumber, rgb);
}

bool LedController::SetStaticColor(uint16_t ledNumberFrom, uint16_t count, RGB rgb) {
    StopPulse();
	return setStaticColor(ledNumberFrom, count, rgb);
}
//endregion





//region Fade functions
bool LedController::IsFading() {
	for (uint16_t i = 0; i < GetLedCount(); i++) {
		if(!fadeEndedArray[i])
			return false;
	}
	return true;
}

bool LedController::fadeToStaticColor(uint16_t period, RGB rgb) {
    if(period == 0)
        period = 1;

    uint32_t time = to_ms_since_boot(get_absolute_time());

    for (uint16_t i = 0; i < GetLedCount(); i++) {
        fadeOrigRgbArray[i] = rgbArray[i];
        fadeDestRgbArray[i] = rgb;
        fadePeriodArray[i] = period;
        fadeStepArray[i] = 0;
        fadeStartTimeArray[i] = time;
        fadeEndedArray[i] = false;
    }
    return true;
}

bool LedController::fadeToStaticColor(uint16_t period, uint16_t ledNumber, RGB rgb) {
    if (!isLedNumber(ledNumber))
        return false;

    if(period == 0)
        period = 1;

    fadeOrigRgbArray[ledNumber - 1] = rgbArray[ledNumber-1];
    fadeDestRgbArray[ledNumber - 1] = rgb;
    fadePeriodArray[ledNumber - 1] = period;
    fadeStepArray[ledNumber - 1] = 0;
    fadeStartTimeArray[ledNumber - 1] = to_ms_since_boot(get_absolute_time());
    fadeEndedArray[ledNumber - 1]= false;

    return true;
}

bool LedController::fadeToStaticColor(uint16_t period, uint16_t ledNumberFrom, uint16_t count, RGB rgb) {
    if (!isLedNumber(ledNumberFrom) || !isLedNumber(ledNumberFrom + count - 1))
        return false;

    if(count == 0)
        return false;

    if(period == 0)
        period = 1;

    uint32_t time = to_ms_since_boot(get_absolute_time());

    for (uint16_t i = ledNumberFrom - 1; i < ledNumberFrom + count - 1; i++) {
        fadeOrigRgbArray[i] = rgbArray[i];
        fadeDestRgbArray[i] = rgb;
        fadePeriodArray[i] = period;
        fadeStepArray[i] = 0;
        fadeStartTimeArray[i] = time;
        fadeEndedArray[i] = false;
    }

    return true;
}


bool LedController::FadeToStaticColor(uint16_t period, RGB rgb) {
    StopPulse();
    return fadeToStaticColor(period, rgb);
}

bool LedController::FadeToStaticColor(uint16_t period, uint16_t ledNumber, RGB rgb) {
    StopPulse();
    return fadeToStaticColor(period, ledNumber, rgb);
}

bool LedController::FadeToStaticColor(uint16_t period, uint16_t ledNumberFrom, uint16_t count, RGB rgb) {
    StopPulse();
    return fadeToStaticColor(period, ledNumberFrom, count, rgb);
}

void LedController::pushRgbArrayToStrip() {
    for(int i = 0; i < GetLedCount(); ++i){
        led.setPixelColor(i, rgbArray[i].R, rgbArray[i].G, rgbArray[i].B);
    }
}

bool LedController::IsPulsing() {
    if(effect == Pulse)
        return true;
    return false;
}


void LedController::StopPulse() {
    if(!IsPulsing())
        return;

    StopFade();

    effect = None;
    pulseCountLeft = 0;
}

void LedController::FinishPulseUp() {
    if(!IsPulsing())
        return;

    pulseCountLeft = 1;
    pulseFinishUp = true;
}

void LedController::FinishPulseDown() {
    if(!IsPulsing())
        return;

    pulseCountLeft = 1;
    pulseFinishUp = false;
}

bool LedController::StartPulse(uint16_t periodUp, uint16_t periodHoldUp, uint16_t periodDown, uint16_t periodHoldDown, uint16_t pulseCount, bool finishPulseUp, RGB rgb) {
    if((periodUp == 0 && periodHoldUp == 0) || (periodDown == 0 && periodHoldDown == 0))
        return false;

    if(periodUp == 0)
        periodUp = 1;

    if(periodDown == 0)
        periodDown = 1;

    StopPulse();
    effect = Pulse;
	pulseCountLeft = pulseCount;
	pulsePeriodUp = periodUp;
	pulsePeriodHoldUp = periodHoldUp;
	pulsePeriodDown = periodDown;
    pulsePeriodHoldDown = periodHoldDown;
    pulseFinishUp = finishPulseUp;
    pulseStartTime = to_ms_since_boot(get_absolute_time());
    pulseRgb = rgb;

    return true;
}

bool LedController::StartPulse(uint16_t periodUp, uint16_t periodHoldUp, uint16_t periodDown, uint16_t periodHoldDown, RGB rgb) {
	return StartPulse(periodUp, periodHoldUp, periodDown, periodHoldDown, 0, true, rgb);
}

void LedController::updatePulseEffect(){
	if(!IsPulsing() || !fadeEndedArray[0])
		return;

    uint32_t time = to_ms_since_boot(get_absolute_time());

    if(time >= pulseStartTime + pulsePeriodUp + pulsePeriodHoldUp){

        if (pulseCountLeft > 0) {
            pulseCountLeft--;
            if (pulseCountLeft == 0) {
                if (!pulseFinishUp)
                    fadeToStaticColor(pulsePeriodDown, RGB(0, 0, 0));
                effect = None;
                return;
            }
        }

        fadeToStaticColor(pulsePeriodDown, RGB(0,0,0));
        pulseStartTime = time + pulsePeriodDown + pulsePeriodHoldDown;
    }
    else if(time >= pulseStartTime && time < pulseStartTime + pulsePeriodUp){
        if(rgbArray[0] == 0)
            fadeToStaticColor(pulsePeriodUp, pulseRgb);
    }
}




void LedController::StopFade() {
	for (uint16_t i = 0; i < GetLedCount(); i++) {
		fadeEndedArray[i] = true;
	}
}

void LedController::updateFade() {
	RGB rgb;

    uint32_t time = to_ms_since_boot(get_absolute_time());

	for (uint16_t i = 0; i < GetLedCount(); i++) {
		if(fadeEndedArray[i])
			continue;

		fade.SetEnded(false);
		fade.SetColors(fadeOrigRgbArray[i], fadeDestRgbArray[i]);
		fade.SetPeriod(fadePeriodArray[i]);
		fade.SetStep(fadeStepArray[i]);
		fade.SetTimeStart(fadeStartTimeArray[i]);


		if(!fade.GetNext(rgb, time)){
            fadeEndedArray[i] = fade.GetEnded();
			continue;
        }

		fadeEndedArray[i] = fade.GetEnded();
		fadeStepArray[i] = fade.GetStep();

        setPixelColor(i+1, rgb);
        toUpdate = true;
	}
}
//endregion



bool LedController::SetGradient(RGB rgbStart, RGB rgbEnd) {
    return SetGradient(1, GetLedCount(), rgbStart, rgbEnd);
}

//region SetGradient
bool LedController::SetGradient(uint16_t ledNumberFrom, uint16_t count, RGB rgbStart, RGB rgbEnd) {
	if (!isLedNumber(ledNumberFrom) || !isLedNumber(ledNumberFrom + count - 1))
		return false;

	if(count == 0)
		return false;

    StopPulse();

	setPixelColor(ledNumberFrom, rgbStart);

	toUpdate = true;

	if(count == 1)
		return true;

	for (uint16_t i = 1; i < count; i++) {
		uint8_t red = (1.0/255.0)*pow(((rgbStart.R * (count - 1 - i)) + (rgbEnd.R * i)) / (count-1), 2);
		uint8_t green = (1.0/255.0)*pow(((rgbStart.G * (count - 1 - i)) + (rgbEnd.G * i)) / (count-1), 2);
		uint8_t blue = (1.0/255.0)*pow(((rgbStart.B * (count - 1 - i)) + (rgbEnd.B * i)) / (count-1), 2);
		// Sets the pixels to the color adjusted in the fade
        setPixelColor(ledNumberFrom + i, RGB(red, green, blue));
	}

	return true;
}

bool LedController::FadeToGradient(uint16_t period, RGB rgbStart, RGB rgbEnd) {
    return FadeToGradient(period, 1, GetLedCount(), rgbStart, rgbEnd);
}

bool LedController::FadeToGradient(uint16_t period, uint16_t ledNumberFrom, uint16_t count, RGB rgbStart, RGB rgbEnd) {
	if (!isLedNumber(ledNumberFrom) || !isLedNumber(ledNumberFrom + count - 1))
		return false;

	if(count == 0)
		return false;

    StopPulse();

    FadeToStaticColor(period, ledNumberFrom, rgbStart);

	toUpdate = true;

	if(count == 1)
		return true;

	for (uint16_t i = 1; i < count; i++) {
		uint8_t red = (1.0/255.0)*pow(((rgbStart.R * (count - 1 - i)) + (rgbEnd.R * i)) / (count-1), 2);
		uint8_t green = (1.0/255.0)*pow(((rgbStart.G * (count - 1 - i)) + (rgbEnd.G * i)) / (count-1), 2);
		uint8_t blue = (1.0/255.0)*pow(((rgbStart.B * (count - 1 - i)) + (rgbEnd.B * i)) / (count-1), 2);
		// Sets the pixels to the color adjusted in the fade
        FadeToStaticColor(period, ledNumberFrom + i, RGB(red,green,blue));
	}
	return true;
}
//endregion





//region Cycle functions
bool LedController::IsCycling() {
    if(effect == Cycle)
        return true;
	return false;
}

void LedController::StartCycle(uint16_t period, bool direction) {
    StopPulse();
    effect = Cycle;

	cyclePeriod = period;
	lastCycle = false;
	cycleStartTime = to_ms_since_boot(get_absolute_time());
	cyclePosition = 0;
	cycleDirection = direction;

	if(cyclePeriod / GetLedCount() >= 20.0){
		cycleSmoothAnimation = true;
		for (uint16_t i = 0; i < GetLedCount(); i++) {
			cycleFadeStepArray[i] = 0;
		}
	}
	else
		cycleSmoothAnimation = false;
}

void LedController::StopCycle() {
	if (!IsCycling())
        return;

    effect = None;
}

void LedController::FinishCycle() {
	if (IsCycling() && !lastCycle)
		lastCycle = true;
}

void LedController::cycleFadeUpdate() {
	if(!cycleSmoothAnimation)
		return;

	RGB rgb;

    uint32_t time = to_ms_since_boot(get_absolute_time());

	if(cycleDirection){
		for (uint16_t i = 0; i < GetLedCount(); ++i) {
			uint16_t step = (i + cyclePosition)%GetLedCount();

			int stepBefore = (step - 1);
			if(stepBefore < 0)
				stepBefore = (GetLedCount()) + stepBefore;


			fade.SetEnded(false);
			fade.SetColors(rgbArray[stepBefore], rgbArray[step]);
			fade.SetPeriod(cyclePeriod / GetLedCount());
			fade.SetStep(cycleFadeStepArray[i]);
            uint32_t timeStart = cycleStartTime + (cyclePeriod / GetLedCount())*(cyclePosition);
            fade.SetTimeStart(timeStart);


			if(!fade.GetNext(rgb, time))
				continue;

            cycleFadeStepArray[i] = fade.GetStep();

			led.setPixelColor(i, rgb.R, rgb.G, rgb.B);
            toUpdate = true;
		}
	}
	else{
		for (uint16_t i = 0; i < GetLedCount(); ++i) {
			int step = i - cyclePosition;
			if(step < 0)
				step = (GetLedCount()) + step;

			uint16_t stepBefore = (step + 1)%GetLedCount();

			fade.SetEnded(false);
			fade.SetColors(rgbArray[stepBefore], rgbArray[step]);
            fade.SetPeriod(cyclePeriod / GetLedCount());

            fade.SetStep(cycleFadeStepArray[i]);
            uint32_t timeStart = cycleStartTime + (cyclePeriod / GetLedCount())*(cyclePosition);
            fade.SetTimeStart(timeStart);

			if(!fade.GetNext(rgb, time))
				continue;

			cycleFadeStepArray[i] = fade.GetStep();

			led.setPixelColor(i, rgb.R, rgb.G, rgb.B);
            toUpdate = true;
        }
	}

}


void LedController::updateCycleEffect() {
	if(!IsCycling())
		return;

	cycleFadeUpdate();

	if(to_ms_since_boot(get_absolute_time()) < cycleStartTime + ((cyclePeriod / GetLedCount()) * (cyclePosition+1))){
		return;
	}


	if(cycleSmoothAnimation){
		//Falls ein Fade zwischen den einzelnen Steps nötig ist
		for (uint16_t i = 0; i < GetLedCount(); i++) {
			cycleFadeStepArray[i] = 0;
		}
	}
	//Falls kein Fade zwischen den steps nötig ist
	else{
		if(cycleDirection){
			for (uint16_t i = 0; i < GetLedCount(); i++) {
				uint16_t step = i + cyclePosition;
				if(step >= GetLedCount())
					step = step - (GetLedCount() - 1);

				led.setPixelColor(i, rgbArray[step].R, rgbArray[step].G, rgbArray[step].B);
                toUpdate = true;
            }
		}
		else{
			for (uint16_t i = 0; i < GetLedCount(); i++) {
				int step = i - cyclePosition;
				if(step < 0)
					step = (GetLedCount() - 1) + step;

                led.setPixelColor(i, rgbArray[step].R, rgbArray[step].G, rgbArray[step].B);
	            toUpdate = true;

            }
		}
	}




	if(cyclePosition == 0 && lastCycle){
		effect = None;
		lastCycle = false;
		return;
	}

	cyclePosition++;

	if(cyclePosition >= GetLedCount()){
		cycleStartTime = to_ms_since_boot(get_absolute_time());
		cyclePosition = 0;
	}
}

//endregion