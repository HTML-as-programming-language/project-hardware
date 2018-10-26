#define F_CPU 16E6
#include <avr/io.h>
#include <avr/interrupt.h>
#include "analog.h"
#include <avr/eeprom.h>
#include <util/delay.h>

#define BAUD 9600
#define UBBRVAL F_CPU/16/BAUD-1

#define LEDPin PIND5
#define TrigPin PIND3
#define EchoPin PIND2

volatile uint8_t pingState = 0;
volatile int centimeter = 0;

int tempOn = 250; // de temperatuur waarop het zonnescherm omlaag moet worden gedraaid, default is 25,0 graden
int tempOff = 200; // de temperatuur waarop het zonnescherm omhoog moet worden gedraaid, tempOn en tempOff worden vervangen als er een andere waarde in de eeprom staat
uint8_t screenPos = 0; // de postie van het zonnescherm. 0x00 = omhoog, 0xff = omlaag

#include "ttc.h"
#include "serial.h"
#include "sensor.h"

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

	OCR0A = (uint8_t)625;
	TCCR0B = (1 << CS02)|(1 << CS00);
	TCCR0A = (1 << WGM01);
	TIMSK0 = 1 << OCIE0A;
}

void SCH_Start(void)
{
	sei();
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
	centimeter = -1;
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
			{

				SCH_tasks_G[Index].Delay -= 1;
			}
		}
	}
}

void update_leds()
{
	PORTB ^= 0x1;
}


void initSensor()
{
	startPacket();
	tx(init);
	tx(0x00);
	tx(0x00);
}

void sendData()
{
	union {uint16_t ValInt; unsigned char Bytes[2];} tempInt;
	tempInt.ValInt = getTemp(); // meet de temperatuur
	startPacket();
	tx(temp); //verteld dat het om het doorgeven van de temperatuur gaat
	tx(tempInt.Bytes[1]); //geeft de temperatuur door
	tx(tempInt.Bytes[0]);
}

void handleRx()
{
          //leest lastMessage en neemt de bijbehorede acties 
          int command = ((lastMessage.Bytes[0] * 0x100) + lastMessage.Bytes[1]); //het commando (bovenste 16 bits
          int payload = ((lastMessage.Bytes[2] * 0x100) + lastMessage.Bytes[3]); //de payload van het bericht
          switch(command)
          {
                  case 11: 
                          tempOn = payload;
                          break; 
                  case 12: 
                          tempOff = payload;
                          break;
                  case 51:  
                          setScreen(0xff);
                          break;  
                  case 52:  
                          setScreen(0x00);
        }
 }

void setScreen(uint8_t pos)
{
	// veranderd de positie van het zonnescherm. pos: 0xff = omlaag, 0x00 = omhoog
}

void checkScreenPos()
{
	int temp = getTemp();
	if (temp <= tempOff)
	{
		setScreen(0x00); //draai het scherm omhoog
	}
	else if (temp >= tempOn)
	{
		setScreen(0xff); // draai het scherm naar beneden
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

union u_type
{
	uint32_t IntVar;
	unsigned char Bytes[4];
}
txtestbyte;

void txtest()
{
	txtestbyte.Bytes[0] = rx();
	txtestbyte.Bytes[1] = rx();
	txtestbyte.Bytes[2] = rx();
	txtestbyte.Bytes[3] = rx();
	tx(txtestbyte.Bytes[0]);
	tx(txtestbyte.Bytes[1]);
	tx(txtestbyte.Bytes[2]);
	tx(txtestbyte.Bytes[3]);
}

void txLight() {
	txInt(getLight());
}

int main()
{
	SPH = (RAMEND & 0xFF00) >> 8;
	SPL = (RAMEND & 0x00FF);

	TCCR1B = (1<<CS10);
	TIMSK1 = (1<<TOIE1);

	PCICR = (1<<PCIE2);
	PCMSK2 = (1<<PCINT18);

	DDRD = (1<<TrigPin) | (1<<LEDPin);
	TCNT1 = 0;
	incReboot();
	uart_init();
	DDRB = 1 << 0;
	adc_init();
	SCH_Init_T1();

	/* SCH_Add_Task(&initSensor, 0, 0); */
	//SCH_Add_Task(&sendData, 0, 50);
	/* SCH_Add_Task(&sensorTest, 0, 50); */
	/* SCH_Add_Task(&txtest, 0, 50); */
	// SCH_Add_Task(&checkRx, 0, 50);
	/* SCH_Add_Task(&update_leds, 0, 50); */
	/* SCH_Add_Task(&ultrasoon, 0, 5); */
	/* SCH_Add_Task(&testReboot, 0, 100); */
	SCH_Add_Task(&txLight, 0, 100);

	SCH_Start();
	while (1)
	{
		SCH_Dispatch_Tasks();
	}
	return 0;
}
