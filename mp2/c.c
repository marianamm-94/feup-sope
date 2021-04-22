#include <stdio.h>
#include <string.h>


#include <time.h>

int nsec;
char fifoname[256];
time_t start;


int processInput(int argc, char *argv[])
{
	if(argc!=4)
		return -1;
	if(strcmp(argv[1], "-t")!=0)
		return -1;
	if(atoi(argv[2])<=0)
		return -1;
	else
		nsec = atoi(argv[2]);
	strncpy(fifoname, argv[3], sizeof(fifoname));
	
	return 0;
}
int main(int argc, char *argv[])
{

	if(processInput(argc, argv))
	{
		perror("Invalid arguments!");
		exit(-1);
	}

	start = time(NULL);
	while ((time(NULL)-start)<nsec)
	{
		//create threads que têm como argumento o id que vai incrementando
	}
	

}
