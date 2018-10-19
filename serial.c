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

uint8_t rx() {
loop_until_bit_is_set(UCSR0A, RXC0);
return UDR0;
}
