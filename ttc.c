#define F_CPU 16E6
#include <avr/io.h>
#include <avr/interrupt.h>
#include "analog.h"
#include <avr/eeprom.h>
#include <util/delay.h>

#define BAUD 9600
#define UBBRVAL F_CPU/16/BAUD-1

#define TrigPin PIND3
#define EchoPin PIND4

volatile uint8_t pingState = 0;
volatile uint8_t centimeter = 0;

uint8_t tempOn EEMEM = 30;
uint8_t tempOff EEMEM = 40;
uint8_t lightOn EEMEM = 50;
uint8_t lightOff EEMEM = 80;
uint8_t servoLaag EEMEM = 35;
uint8_t servoHoog EEMEM = 70;

int manual = 0;

#include "ttc.h"
#include "serialTx.h"
#include "serialRx.h"
#include "sensor.h"
#include "servo.h"

enum
{
	init = 0x65,
	temp = 0x66,
	licht = 0x67,
	rolluik = 0x68,
	afst = 0x69
};

union
{
	uint16_t ValInt;
	unsigned char Bytes[2];
} tempInt;

sTask SCH_tasks_G[SCH_MAX_TASKS];
void SCH_Dispatch_Tasks(void)
{
	unsigned char Index;

	for(Index = 0; Index < SCH_MAX_TASKS; Index++)
	{
		if((SCH_tasks_G[Index].RunMe > 0) && (SCH_tasks_G[Index].pTask != 0))
		{
			(*SCH_tasks_G[Index].pTask)();
			SCH_tasks_G[Index].RunMe -= 1;

			if(SCH_tasks_G[Index].Period == 0)
			{
				SCH_Delete_Task(Index);
			}
		}
	}
}

void uart_init()
{
	UBRR0H = 0;
	UBRR0L = UBBRVAL;
	UCSR0B |= _BV(RXEN0) | _BV(TXEN0) | _BV(RXCIE0);
	UCSR0C |= _BV(UCSZ00) | _BV(UCSZ01);
}

unsigned char SCH_Add_Task(void (*pFunction)(), const unsigned int DELAY, const unsigned int PERIOD)
{
	unsigned char Index = 0;

	while((SCH_tasks_G[Index].pTask != 0) && (Index < SCH_MAX_TASKS))
	{
		Index++;
	}


	if(Index == SCH_MAX_TASKS)
	{
		return SCH_MAX_TASKS;
	}


	SCH_tasks_G[Index].pTask = pFunction;
	SCH_tasks_G[Index].Delay =DELAY;
	SCH_tasks_G[Index].Period = PERIOD;
	SCH_tasks_G[Index].RunMe = 0;

	return Index;
}

unsigned char SCH_Delete_Task(const unsigned char TASK_INDEX)
{

	unsigned char Return_code = 0;

	SCH_tasks_G[TASK_INDEX].pTask = 0;
	SCH_tasks_G[TASK_INDEX].Delay = 0;
	SCH_tasks_G[TASK_INDEX].Period = 0;
	SCH_tasks_G[TASK_INDEX].RunMe = 0;

	return Return_code;
}

void SCH_Init_T0(void)
{
	unsigned char i;

	for(i = 0; i < SCH_MAX_TASKS; i++)
	{
		SCH_Delete_Task(i);
	}

	OCR0A = (uint8_t)156;
	TCCR0B = (1 << CS02)|(1 << CS00);
	TCCR0A = (1 << WGM01);
	TIMSK0 = 1 << OCIE0A;
}

void SCH_Start(void)
{
	sei();
}

int bla = 1;
ISR(INT0_vect)
{
	update_leds(bla);
	bla ^= 1;
	_delay_ms(500);
}

ISR(USART_RX_vect)
{
	buffer(UDR0);
}

ISR(PCINT2_vect)
{
	if(PIND & (1<<EchoPin))
	{
		TCNT1 = 0;
		TCCR1B |= (1<<CS11);
	}

	else
	{
		TCCR1B &= ~(1<<CS11);
		centimeter = TCNT1/58/2;
		pingState = 2;
	}
}

ISR(TIMER1_OVF_vect)
{
	TCCR1B &= ~(1<<CS10);
	pingState = 2;
}

