void uart_init(){
	UBRR0H = 0;
	UBRR0L = UBBRVAL;
	UCSR0B |= _BV(RXEN0) | _BV(TXEN0);
	UCSR0C |= _BV(UCSZ00) | _BV(UCSZ01);
}

void tx( unsigned char data )
{
	while ( !( UCSR0A & (1<<UDRE0)) );
	UDR0 = data;
}

void txChar(uint8_t message[]) {
	for (int c = 0; message[c] != 0; c++)
		tx(message[c]);

}

void txInt(int message) {
 	//verzend een int (16 bit) in twee packets van 8 bits
	union {uint16_t ValInt; unsigned char Bytes[2];} MessInt;
	MessInt.ValInt = message;
 	tx(MessInt.Bytes[1]);
	tx(MessInt.Bytes[0]);
}

unsigned char rx( void )
{
	while ( !(UCSR0A & (1<<RXC0)) );
	return UDR0;
}
