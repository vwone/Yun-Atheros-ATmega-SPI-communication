import spidev
import time

spi = spidev.SpiDev()
spi.open(0,0)
spi.mode = 0b11
try:

while True :
	resp = spi.readbytes(1)
	if(resp[0] != 255)
	print(resp)
	time.sleep(1)

except KeyboardInterrupt
spi.close()

