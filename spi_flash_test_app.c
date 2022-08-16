#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <linux/types.h>

#define DEVNAME		         "/dev/spidev1.0"

uint8_t rdid_cmd[4] = {0xAB, 0x01, 0x00, 0x00};
uint8_t device_id[4];
struct spi_ioc_transfer mesg[2];
uint32_t ioctl_spi_mode = 0;
int fd;

int open_file()
{
	fd = open (DEVNAME, O_RDWR);
	if (fd < 0 )
	{
		printf ("Cannot open SPI device file\n");
                return -1;
	}

	printf("fd = %x\n", fd);
	return 0;
}

int transfer(int fd, const uint8_t *tx_buf,unsigned tx_len,
        uint8_t *rx_buf, unsigned rx_len)
{
        int ret, i = 1;
        struct spi_ioc_transfer x[2];

        memset(x, 0, sizeof(x));

        x[0].tx_buf = (unsigned)tx_buf;
        x[0].len = tx_len;

        if (rx_buf) {
        x[1].rx_buf = (unsigned)rx_buf;
        x[1].len = rx_len;
        i++;
        }

        ret = ioctl(fd, SPI_IOC_MESSAGE(i), x);

        if (ret != tx_len + rx_len) {
                printf("spi transfer error: %d\n", ret);
                return -1;
        }

        if (rx_buf) {
                for (i = 0; i < rx_len; i++)
                        printf("%02X ", rx_buf[i]);
                printf("\n");
        }

        return 0;
}

int rdid(int fd)
{
        uint8_t tx[4] = {0xAB,0x01, 0x00, 0x00},  rx[3];
        return transfer(fd, tx, 4, rx, 1);
}

int wren(int fd) 
{
	uint8_t tx[1] = {0x06}; /* Write Enable */
	return transfer(fd, tx, 1, NULL, 0);
}

int main()
{
	int ret, i;
	
	open_file();
   	sleep(1);/*Initial power on delay for Tuner chip to initialize.*/

	printf("Communicating Flash..\n");

	/*SPI mode - 3, i.e. SPI_CPHA = 1 and SPI_CPOL = 1*/
	ioctl_spi_mode |= SPI_CPHA;
	ioctl_spi_mode |= SPI_CPOL;

	ret = ioctl(fd, SPI_IOC_WR_MODE32, &ioctl_spi_mode);
	if (ret == -1)
		perror("can't set spi mode");

	ret = ioctl(fd, SPI_IOC_RD_MODE32, &ioctl_spi_mode);
	if (ret == -1)
		perror("can't get spi mode");

	wren(fd);

	for(i = 0;i<50;i++)
	{
        	rdid(fd);
                usleep(5);
        }

#if 0
		mesg[0].tx_buf = (unsigned long)rdid_cmd;
		mesg[0].len = 4;
		mesg[0].cs_change = 0;

		mesg[1].rx_buf = (unsigned long)device_id;
		mesg[1].len = 2;
		mesg[1].cs_change = 1;

		while(1) {
		ret = ioctl(fd, SPI_IOC_MESSAGE(2), mesg);
		if( ret == -1) {
			perror("SPI_IOC_MESSAGE");
			return -1;
		}

		printf("Device ID be  - %x %x %x %x\n", device_id[0], device_id[1], device_id[2], device_id[3]);
		//usleep(100);
		}

		#if 0
		ret = write(fd, rdid_cmd, 4);
		printf("No of bytes written = %d\n", ret);
		ret = read(fd, &device_id, 1);
		printf("No of bytes read = %d\n", ret);
		#endif
#endif

	close(fd);

	return 0;
}

