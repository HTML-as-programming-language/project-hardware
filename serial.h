void uart_init(){
	UBRR0H = 0;
	UBRR0L = UBBRVAL;
	UCSR0B |= _BV(RXEN0) | _BV(TXEN0);
	UCSR0C |= _BV(UCSZ00) | _BV(UCSZ01);
}

void tx(uint8_t data) {
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = data;
}

void txChar(uint8_t message[]) {
	for (int c = 0; message[c] != 0; c++)
		tx(message[c]);

}

void txInt(int message) {
	//verzend een int (16 bit) in twee packets van 8 bits
	int a = (message % 0x10000);
	int b = (message - a);
	tx(a);
	tx(b);
}

uint8_t rx() {
	loop_until_bit_is_set(UCSR0A, RXC0);
	return UDR0;
}
