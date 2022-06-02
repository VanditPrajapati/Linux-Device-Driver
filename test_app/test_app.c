#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>

int8_t write_buf[1024];
int8_t read_buf[1024];

int main()
{
	int fd, option;

	printf("Welcome to the Char Driver Test APP\n");

	fd=open("/dev/chardriver", O_RDWR);
	if(fd<0)
	{
		printf("File could not be opened\n");
		printf("Test Application Exiting...\n");
		return -1;
	}

	while(1)
	{
		printf("\nEnter the operation to be performed:\n1. Write\n2. Read\n3. Exit\n");
		scanf("%d", &option);

		switch(option)
		{
			case 1:
				printf("Enter the Data to be entered: \n");
				scanf(" %[^\t\n]s", write_buf);
				printf("Data Writing\n");
				write(fd, write_buf, strlen(write_buf)+1);
				printf("Data Writing Done\n");
				break;

			case 2:
				printf("Data is Reading..\n");
				read(fd, read_buf, 1024);
				printf("Data Reading Done\n");
				printf("Data Read is: %s\n", read_buf);
				break;
			
			case 3:
				close(fd);
				exit(1);
				break;

			default:
				printf("Please enter proper option number\n");
				break;
		}
	}
	close(fd);
}

