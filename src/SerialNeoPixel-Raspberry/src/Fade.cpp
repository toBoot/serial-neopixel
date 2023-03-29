//
// Created by Tobias on 22.07.2021.
//
#include <pico/stdlib.h.>
#include <pico/double.h>
//#include <pico.h>

#include "../include/Fade.h"

bool Fade::GetEnded() {
	return ended;
}

void Fade::SetEnded(bool b) {
	ended = b;
}

void Fade::SetPeriod(int p){
    period = p;

	if(period < 255)
		steps = period;
	else
		steps = 255;
}

void Fade::SetColors(RGB StartRgb, RGB EndRgb) {
	startRgb = StartRgb;
	endRgb = EndRgb;
}

bool Fade::GetNext(RGB &rgb, uint32_t time) {
	if(ended)
		return false;

	if(time > timeStart + step*(period/steps)){
		if(step == 0)
		{
			rgb = startRgb;
			step ++;
			return true;
		}

		rgb.R = ((startRgb.R * (steps - step)) + (endRgb.R * step)) / (steps);
		rgb.G = ((startRgb.G * (steps - step)) + (endRgb.G * step)) / (steps);
		rgb.B = ((startRgb.B * (steps - step)) + (endRgb.B * step)) / (steps);

		//Falls das Berechnen zu lange dauert und man steps überspringen muss:
		double deltaT = (double)period/(double)steps;
		double timeStep = (time-timeStart-step*deltaT)/deltaT;
		step = step + (int)round(timeStep);

		if(step >= steps){
			rgb = endRgb;
			ended = true;
		}

		return true;
	}
	else{
		return false;
	}
}

void Fade::ForceNext(RGB &rgb) {
	if(step == 0)
	{
		rgb = startRgb;
		return;
	}

	rgb.R = ((startRgb.R * (steps - step)) + (endRgb.R * step)) / (steps);
	rgb.G = ((startRgb.G * (steps - step)) + (endRgb.G * step)) / (steps);
	rgb.B = ((startRgb.B * (steps - step)) + (endRgb.B * step)) / (steps);

	if(step >= steps){
		rgb = endRgb;
	}
}

void Fade::SetStep(int s) {
	if(s <= steps){
		step = s;
	}
	else{
		step=steps;
	}
}

int Fade::GetStep() {
	return step;
}


void Fade::SetTimeStart(uint32_t t) {
	timeStart = t;
}

unsigned long Fade::GetTimeStart() {
	return timeStart;
}

