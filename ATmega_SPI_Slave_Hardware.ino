// Do not Loop the MOSI pin (5) on the MISO pin (1)!!!
// Upload the sketch
// Open the monitor

//#define DEBUG

char Received;
char count=0x1;

void setup (void)
{
  /*---------------------------------------------------------------*/
  /* µC <=> Monitor serial communication.                          */
  /*---------------------------------------------------------------*/
#ifdef DEBUG
  Serial.begin( 115200 ); // Serial speed 115200 bauds
  while(!Serial);
  Serial.println( "Let's go!" );
#endif //DEBUG

  // have to send on master in, *slave out*

  // Set 
  // MOSI (PB2) : input  : 0 
  // SCK  (PB1) : input  : 0
  // SS   (PB0) : input  : 0
  // MISO (PB3) : output : 1
  DDRB = (1<<DDB3);

  // SPCR
  // SPIE : Interrupt Enable : disable   : 0
  // SPE  : SPI enable       : Enable    : 1
  // DORD : Data Order       : MSB First : 0
  // MSTR : Mode             : Slave     : 0
  // CPOL                                : 0
  // CPHA                                : 0
  // SPR1                                : x
  // SPR0 : fosc/4 = 16 Mhz/4 = 4 Mhz    : x  //No effect on slave mode
  SPCR = 0x40; // Interrupt disable
  //SPCR = 0xC0; Interrupt enable

  // SPSR
  // SPIF : SPI Interrupt Flag : Reset : 0
  // SPI2X : fosc/4 = 16 Mhz/4 = 4 Mhz   : x //No effect on slave mode, but Master musn't exceed this speed. 
  SPSR = 0;
  
}  // end of setup


// SPI interrupt routine
ISR (SPI_STC_vect)
{
  Serial.print( "Received :");    
  Serial.println( Received, HEX);
}  // end of interrupt routine SPI_STC_vect

// main loop - wait for flag set in interrupt routine
void loop (void)
{
  //Increment return value
  SPDR = count++;
  
  // Wait for transmission complete
  // reset when read at '1'
  while( !(SPSR & (1<<SPIF) ));

  Received = SPDR;

  if(Received == 'C')
  {
    count = 0;    
  }
  
#ifdef DEBUG  
  Serial.print( "Received : 0x");    
  Serial.println( Received & 0xFF, HEX );    
#endif //DEBUG
    
} // end of loop
