avr-gcc -mmcu=atmega32u4 $1.c -o $1.o
avr-objcopy -j .text -j .data -O ihex $1.o $1.hex
printf "USB poort aan het zoeken. Reset nu je controller.";
ls /dev/tty* > /tmp/1;
while [ -z $USB ]; do
	sleep 0.5;
	printf ".";
	ls /dev/tty* > /tmp/2;
	USB=`comm -13 /tmp/1 /tmp/2 | grep -o '/dev/tty.*'`;
	mv /tmp/2 /tmp/1;
done;
echo "Controller gevonden op USB: $USB";
sudo avrdude -pm32u4 -cavr109 -P $USB -e -U flash:w:$1.hex
