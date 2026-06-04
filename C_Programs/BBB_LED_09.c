/************************  To execute this Application Execute below Commands
******	./BBB_LED_09 trigger heartbeat
******	./BBB_LED_09 trigger oneshot
******	./BBB_LED_09 trigger none

******	./BBB_LED_09 brightness 1
******	./BBB_LED_09 brightness 8   
******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#define SYS_FS_LEDS_PATH  "/sys/class/leds"

#define USR_LED_NUMBER	3
#define SOME_BYTES	100

int write_trigger_values(uint8_t led_no, char *value)
{
	int fd, ret=0;
	char buf[SOME_BYTES];
	
	snprintf(buf, sizeof(buf),SYS_FS_LEDS_PATH, "beaglebone:green:usr%d/trigger",led_no);
	
	fd = open(buf, O_WRONLY);
	if(fd<0)
	{
		perror("Write Trigger Error\n");
		return -1;
	}
	return 0;
}

int write_brightness_values(uint8_t led_no, int value)
{
	int fd, ret=0;
	char buf[SOME_BYTES];
	snprintf(buf, sizeof(buf),SYS_FS_LEDS_PATH, "beaglebone:green:usr%d/brightness",led_no);
	
	fd = open(buf, O_WRONLY);
	if(fd<0)
	{
		perror("Write Trigger Error\n");
		return -1;
	}
	
	ret = write(fd, &value, sizeof(value));
	if(ret<0)
	{
		printf("Trigger Value Write Error\n");
		return -1;
	}
	return 0;
}

void process_trigger_values(char *value)
{
	if(strcmp(value, "timer")|| strcmp(value, "heartbeat")||\
		strcmp(value, "none")|| strcmp(value, "oneshot")||\
		strcmp(value, "default-on"))
	{
		write_trigger_values(USR_LED_NUMBER, value);
	}
	else
	{
		printf("Invalid Value\n");
		printf("Valid Trigger Value: heartbeat, timer, none, oneshot, default-on\n");
	}
}

void process_brightness_values(int value)
{
	switch(value)
	{
		case 1:
			write_brightness_values(USR_LED_NUMBER,value);
			break;
		
		case 0:
			write_brightness_values(USR_LED_NUMBER, value);
			break;
		default:
			printf("Invalid Value\n");
			printf("Valid Brightness Value: 0, 1 \n");
	}
}

int main(int argc, char *argv[])
{
	printf("This Application control the User LED 3\n");
	if(argc != 3)
	{
		printf("usage: %s <control option> <value>\n",argv[0]);
		printf("valid control options: brightness, Trigger \n");
		printf("Valid Brightness Value: 0,1\n");
		printf("Valid Trigger Values: heartbeat, timer, none, oneshot,default-on\n");
		
	}
	else
	{
		if(strcmp(argv[1],"trigger") == 0)
		{
			process_trigger_values(argv[2]);
		}
		else if(strcmp(argv[1],"brightness") == 0)
		{
			int value = atoi(argv[2]);
			process_brightness_values(value);
		}
		else
		{
			printf("Value is not correct\n");
		}
	}
	return 1;
}


