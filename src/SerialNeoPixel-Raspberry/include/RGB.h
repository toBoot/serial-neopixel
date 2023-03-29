//
// Created by Tobias on 22.07.2021.
//

#ifndef SERIALNEOPIXEL_RASPBERRY_RGB_H
#define SERIALNEOPIXEL_RASPBERRY_RGB_H

struct RGB{
	RGB(){}
	RGB(uint8_t r, uint8_t g, uint8_t b){
		R = r;
		G = g;
		B = b;
	}
	RGB(uint32_t rgb){
		B = rgb & 0xFF;
		G = (rgb >> 8) & 0xFF;
		R = (rgb >> 16) & 0xFF;
	}
	uint8_t R;
	uint8_t G;
	uint8_t B;

    public:
        bool operator==(const RGB rgb){
            if(rgb.R == R && rgb.G == G && rgb.B == B)
                return true;
            return false;
        }
};


#endif //SERIALNEOPIXEL_ARDUINO_RGB_H
