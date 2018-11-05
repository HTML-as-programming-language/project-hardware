#include <avr/io.h>
// PWM
// begint een pwm controller op pin 3
void servo_init() {
	DDRD |= 0x08; // set pin 3 als output
	TCCR2A = _BV(COM2A1) | _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
  	TCCR2B = _BV(WGM22) | _BV(CS20);
  	OCR2A = 0xFF;
	OCR2B = 0x00; //000
}

void servo_set(uint8_t value) { //value is een getal tussen de 0x00 en 0xff
	OCR2B = value;
}