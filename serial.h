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
	int a = (message / 0x100);                                                                                                                                                                                                                                         
 	int b = (message % 0x100);                                                                                                                                                                                                                                         
 	tx(a);                                                                                                                                                                                                                                               
	tx(b);
}

unsigned char rx( void )
{
	while ( !(UCSR0A & (1<<RXC0)) );
	return UDR0;
}
