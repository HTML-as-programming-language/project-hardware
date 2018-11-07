union
{
	unsigned char Bytes[2];
	uint16_t IntVar;
} message;

void tx( unsigned char data )
{
	while ( !( UCSR0A & (1<<UDRE0)) );
	UDR0 = data;
}

void txChar(uint8_t message[])
{
	for (int c = 0; message[c] != 0; c++)
		tx(message[c]);
}

void startPacket()
{
	tx(0xff);
}

void sendPacket(int command, int payload)
{
	startPacket();
	tx(command);
	tx(payload);
}
