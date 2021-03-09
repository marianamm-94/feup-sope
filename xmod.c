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

int search_dir_recursive(char *path)
{
	DIR *directory = opendir(path);
	struct dirent *file_name;

	while ((file_name = readdir(directory)) != NULL)
	{
		struct stat path_stat;
		char *path_string = malloc(sizeof(path) + sizeof('/') + sizeof(file_name->d_name));
		sprintf(path_string, "%s/%s", path, file_name->d_name);
		stat(path_string, &path_stat);
		if (S_ISREG(path_stat.st_mode))
		{
			//mudar permissão de ficheiro aqui
		}
		else if (S_ISDIR(path_stat.st_mode) && strcmp(file_name->d_name, "..") && strcmp(file_name->d_name, "."))
		{
		
			//mudar permissão de diretório aqui
			
			int id = fork(), wpid;
			if (id == 0)
			{
				search_dir_recursive(path_string);
				return 0;
			}
			else
			{
				wpid=wait(NULL);
					
			}
		}
		free(path_string);
	}
	
	//escrever num ficheiro à parte informações sobre este processo-filho que está prestes a terminar
	
	return 0;
}

int main(int argc, char *argv[]){

search_dir_recursive(argv[1]); // o diretório não é aqui no input mas é para testar a opção -R



return 0;
}
