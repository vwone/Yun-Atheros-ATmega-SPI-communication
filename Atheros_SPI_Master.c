/* 
# Added this section to your /etc/rc.local
# GPIO_21 is connected to the Output enable of the level shifter.
# GPIO_26 is connected to the slave select pin through the level shifter.
#         it isn't automatically assereted. 
#
echo 26 > /sys/class/gpio/export
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

// Slave Select pin = GPIO_26
#define SLAVE_SELECT "/sys/class/gpio/gpio26/value"

static const char *device = "/dev/spidev1.0";
static uint8_t mode = 0;
static uint8_t bits = 8;
//Atmega at 16 Mhz => 4 Mhz is the max speed.
static uint32_t speed = 4000000;
static uint16_t delay;

static void pabort(const char *s);
static int Set_GPIO(char Path[], char Value[]);
static int transfer(int fd, struct spi_ioc_transfer *transfer_stuct, unsigned char *Pt_Data);

unsigned char send[2], received[2];
unsigned char Data;
unsigned char n;

int main (int argc, char* argv[])
{
	int ret = 0;
	int fd;

	//ioctl structure
	struct spi_ioc_transfer transfer_stuct = 
	{
		.tx_buf = (unsigned long) send,
		.rx_buf = (unsigned long) received,
		.len = 1,
		// .delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};
		
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

	close(fd);

	// Output disable
	Set_GPIO(LEVEL_SHIFTER_ENABLE, "0");
	
	return 0;
	
}

static int transfer(int fd, struct spi_ioc_transfer *transfer_stuct, unsigned char *Pt_Data)
{
	/* Transfer type 1*/
   send[0]= *Pt_Data;

	// Slave asserted
   	Set_GPIO(SLAVE_SELECT, "0");

   if (ioctl(fd, SPI_IOC_MESSAGE(1), transfer_stuct)<0){
      perror("Failed to send SPI message");
	  return -1;
   }
   
	// Slave negated
  	Set_GPIO(SLAVE_SELECT, "1");
	
	*Pt_Data = received[0];
	
	return 0;
	
}//transfer()


int Set_GPIO(char Path[], char Value[])
{
	FILE *fp;
	
	fp = fopen(Path, "w+");	
	fprintf( fp, "%s", Value);
	
	fclose(fp);
	
	return 0;
	
}//Write_GPIO()

static void pabort(const char *s)
{
	perror(s);
	abort();
}//pabort()



