union
{
	unsigned char Bytes[4];
	uint16_t Ints[2];
} message;

union
{
	uint16_t ValInt;
	unsigned char Bytes[2];
} MessInt;

void tx( unsigned char data )
{
	while ( !( UCSR0A & (1<<UDRE0)) );
	UDR0 = data;
}

void txChar(uint8_t message[])
{
	for (int c = 0; message[c] != 0; c++)
	{
		tx(message[c]);
	}

}

void startPacket()
{
	tx(0xff);
	tx(0xff);
}

void sendPacket(int command, int payload)
{
	message.Ints[1] = command;
	message.Ints[0] = payload;
	startPacket();
	tx(message.Bytes[3]);
	tx(message.Bytes[2]);
	tx(message.Bytes[1]);
	tx(message.Bytes[0]);
}
