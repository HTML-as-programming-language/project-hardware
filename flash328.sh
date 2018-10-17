avr-gcc -mmcu=atmega328p $1.c -o $1.o
avr-objcopy -j .text -j .data -O ihex $1.o $1.hex
sudo avrdude -pm328p -cusbasp -e -U flash:w:$1.hex
