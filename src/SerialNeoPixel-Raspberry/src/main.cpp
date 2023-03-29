//
// Created by Tobias on 17.08.2021.
//

#ifndef SERIALNEOPIXEL_RASPBERRY_MAIN_H
#define SERIALNEOPIXEL_RASPBERRY_MAIN_H

#include <stdio.h>
#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <hardware/irq.h>
//#include <Adafruit_NeoPixel.hpp>
#include <iostream>

#include "../include/LedController.h"
#include "../include/SerialController.h"

SerialController serialController;
LedController ledController;


std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
	size_t start_pos = 0;
	while((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Handles case where 'to' is a substr of 'from'
	}
	return str;
}

void core1_entry() {
	//init Function im 2. Kern
	char input;
	string serialInput = "";

	bool receiving = false;

	const char startMarker = '>';
	const char endMarker = ';';

	//Main loop vom 2. Kern
	while (true) {

		input = getchar();


		if (input == startMarker) {
			serialInput = "";
			receiving = true;
		} else if (receiving) {
			if (input == endMarker) {
				serialInput = ReplaceAll(serialInput, " ", "");
				receiving = false;

				if(!multicore_fifo_wready())
					continue;

				//multicore_fifo_drain();
				multicore_fifo_push_blocking((uint32_t) new string(serialInput));

			} else {
				serialInput += input;
			}
		}
	}
}




void init(){
	stdio_init_all();

	gpio_init(PICO_DEFAULT_LED_PIN);
	gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
	gpio_put(PICO_DEFAULT_LED_PIN, true);



	//TODO: Maximal mögliche Anzahl vom RPI herausfinden
	ledController.Init(1024);
	serialController.Init(&ledController);


	//Startup routine -> Pulsierende Animation
    ledController.SetStaticColor(RGB(0,0,0));
	ledController.StartPulse(1500,1000,1500,1000, RGB(0,0,255));

	//Starten vom 2. Kern, um seriell Daten zu empfangen
	multicore_launch_core1(core1_entry);
}


void loop(){
	ledController.Update();
	serialController.Update();
}

int main(){
	init();

	while (true){
		loop();
	}
}


#endif


