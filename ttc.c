#define F_CPU 16E6
#include "ttc.h"
#include <avr/io.h>
#include <stdlib.h>
#include <avr/sfr_defs.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#define UBBRVAL 103
#include "serial.c"
//#define UBBRVAL 103 //set baudrate to 9600

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
	// Breadboard versie
	// OCR1A = (uint16_t)625;
	// TCCR1B = (1 << CS12) | (1 << WGM12);
	OCR1A = (uint16_t)1300;
	TCCR1B = (1 << CS11) | (1 << WGM12);
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

void screenup() {
	//code om het scherm omhoog te draaien
}

void screendown(){
	//code om het zonnescherm naar beneden te draaien
}

void readSensors() {
	//leest de sensoren en schrijft naar globale variabelen
}

void sendData() {
	//verzend data van sensoren naar de centrale
}

int main()
{
	uart_init();
	tx(0x24);
	DDRD = 1 << 1;
	DDRB = 0;
	SCH_Init_T1();
	SCH_Add_Task(update_leds, 0, 50);
	SCH_Start();
	while (1)
	{
		SCH_Dispatch_Tasks();
	}
	return 0;
}
