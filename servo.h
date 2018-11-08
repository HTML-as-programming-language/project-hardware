#include <avr/io.h>

void update_servo(uint8_t status)
{
	DDRB|=0x08;

	TCCR2A=0x81; // COM2A1 en WGM20
	TCCR2B=0x06; // 256 prescaler
	OCR2A = status;
}
