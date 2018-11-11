avr-gcc -Ofast -Os -mmcu=atmega328 $1.c -o $1.o
avr-objcopy -j .text -j .data -O ihex $1.o $1.hex
sudo avrdude -p m328p -c arduino -P /dev/ttyACM0 -e -U flash:w:$1.hex
# sudo avrdude -C /etc/avrdude.conf -c avrisp -P /dev/ttyACM0 -b 115200 -p m328p -U flash:w:$1.hex
