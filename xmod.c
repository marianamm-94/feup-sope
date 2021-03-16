#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <math.h>

int parent;
char* dir;

int convertDecimalToOctal(int decimalNumber)
{
    int octalNumber = 0, i = 1;

    while (decimalNumber != 0)
    {
        octalNumber += (decimalNumber % 8) * i;
        decimalNumber /= 8;
        i *= 10;
    }

    return octalNumber;
}

long long convertOctalToDecimal(int octalNumber)
{
    int decimalNumber = 0, i = 0;

    while(octalNumber != 0)
    {
        decimalNumber += (octalNumber%10) * pow(8,i);
        ++i;
        octalNumber/=10;
    }

    i = 1;

    return decimalNumber;
}

int globalPID;

struct info
{
int octal;
int optionV;
int optionC;
int optionR;
char* fileOrDir;
int add;
int sub;
int replace;
int number;
};


void processInput(struct info* inf, char*option, char*mode, char* name)
{
inf->optionV=0;
inf->optionC=0;
inf->optionR=0;
inf->add=0;
inf->sub=0;
inf->replace=0;
inf->number=0;

if(strcmp(option,"-V")==0 || strcmp(option,"-v")==0)
	inf->optionV=1;
if(strcmp(option,"-C")==0 || strcmp(option,"-c")==0)
	inf->optionC=1;
if(strcmp(option,"-R")==0 || strcmp(option,"-r")==0)
	inf->optionR=1;
	
if(mode[0]=='0'){
	inf->octal = strtol(mode,0,8);
	inf->replace=1;
}
	
else{
int i=0;
int r=0;
int w=0;
int x=0;
int n=0;
	while(mode[i]!='\0')
	{
		if(mode[i]=='r')
			r=1;
		if(mode[i]=='w')
			w=1;
		if(mode[i]=='x')
			x=1;
		i++;
	}
	if(mode[0]=='u' || mode[0]=='a')
		n+=100*(4*r+2*w+x);
	if(mode[0]=='g' || mode[0]=='a')
		n+=10*(4*r+2*w+x);
	if(mode[0]=='o' || mode[0]=='a')
		n+=4*r+2*w+x;
	inf->octal=n;
	if(mode[1]=='=')
		inf->replace=1;
	else if(mode[1]=='+')
	{
		inf->add=1;
		inf->number=convertOctalToDecimal(inf->octal); //fazer operador | com este número e o octal lido em decimal e passar o resultado para octal;
	}
	else if(mode[1]=='-')
	{
		inf->sub=1;
		inf->number =  convertOctalToDecimal(777-inf->octal); //fazer operador & com este número e o octal lido em decimal e passar o resultado para octal;   
	}

}

}


void func(int ai){

//if(getpid()==parent)
	printf("ID: %d ; Diretório: %s\n",getpid(),dir);
	exit(0);
}

int process_permission(struct stat st){
	int perm = 0;

	if (st.st_mode & S_IRUSR) perm += 4*64; //r
	if (st.st_mode & S_IWUSR) perm += 2*64; //w
	if (st.st_mode & S_IXUSR) perm += 1*64; //x
	if (st.st_mode & S_IRGRP) perm += 4*8;  //r
	if (st.st_mode & S_IWGRP) perm += 2*8;  //w
	if (st.st_mode & S_IXGRP) perm += 1*8;  //x
	if (st.st_mode & S_IROTH) perm += 4;   //r
	if (st.st_mode & S_IWOTH) perm += 2;   //w
	if (st.st_mode & S_IXOTH) perm += 1;   //x
	
	return perm;
}
int changePermission(struct stat path_stat, struct info* inf, char *path_string){
int oldPer=process_permission(path_stat), newPer;
			if(inf->replace){
				newPer=inf->octal;
				chmod(path_string,newPer);
			}else if(inf->add){
				newPer=convertOctalToDecimal(oldPer) | inf->number;
				newPer=convertDecimalToOctal(newPer);
				chmod(path_string,newPer);
			}else if(inf->sub){
				newPer=convertOctalToDecimal(oldPer) & inf->number;
				newPer=convertDecimalToOctal(newPer);
				chmod(path_string,newPer);
			}
			printf("old: %o\n new: %o\n",oldPer,newPer);
			if(newPer==oldPer)
				return 1;
	return 0;
}
int search_dir_recursive(char *path, struct info* inf)
{
	
	dir=path;
	printf("PATH: %s\n",dir);
	DIR *directory = opendir(path);
	struct dirent *file_name;

	while ((file_name = readdir(directory)) != NULL)
	{
		struct stat path_stat;
		char *path_string = malloc(sizeof(path) + sizeof('/') + sizeof(file_name->d_name));
		
		sprintf(path_string, "%s/%s", path, file_name->d_name);
		stat(path_string, &path_stat);
		if (S_ISREG(path_stat.st_mode))//faz de conta que está bem
		{	
			changePermission(path_stat,inf,path_string);
			//mudar permissão de ficheiro aqui
		}
		else if (S_ISDIR(path_stat.st_mode) && strcmp(file_name->d_name, "..") && strcmp(file_name->d_name, "."))
		{
			changePermission(path_stat,inf,path_string);
			//mudar permissão de diretório aqui
			
			int id = fork();
			if (id == 0)
			{      sleep(6);
				search_dir_recursive(path_string, inf);
				return 0;
			}
			else
			{
				wait(NULL);
				
					
			}
		}
		free(path_string);
	}
	
	//escrever num ficheiro à parte informações sobre este processo-filho que está prestes a terminar
	
	return 0;
}


int search_dir(char *path, int showAll, struct info* inf)
{
		struct stat path_stat;
		stat(path, &path_stat);
		if (S_ISREG(path_stat.st_mode))
		{
			if(!changePermission(path_stat,inf,path) && !showAll)
				printf("Modo C!\n");
			else if( showAll)
				printf("Modo V!\n");
				//mostrar ficheiros
		}
		else if (S_ISDIR(path_stat.st_mode))
		{
			if(!changePermission(path_stat,inf,path) && !showAll)
				printf("Modo C!\n");
			else if( showAll)
				printf("Modo V!\n");
				//mostrar ficheiros
			
		}
	//escrever num ficheiro à parte informações sobre este processo-filho que está prestes a terminar
	
	return 0;
}


int main(int argc, char *argv[]){


parent=getpid();
struct info inputInfo;
processInput(&inputInfo, argv[1], argv[2],argv[3]);

struct sigaction new, old;
sigset_t smask;
if(sigemptyset(&smask)==-1)
 perror("sigsetfunctions");
new.sa_handler=func;
new.sa_mask=smask;
new.sa_flags=0;
if(sigaction(SIGINT, &new, &old)==-1)
perror("sigaction");

if(inputInfo.optionV){
search_dir(argv[3],1,&inputInfo);
}else if(inputInfo.optionC){
search_dir(argv[3],0,&inputInfo);
}else if(inputInfo.optionR){
search_dir_recursive(argv[3],&inputInfo);
}


//matar todos os processos exceto um
if(getpid()!=parent)
	exit(0);

return 0;
}
