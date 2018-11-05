#include <avr/io.h>
// PWM
// begint een pwm controller op pin 3
void servo_init() {
	DDRD |= 0x08; // set pin 3 als output
	TCCR2A |= 1<<WGM11 | 1<<COM1A1 | 1<<COM1A0;
	TCCR2B |= 1<<WGM13 | 1<<WGM12 | 1<<CS10;
	OCR2A = 0xff;

	OCR2B = ICR1 - 2000; //18000
}

void servo_set(uint16_t value) { //value is een getal tussen de 0 en 19999
	OCR2B = ICR1 - value;
}