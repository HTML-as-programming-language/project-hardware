int getTemp()
{ //returnt de temperatuur in tienden van graden C
	int sensorInput = adc_read(0);
	// Adafruit over de TMP36:
	// Temp in C = (input(mv) - 500) / 10
	double temperatuur = (double)sensorInput / 1024;   // vind het percentage van de input
	temperatuur = temperatuur * 5;              // vermenigvuldig met 5V om het voltage te verkrijgen
	temperatuur = temperatuur - 0.5;            // Haal de offset eraf
	temperatuur = temperatuur * 1000;			// Converteer millivolt naar tienden van graden Celcius
	return(temperatuur);
}

void sensorTest(unsigned byte ch) //test de sensor op de gegeven input pin
{
	uint8_t x = adc_read(0);
	tx(x);
}