union
{
	uint16_t IntVar;
	unsigned char Bytes[2];
}
firstInt;

checkRx()
{ //checkt of er een bericht is binnengekomen op rx en schrijft het naar een variabele
	firstInt.Bytes[0] = rx(); //schrijft het bericht naar de bovenste helft van een int
	firstInt.Bytes[1] = rx(); //alle berichten bestaan uit 16 bits, hier word de tweede helft geschreven
	if (firstInt.IntVar == 0xffff)
	{ //0xffff betekend dat het het begin is van een bericht is, de rest van het bericht wordt nu naar een variabele geschreven
		tx(0x11);
		lastMessage.Bytes[0] = rx();
		lastMessage.Bytes[1] = rx();
		lastMessage.Bytes[2] = rx();
		lastMessage.Bytes[3] = rx();
		handleRx();
	}
}

void handleRx()
{
          //leest lastMessage en neemt de bijbehorede acties 
          int command = ((lastMessage.Bytes[0] * 0x100) + lastMessage.Bytes[1]); //het commando (bovenste 16 bits
          int payload = ((lastMessage.Bytes[2] * 0x100) + lastMessage.Bytes[3]); //de payload van het bericht
          switch(command)
          {
                  case 11: 
                          tempOn = payload;
                          break; 
                  case 12: 
                          tempOff = payload;
                          break;
                  case 51:  
                          setScreen(0xff);
                          break;  
                  case 52:  
                          setScreen(0x00);
        }
 }
