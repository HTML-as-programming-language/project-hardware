#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/sfr_defs.h>
#include <avr/interrupt.h>
#include "analog.h"
#include <avr/eeprom.h>

#define F_CPU 16E6
#define FOSC 16E6 // Clock Speed
#define BAUD 9600
#define UBBRVAL FOSC/16/BAUD-1

#include "ttc.h"
#include "serial.h"

enum { init = 0x65, temp = 0x66, licht = 0x67, afst = 0x68 };

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

void SCH_Init_T1(void)
{
	unsigned char i;

	for(i = 0; i < SCH_MAX_TASKS; i++)
	{
		SCH_Delete_Task(i);
	}

	if (F_CPU == 16E6)
	{
		OCR1A = (uint16_t)625;
		TCCR1B = (1 << CS12) | (1 << WGM12);
	}
	else
	{
		OCR1A = (uint16_t)1300;
		TCCR1B = (1 << CS11) | (1 << WGM12);
	}

	TIMSK1 = 1 << OCIE1A;
}

void SCH_Start(void)
{
	sei();
}

ISR(TIMER1_COMPA_vect)
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
			{

				SCH_tasks_G[Index].Delay -= 1;
			}
		}
	}
}

void update_leds()
{
	PORTD ^= 0xFF;
}

void startPacket()
{
	tx(0xFF);
	tx(0xFF);
	tx(0x00);
}

void sendData()
{
	startPacket();
	tx(temp);
	tx(0x00);
	tx(0x10);
}

void initSensor()
{
	startPacket();
	tx(init);
	tx(0x00);
	tx(0x00);
}

int tempSensor() {
	return(adc_read(0)); //leest de temperatuur op pin A0
}

int lighSensor() {
	return(adc_read(1)); //leest de lichtintensiteid op pin A1
}

int HumiSensor() {
	return(adc_read(2)); //leest de luchtvochtigheid op pin A2
}

void sendString()
{
	txChar("Hello World");
}

void sensorTest() {
	int x = adc_read(0);
	if (x > 0x00ff) {
		txChar("High");
	}
	else {
		txChar("Low");
	}
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

int main()
{
	incReboot();
	uart_init();
	DDRD = 1 << 1;
	DDRB = 0;

	adc_init();
	SCH_Init_T1();

	/* SCH_Add_Task(&initSensor, 0, 0); */
	/* SCH_Add_Task(&sendData, 10, 50); */
	/* SCH_Add_Task(&sensorTest, 0, 100); */
	SCH_Add_Task(&testReboot, 0, 100);


	SCH_Start();
	while (1)
	{
		SCH_Dispatch_Tasks();
	}
	return 0;
}
