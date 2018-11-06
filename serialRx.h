void update_leds(int status);

int aa = 0;
int bb = 0;
int cc = 0;
int dd = 0;
int ee = 0;

union
{
	uint32_t IntVar;
	unsigned char Bytes[4];
} Data;

union
{
	uint16_t IntVar;
	unsigned char Bytes[2];
} Begin;

unsigned char rx( void )
{
	while ( !(UCSR0A & (1<<RXC0)) );
	return UDR0;
}

void handleRx()
{
	//leest Data en neemt de bijbehorede acties
	int command = ((Data.Bytes[0] * 0x100) + Data.Bytes[1]); //het commando (bovenste 16 bits)
	int payload = ((Data.Bytes[2] * 0x100) + Data.Bytes[3]); //de payload van het bericht
	switch(command)
	{
		case 11:
			// tempOn = payload;
			eeprom_write_word(&tempOn, payload);
			break;
		case 12:
			// tempOff = payload;
			eeprom_write_word(&tempOff, payload);
			break;
		case 51:
			// setScreen(0xff);
			update_leds(1);
			manual = 1;
			break;
		case 52:
			manual = 0;
			// setScreen(0x00);
			// update_leds(0);
			break;
	}
}

void buffer(char data)
{
	if (!aa)
	{
		Begin.Bytes[0] = data;
		aa = 1;
	}
	else if (!bb)
	{
		Begin.Bytes[1] = data;
		bb = 1;

		if (!(Begin.IntVar == 0xffff))
		{
			aa = 0;
			bb = 0;
		}
	}
	else if (!cc)
	{
		Data.Bytes[0] = data;
		cc = 1;
	}
	else if (!dd)
	{
		Data.Bytes[1] = data;
		dd = 1;
	}
	else if (!ee)
	{
		Data.Bytes[2] = data;
		ee = 1;
	}
	else
	{
		Data.Bytes[3] = data;

		aa = 0;
		bb = 0;
		cc = 0;
		dd = 0;
		ee = 0;
		handleRx();
	}
}
