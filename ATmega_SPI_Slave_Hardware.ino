// Do not Loop the MOSI pin (5) on the MISO pin (1)!!!
// Upload the sketch
// Open the monitor

#define DEBUG

#define SPI_FRAME_SIZE 12
char SPI_Received[SPI_FRAME_SIZE];
char SPI_Send[SPI_FRAME_SIZE];
char SPI_Count;

char Count;


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
  //SPCR = 0x40; // Interrupt disable
  SPI_Count = 0;
  SPCR = 0xC0; // Interrupt enable

  // SPSR
  // SPIF : SPI Interrupt Flag : Reset : 0
  // SPI2X : fosc/4 = 16 Mhz/4 = 4 Mhz   : x //No effect on slave mode, but Master musn't exceed this speed. 
  SPSR = 0;

  Count = 0;
  
}  // end of setup

// SPI interrupt routine
ISR (SPI_STC_vect)
{
  //Send
  SPDR = SPI_Send[SPI_Count+1];

  //Receive
  SPI_Received[SPI_Count] = SPDR;

  SPI_Count++;
  
}  // end of interrupt routine SPI_STC_vect


// main loop - wait for flag set in interrupt routine
void loop (void)
{

  if( SPI_Count == SPI_FRAME_SIZE)
  {
#ifdef DEBUG  
    Serial.print( "Received : 0x");    
    for( SPI_Count=0; SPI_Count < SPI_FRAME_SIZE; SPI_Count++)
    {
      Serial.print( SPI_Received[SPI_Count] & 0xFF, HEX);    
    }      
    Serial.println( "" );    
#endif //DEBUG

    if( SPI_Received[0] == 'C')
    {
      Count = 0;   
    }

    for( SPI_Count=0; SPI_Count < SPI_FRAME_SIZE; SPI_Count++)
    {
      SPI_Send[SPI_Count] = Count++;
    }
    
#ifdef DEBUG  
    Serial.print( "Send : 0x");    
    for( SPI_Count=0; SPI_Count < SPI_FRAME_SIZE; SPI_Count++)
    {
      Serial.print( SPI_Send[SPI_Count] & 0xFF, HEX);    
    }      
    Serial.println( "" );    
#endif //DEBUG

    //prepare the next package
    SPI_Count = 0;
    SPDR = SPI_Send[SPI_Count];
 
  }



//  SPDR = 0x55; 
  // Wait for transmission complete
  // reset when read at '1'
//  while( !(SPSR & (1<<SPIF) ));

      
} // end of loop

void InitialiseInterrupt(){

  //Turn off global interrupt
  cli();    // switch interrupts off while messing with their settings
//noInterrupts  //  Do the same (To be analysed)              

//  EICRA = 0x2;          // Falling edge of INT0 generates an interrupt request
//  EICRB = 0xFF;          // Any edge of INT0 generates an interrupt request
  EIMSK = 0;          // Disable INTx interrupt enbale
  PCICR = 1;          // PCIE0 = 1 : Enable PCINT0 interrupt
  PCIFR = 1;          // PCIF0 = 1 : Clear interrupt flags PCIFR 

//  PCMSK0 = 0x80;         //Enable PCINT7 only !!!   
//  PCMSK0 = 0x82;         //Enable PCINT7 & PCINT1 only !!!   
  PCMSK0 = 0x88;         //Enable PCINT7 & PCINT3 only !!!   
//  PCMSK0 = 0x84;         //Enable PCINT7 & PCINT2 only !!!   

////  PCMSK1 = 0x2;         //PCINT1 15:8 on other board but Yun!!!
  
  
  sei();    // turn interrupts back on (also useable interrupts())
  
}//InitialiseInterrupt()
