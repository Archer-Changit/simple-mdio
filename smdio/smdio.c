#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>

/* a smiple MDIO char device */
#define SIMPLE_MDIO_DEVICE "/dev/smdio"

#define SMDIO_MAGIC 's'
#define SMDIO_READ_PHY_REG   _IOWR(SMDIO_MAGIC, 1, struct mdio_reg_msg)
#define SMDIO_WRITE_PHY_REG  _IOW (SMDIO_MAGIC, 2, struct mdio_reg_msg)

#define SIMPLE_MDIO_NAME_SIZE 16

struct mdio_reg_msg
{
	char mdioBus[SIMPLE_MDIO_NAME_SIZE];

    int phyAddr;
	int mmd;
    uint16_t regAddr;
    uint16_t regValue;
};

static void __smdio_usage(void)
{
	printf("SYNOPSIS\r\n"
	      "  smdio r [mdiobus] [phyaddr] [mmd] [regaddr]\r\n"
	      "  smdio w [mdiobus] [phyaddr] [mmd] [regaddr] [value]\r\n"
		  "\r\n"
		  "  example:\r\n"
	      "  smdio r stmmac-1 0x01 0x1e 0x0180\r\n"
	      "  smdio w stmmac-1 0x01 0x1e 0x0180 0x0000\r\n"
	      "\r\n");

	return;
}

int main(int argc, char **argv)
{
    int fd;
    struct mdio_reg_msg msg;

    int opt;

	while ((opt = getopt(argc, argv, "h")) != -1)
	{
		switch (opt)
		{
		case 'h':
			__smdio_usage();

			return 0;

		default:
			break;
		}
	}

    fd = open(SIMPLE_MDIO_DEVICE, O_RDWR);
    if (fd < 0)
    {
        printf("Failed to open device(%s).\r\n", SIMPLE_MDIO_DEVICE);

        return -1;
    }

    if ((argc == 6) && (strcmp(argv[1], "r") == 0))
    {
        strncpy(msg.mdioBus, argv[2], SIMPLE_MDIO_NAME_SIZE - 1);
        msg.phyAddr = strtol(argv[3], NULL, 16);
        msg.mmd     = strtol(argv[4], NULL, 16);
        msg.regAddr = strtoul(argv[5], NULL, 16);
        if (ioctl(fd, SMDIO_READ_PHY_REG, &msg) < 0)
        {
            printf("ioctl read failed(%s).\r\n", strerror(errno));
        }
        else
        {
            printf("Read PHY[%d] MMD[0x%04x] Reg[0x%04x] = 0x%04x.\r\n", 
                                msg.phyAddr, msg.mmd, msg.regAddr, msg.regValue);
        }
    }
    else if ((argc == 7) && (strcmp(argv[1], "w") == 0))
    {
        strncpy(msg.mdioBus, argv[2], SIMPLE_MDIO_NAME_SIZE - 1);
        msg.phyAddr  = strtol(argv[3], NULL, 16);
        msg.mmd      = strtol(argv[4], NULL, 16);
        msg.regAddr  = strtoul(argv[5], NULL, 16);
        msg.regValue = strtoul(argv[6], NULL, 16);
        if (ioctl(fd, SMDIO_WRITE_PHY_REG, &msg) < 0)
        {
            printf("ioctl write failed(%s).\r\n", strerror(errno));
        }
        else
        {
            printf("Write PHY[%d] MMD[0x%04x] Reg[0x%04x] = 0x%04x.\r\n", 
                                msg.phyAddr, msg.mmd, msg.regAddr, msg.regValue);
        }
    }
    else
    {
        __smdio_usage();
    }

    close(fd);

    return 0;
}