ISR(TIMER0_COMPA_vect)
{
	unsigned char Index;
	for(Index = 0; Index < SCH_MAX_TASKS; Index++)
	{
		if(SCH_tasks_G[Index].pTask)
		{
			if(SCH_tasks_G[Index].Delay == 0)
			{
				SCH_tasks_G[Index].RunMe += 1;
				if(SCH_tasks_G[Index].Period)
				{
					SCH_tasks_G[Index].Delay = SCH_tasks_G[Index].Period;
					SCH_tasks_G[Index].Delay -= 1;
				}
			}
			else
				SCH_tasks_G[Index].Delay -= 1;
		}
	}
}
int aan = 1;
void update_leds(int status)
{
	if(status)
	{
		PORTB = 0x1;
		if(!aan)
		{
			aan = 1;
			for (int i = 0; i < 10; ++i)
			{
				PORTB ^= 0xfc;
				_delay_ms(100);
			}
			update_servo(eeprom_read_byte(&servoHoog));
			sendPacket(rolluik, 100);
		}
	}
	else
	{
		PORTB = 0x2;
		if(aan)
		{
			aan = 0;
			for (int i = 0; i < 10; ++i)
			{
				PORTB ^= 0xfc;
				_delay_ms(100);
			}
			update_servo(eeprom_read_byte(&servoLaag));
			sendPacket(rolluik, 0);
		}
	}
}

void initSensor()
{
	sendPacket(init, 0b00000011);
}

void tempTx()
{
	sendPacket(temp, getTemp());
}

void ultrTx()
{
	sendPacket(afst, ultrasoon());
}

void lightTx()
{
	sendPacket(licht, getLight());
}

uint8_t EEMEM eeprombyte=0x10;
uint16_t EEMEM eepromword=0x5555;
uint8_t EEMEM eepromstring[5]={"Test\0"};
uint16_t reboots EEMEM = 0;

void incReboot()
{
	uint16_t reboot_count = eeprom_read_word(&reboots);
	reboot_count++;
	eeprom_write_word(&reboots, reboot_count);
}

void eepromTest()
{
	uint8_t RAMbyte;
	uint16_t RAMword;
	uint8_t RAMstring[5];
	RAMbyte = eeprom_read_byte(&eeprombyte);
	RAMword = eeprom_read_word(&eepromword);
	eeprom_read_block ((void *)&RAMstring, (const void *)&eepromstring,5);
}

void testReboot()
{
	uint16_t reboot_count;
	reboot_count = eeprom_read_word(&reboots);
	tx(reboot_count);
}

void autoCheck()
{
	if ((getTemp() < eeprom_read_byte(&tempOn) || getLight() < eeprom_read_byte(&lightOn)) && !manual)
		update_leds(1);
	else if ((getTemp() > eeprom_read_byte(&tempOff) || getLight() > eeprom_read_byte(&lightOff)) && !manual)
		update_leds(0);
}

void blaleds()
{
	update_leds(1);
	_delay_ms(1000);
	update_leds(0);
	_delay_ms(1000);
}

void tempcheck()
{
	uint8_t bla = 0;
	bla = eeprom_read_byte(&tempOn);

	uint8_t blaa = 0;
	blaa = eeprom_read_byte(&tempOff);

	sendPacket(licht, bla);
	sendPacket(licht, blaa);
}

void servo()
{
	update_servo(35);
	_delay_ms(1000);
	update_servo(70);
	_delay_ms(1000);
}

int main()
{
	SPH = (RAMEND & 0xFF00) >> 8;
	SPL = (RAMEND & 0x00FF);

	TCCR1B = (1<<CS10);
	TIMSK1 = (1<<TOIE1);

	PCICR = (1<<PCIE2);
	PCMSK2 = (1<<PCINT20);

	DDRD = (1<<TrigPin);
	TCNT1 = 0;
	incReboot();
	uart_init();

	DDRB |= (1 << PB0); // led rolluik open
	DDRB |= (1 << PB1); // led rolluik dicht
	DDRB |= (1 << PB2); // led rolluik open/dicht aan het gaan
	adc_init();
	SCH_Init_T0();

	EIMSK |= 1 << INT0; // zet de interrupt pin aan op D2

	SCH_Add_Task(&initSensor, 0, 0);

	SCH_Add_Task(&tempTx, 0, 50);
	SCH_Add_Task(&ultrTx, 0, 50);
	SCH_Add_Task(&lightTx, 0, 50);
	SCH_Add_Task(&autoCheck, 0, 10);

	/* SCH_Add_Task(&servo, 0, 100); */
	/* SCH_Add_Task(&tempcheck, 0, 20); */
	/* SCH_Add_Task(&blaleds, 0, 200); */
	/* SCH_Add_Task(&testReboot, 0, 100); */

	SCH_Start();
	while (1)
	{
		SCH_Dispatch_Tasks();
	}
	return 0;
}
