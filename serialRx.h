void update_leds(int status);

int aa = 0;
int bb = 0;
int cc = 0;
int dd = 0;
int ee = 0;

union
{
	uint16_t IntVar;
	unsigned char Bytes[2];
} Data;

unsigned char rx( void )
{
	while ( !(UCSR0A & (1<<RXC0)) );
	return UDR0;
}

void handleRx()
{
	//leest Data en neemt de bijbehorede acties
	uint8_t command = Data.Bytes[0];
	uint8_t payload = Data.Bytes[1];
	switch(command)
	{
		case 11:
			eeprom_write_byte(&tempOn, payload);
			break;
		case 12:
			eeprom_write_byte(&tempOff, payload);
			break;
		case 13:
			eeprom_write_byte(&lightOn, payload);
			break;
		case 14:
			eeprom_write_byte(&lightOff, payload);
			break;
		case 51:
			update_leds(1);
			break;
		case 52:
			update_leds(0);
			break;
		case 53:
			manual = payload;
			break;
	}
}

int id = 0;
int pid = 0;
void buffer(uint8_t data)
{
	if (data == 0xff)
	{
		id = 1;
	}
	else if (id)
	{
		id = 0;
		pid = data;
	}
	else if (pid)
	{
		Data.Bytes[0] = pid;
		Data.Bytes[1] = data;
		pid = 0;
		handleRx();
	}
}
