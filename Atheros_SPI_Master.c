/* 
# Added this section to your /etc/rc.local
# GPIO_21 is connected to the Output enable of the level shifter.
# GPIO_26 is connected to the slave select pin through the level shifter.
#         it isn't automatically assereted. 
#
echo 26 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio26/direction
*/
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

//#define DEBUG

// Level shifters output enable = GPIO_21
#define LEVEL_SHIFTER_ENABLE "/sys/class/gpio/gpio21/value"
#define GPIO_21 21

// Slave Select pin = GPIO_26
#define SLAVE_SELECT "/sys/class/gpio/gpio26/value"
#define GPIO_26 26

static const char *device = "/dev/spidev1.0"; // LEDE Distribution
//static const char *device = "/dev/ttySPI0"; // Linino distribution
static uint8_t mode = 0;
static uint8_t bits = 8;
//Atmega at 16 Mhz => 4 Mhz is the max speed.
//static uint32_t speed = 4000000;
static uint32_t speed = 100000;
//static uint16_t delay = 1000; // works fine with cs_change = 0 or 1
//static uint16_t delay = 100; // works fine with cs_change = 0 or 1
static uint16_t delay = 0; // works fine with cs_change = 0 or 1

static void pabort(const char *s);

int Export_GPIO(unsigned int GPIO_Num, char *Direction);
static int Set_GPIO(char Path[], char Value[]);

static int transfer(int fd, struct spi_ioc_transfer *transfer_stuct, unsigned char *Pt_Data);
static int transfer2(int fd, struct spi_ioc_transfer *transfer_stuct);

//'C' Clear arduino counter at first emission
unsigned char Send_Buf[13]={'C',0,0,0,0,0,0,0,0,0,0,0,0};
unsigned char Receive_Buf[13]={0,0,0,0,0,0,0,0,0,0,0,0,0};

unsigned char Receive[13]={0,0,0,0,0,0,0,0,0,0,0,0,0};

unsigned char Data;
unsigned char n;

int main (int argc, char* argv[])
{
	int ret = 0;
	int fd;

	//ioctl structure
	
	struct spi_ioc_transfer transfer_stuct_tab[12];

	for (n = 0; n < 12 ; n++)
	{
		memset(&transfer_stuct_tab[n], 0, sizeof(transfer_stuct_tab[n]));
		transfer_stuct_tab[n].tx_buf = (unsigned long) &Send_Buf[n];
		transfer_stuct_tab[n].rx_buf = (unsigned long) &Receive_Buf[n];
		transfer_stuct_tab[n].len = 1;
		transfer_stuct_tab[n].delay_usecs = delay;
//		transfer_stuct_tab[n].cs_change = 0;
		transfer_stuct_tab[n].cs_change = 1;
		transfer_stuct_tab[n].speed_hz = speed;
		transfer_stuct_tab[n].bits_per_word = bits;

	}

	
	struct spi_ioc_transfer transfer_stuct = 
	{
		.tx_buf = (unsigned long) Send_Buf,
		.rx_buf = (unsigned long) Receive_Buf,
		.len = 1,
		.delay_usecs = delay,
		.cs_change = 1,
		.speed_hz = speed,
		.bits_per_word = bits,
	};
	
	// Export GPIO
	Export_GPIO(GPIO_26, "out" );
	//Fails due to the absence of the "direction" file in the gpio21 directory ?!
	//Export_GPIO(GPIO_21, "out" );
	
	// Output disable
	Set_GPIO(LEVEL_SHIFTER_ENABLE, "1");

	fd = open(device, O_RDWR);
	if (fd < 0)
		pabort("can't open device");

	/*
	 * spi mode
	 */
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		pabort("can't set spi mode");

	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		pabort("can't get spi mode");

	/*
	 * bits per word
	 */
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't set bits per word");

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't get bits per word");

	/*
	 * max speed hz
	 */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't set max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't get max speed hz");

	printf("spi mode: %d\n", mode);
	printf("bits per word: %d\n", bits);
	printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);
/*
	//Clear AVR Counter
	Data = 'C';
	
	if( transfer( fd, &transfer_stuct, &Data) < 0)
	{
		pabort("Transfert failed !!!");
	}
	
	sleep(1);
	
	//Verify AVR Counter
	Data = 'C';
	
	if( transfer( fd, &transfer_stuct, &Data) < 0)
	{
		pabort("Transfert failed !!!");
	}

	if( Data == 0)
	{
		printf("Reset succeeded : 0x%x\n", Data);
	}
	else
	{
		printf("Reset failed !!! : 0x%x\n", Data);		
	}
	
	sleep(1);

	//Test AVR link and counter value [1:255]
	for(n=0; n<255; n++)
	{
		Data = 'R';
		
		if( transfer( fd, &transfer_stuct, &Data) < 0)
		{
			pabort("Transfert failed !!!");
		}
		
		if( Data != n)
		{
			printf("Counter Error!!! : n 0x%x : Data : 0x%x\n", n, Data);
		}

#ifdef DEBUG		
		else
		{
			printf("Counter loop : n 0x%x : Data : 0x%x\n", n, Data);			
		}
		
		sleep(1);
#endif //DEBUG
		
	}//Counter

	n--;
	printf("Counter loop : n 0x%x : Data : 0x%x\n", n, Data);			
*/
/*
	
	for(n=0; n<12; n++)
	{
		Data = 'C';
		
		if( transfer( fd, &transfer_stuct, &Data) < 0)
		{
			pabort("Transfert failed !!!");
		}
		
		Receive[n] = Data;
	}

	printf("Received : 0x%x%x%x%x 0x%x%x%x%x 0x%x%x%x%x\n",
		Receive[0],
		Receive[1],
		Receive[2],
		Receive[3],
		Receive[4],
		Receive[5],
		Receive[6],
		Receive[7],
		Receive[8],
		Receive[9],
		Receive[10],
		Receive[11]);
*/
/*
	if( transfer2( fd, &transfer_stuct[0]) < 0)
	{
		pabort("Transfer failed !!!");
	}
*/	
	// Slave asserted
   	Set_GPIO(SLAVE_SELECT, "0");

