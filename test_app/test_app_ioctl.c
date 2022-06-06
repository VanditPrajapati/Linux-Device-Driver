#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/ioctl.h>

#define WR_DATA _IOW('a','a',char *)
#define RD_DATA _IOR('a','b',char *)
#define SET_VARIABLE _IOW('a','c',int *)
#define GET_VARIABLE _IOR('a','d',int *)


int main()
{
	int fd, option;
	
	unsigned long int var;

	char *write_buf = (char *)malloc(500*sizeof(char));
	char *read_buf = (char *)malloc(500*sizeof(char));

	printf("Welcome to the Test APP for testing IOCTL function\n");
	
	fd=open("/dev/chardriver", O_RDWR);
	if(fd<0)
	{
		printf("Couldn't open the file\n");
		return -1;
	}

	while(1)
	{
		printf("Enter the operation to be performed:\n1. Write\n2. Read\n3. Set the value of variable\n4. Get the value of variable\n5. Exit\n");
		scanf("%d", &option);

		switch(option)
		{
			case 1:
				printf("Enter the data to send: \n");
				scanf("%[^\n]%*c", write_buf);
				printf("Data is writing...\n");
				ioctl(fd, WR_DATA, write_buf);
				printf("Data Written: %s\n", write_buf);
				break;

			case 2:
				printf("Reading Data..\n");
				ioctl(fd, RD_DATA, read_buf);
				printf("Data Read: %s\n", read_buf);
				break;
	
			case 3:
				printf("Enter the new value of variable: \n");
				scanf("%ld", &var);
				ioctl(fd, SET_VARIABLE, var);
				printf("New value of variable is: %ld\n", var);
				break;
				
			case 4:
				printf("Reading the value of variable\n");
				ioctl(fd, GET_VARIABLE, var);
				printf("The value of variable is: %ld\n", var);
				break;
	
			case 5:
				close(fd);
				exit(1);
				break;

			default:
				printf("Please enter the valid option number\n");
				break;
		}
	}
	close(fd);
}