//   if (ioctl(fd, SPI_IOC_MESSAGE(1), &transfer_stuct)<0)
//   if (ioctl(fd, SPI_IOC_MESSAGE(12), &transfer_stuct)<0)
   if (ioctl(fd, SPI_IOC_MESSAGE(12), &transfer_stuct_tab[0])<0 )
   {
      perror("Failed to send SPI message");
	  return -1;
   }
   
	// Slave negated
  	Set_GPIO(SLAVE_SELECT, "1");

	printf("Received : 0x%x%x%x%x 0x%x%x%x%x 0x%x%x%x%x\n",
		Receive_Buf[0],
		Receive_Buf[1],
		Receive_Buf[2],
		Receive_Buf[3],
		Receive_Buf[4],
		Receive_Buf[5],
		Receive_Buf[6],
		Receive_Buf[7],
		Receive_Buf[8],
		Receive_Buf[9],
		Receive_Buf[10],
		Receive_Buf[11]);

	
	close(fd);

	// Output disable
	Set_GPIO(LEVEL_SHIFTER_ENABLE, "0");
	
	return 0;
	
}//main()

static int transfer(int fd, struct spi_ioc_transfer *transfer_stuct, unsigned char *Pt_Data)
{
	/* Transfer type 1*/
   Send_Buf[0]= *Pt_Data;

	// Slave asserted
   	Set_GPIO(SLAVE_SELECT, "0");

//   if (ioctl(fd, SPI_IOC_MESSAGE(1), transfer_stuct)<0){
   if (ioctl(fd, SPI_IOC_MESSAGE(12), transfer_stuct)<0){
      perror("Failed to send SPI message");
	  return -1;
   }
   
	// Slave negated
  	Set_GPIO(SLAVE_SELECT, "1");
	
	*Pt_Data = Receive_Buf[0];
	
	return 0;
	
}//transfer()

static int transfer2(int fd, struct spi_ioc_transfer *transfer_stuct)
{

	// Slave asserted
   	Set_GPIO(SLAVE_SELECT, "0");

//   if (ioctl(fd, SPI_IOC_MESSAGE(1), transfer_stuct)<0){
   if (ioctl(fd, SPI_IOC_MESSAGE(12), transfer_stuct)<0){
      perror("Failed to send SPI message");
	  return -1;
   }
   
	// Slave negated
  	Set_GPIO(SLAVE_SELECT, "1");
		
	return 0;
	
}//transfer()


int Set_GPIO(char Path[], char Value[])
{
	FILE *fp;
	
	fp = fopen(Path, "w+");	
	fprintf( fp, "%s", Value);
	
	fclose(fp);
	
	return 0;
	
}//Set_GPIO()

int Export_GPIO(unsigned int GPIO_Num, char *Direction)
{
	FILE *fp;
	char Path[255];

	//Export ?
	sprintf( Path, "/sys/class/gpio/gpio%u", GPIO_Num);
	printf("Exporting : %s\n", Path);

	//File exits?
//	if(!fileexists(Path))
//	{
//		fp = fopen("/sys/class/gpio/export", "w+");	
//		fprintf( fp, "%u", GPIO_Num);		
//		fclose(fp);		
//	}

	if( (fp = fopen(Path, "w")) == NULL )
	{
		printf("File doesn't exist : %s\n", Path);
		fp = fopen("/sys/class/gpio/export", "w");	
		if (fp != NULL)
		{
			printf("/sys/class/gpio/export open\n", GPIO_Num);			
		}
		else
		{
			printf("Failed to open : /sys/class/gpio/export\n", GPIO_Num);						
		}
		
		printf("/sys/class/gpio/export > %u\n", GPIO_Num);
		fprintf( fp, "%u\n", GPIO_Num);		
		fclose(fp);		
	}
	else
	{
		printf("File exists : %s\n", Path);
		fclose(fp);
	}
	
	//Set Direction
	sprintf( Path, "/sys/class/gpio/gpio%u/direction", GPIO_Num);
	printf("Setting direction : %s as %s\n", Path, Direction);

	fp = fopen(Path, "w+");	
	printf(Direction);
	fprintf( fp, "%s\n", Direction);		
	fclose(fp);		
	
	return 0;
	
}//Export_GPIO()

#define LEVEL_SHIFTER_ENABLE "/sys/class/gpio/gpio21/value"


static void pabort(const char *s)
{
	perror(s);
	abort();
}//pabort()



