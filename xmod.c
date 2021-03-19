#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include "logging.h"
#include "signals.h"

extern volatile sig_atomic_t hold;
extern FILE *file;


int openfile(char* p);
int closefile();
clock_t clock();

int nftot, nfmod;
char *arg0;
char *arg1;
char *arg2;
char *dir;

void printInformation(int oldOctal, int newOctal, char *dir, int retained) {
    char old[12] = "(---------)";
    char new[12] = "(---------)";
    if (oldOctal & 0x100)
        old[1] = 'r';
    if (oldOctal & 0x80)
        old[2] = 'w';
    if (oldOctal & 0x40)
        old[3] = 'x';
    if (oldOctal & 0x20)
        old[4] = 'r';
    if (oldOctal & 0x10)
        old[5] = 'w';
    if (oldOctal & 0x8)
        old[6] = 'x';
    if (oldOctal & 0x4)
        old[7] = 'r';
    if (oldOctal & 0x2)
        old[8] = 'w';
    if (oldOctal & 0x1)
        old[9] = 'x';

    if (newOctal & 0x100)
        new[1] = 'r';
    if (newOctal & 0x80)
        new[2] = 'w';
    if (newOctal & 0x40)
        new[3] = 'x';
    if (newOctal & 0x20)
        new[4] = 'r';
    if (newOctal & 0x10)
        new[5] = 'w';
    if (newOctal & 0x8)
        new[6] = 'x';
    if (newOctal & 0x4)
        new[7] = 'r';
    if (newOctal & 0x2)
        new[8] = 'w';
    if (newOctal & 0x1)
        new[9] = 'x';
    if (retained)
        printf("mode of '%s' retained as %o %s\n", dir, oldOctal, old);
    else
        printf("mode of '%s' changed from %o %s to %o %s\n", dir, oldOctal, old, newOctal, new);
}


struct info {
    int octal;
    int optionV;
    int optionC;
    int optionR;
    char *fileOrDir;
    int add;
    int sub;
    int replace;
    int number;
};


void processInput(struct info *inf, char *option, char *mode, char *name) {
    inf->optionV = 0;
    inf->optionC = 0;
    inf->optionR = 0;
    inf->add = 0;
    inf->sub = 0;
    inf->replace = 0;
    inf->number = 0;

    if (strcmp(option, "-V") == 0 || strcmp(option, "-v") == 0)
        inf->optionV = 1;
    if (strcmp(option, "-C") == 0 || strcmp(option, "-c") == 0)
        inf->optionC = 1;
    if (strcmp(option, "-R") == 0 || strcmp(option, "-r") == 0)
        inf->optionR = 1;

    if (mode[0] != 'u' && mode[0] != 'g' && mode[0] != 'o' && mode[0] != 'a') {
        inf->octal = strtol(mode, 0, 8);
        inf->replace = 1;
    } else {
        int i = 0;
        int r = 0;
        int w = 0;
        int x = 0;
        int n = 0;
        while (mode[i] != '\0') {
            if (mode[i] == 'r')
                r = 1;
            if (mode[i] == 'w')
                w = 1;
            if (mode[i] == 'x')
                x = 1;
            i++;
        }
        if (mode[0] == 'u' || mode[0] == 'a')
            n += 64 * (4 * r + 2 * w + x);
        if (mode[0] == 'g' || mode[0] == 'a')
            n += 8 * (4 * r + 2 * w + x);
        if (mode[0] == 'o' || mode[0] == 'a')
            n += 4 * r + 2 * w + x;
        inf->octal = n;
        if (mode[1] == '=')
            inf->replace = 1;
        else if (mode[1] == '+') {
            inf->add = 1;
            inf->number = inf->octal;
        } else if (mode[1] == '-') {
            inf->sub = 1;
            inf->number = 511 - inf->octal;
        }

    }

}

int process_permission(struct stat st) {
    int perm = 0;

    if (st.st_mode & S_IRUSR) perm += 4 * 64; //r
    if (st.st_mode & S_IWUSR) perm += 2 * 64; //w
    if (st.st_mode & S_IXUSR) perm += 1 * 64; //x
    if (st.st_mode & S_IRGRP) perm += 4 * 8;  //r
    if (st.st_mode & S_IWGRP) perm += 2 * 8;  //w
    if (st.st_mode & S_IXGRP) perm += 1 * 8;  //x
    if (st.st_mode & S_IROTH) perm += 4;   //r
    if (st.st_mode & S_IWOTH) perm += 2;   //w
    if (st.st_mode & S_IXOTH) perm += 1;   //x

    return perm;
}

int changePermission(struct stat path_stat, struct info *inf, char *path_string, int *old, int *new) {
    int oldPer = process_permission(path_stat), newPer;
    if (inf->replace) {
        newPer = inf->octal;
        chmod(path_string, newPer);
    } else if (inf->add) {
        newPer = oldPer | inf->number;
        newPer = newPer;
        chmod(path_string, newPer);
    } else if (inf->sub) {
        newPer = oldPer & inf->number;
        newPer = newPer;
        chmod(path_string, newPer);
    }
    *old = oldPer;
    *new = newPer;
    if (newPer == oldPer)
        return 1;
    return 0;
}

int search_dir_recursive(char *path, struct info *inf) {
    int old, new;
    if (getpgid(0) == getpid()) 
        setParent();
    else
    	setChild();
    DIR *directory = opendir(path);
    struct dirent *file_name;

    while ((file_name = readdir(directory)) != NULL) {
        if (checkcurrentstat())
            fprintf(stderr,"ERROR");
        if (hold)
            pause();
        struct stat path_stat;
        char *path_string = malloc(sizeof(path) + sizeof('/') + sizeof(file_name->d_name));
	if (checkcurrentstat())
            fprintf(stderr,"ERROR");
        sprintf(path_string, "%s/%s", path, file_name->d_name);
        stat(path_string, &path_stat);
        if (S_ISREG(path_stat.st_mode)) {
            nftot++;
            if (!changePermission(path_stat, inf, path_string, &old, &new)){
                nfmod++;
                loggingFile("FILE_MODF", path_string, old, new);
                }
        } else if (S_ISDIR(path_stat.st_mode) && strcmp(file_name->d_name, "..") && strcmp(file_name->d_name, ".")) {
            if(!changePermission(path_stat, inf, path_string, &old, &new))
		loggingFile("FILE_MODF", path_string, old, new);
            int id = fork();
            if (id == 0) {
                char *ar[] = {arg0, arg1, arg2, path_string,NULL};
                execvp(ar[0], ar);
                return 0;
            } else {
              
            }
        }
        free(path_string);
    }

    closedir(directory);
    while (waitpid(-1,NULL,WNOHANG)>=0)
    {
        checkcurrentstat();
    }
    return 0;
}


int search_dir(char *path, int showAll, struct info *inf) {
    int old, new;
    struct stat path_stat;
    stat(path, &path_stat);
    if (S_ISREG(path_stat.st_mode)) {
        if (changePermission(path_stat, inf, path, &old, &new)) {
            if (showAll)
                printInformation(old, new, path, 1);
        } else {
            loggingFile("FILE_MODF", path, old, new);
            printInformation(old, new, path, 0);
            nfmod++;
        }


        nftot++;

    } else if (S_ISDIR(path_stat.st_mode)) {
        if (!changePermission(path_stat, inf, path, &old, &new)){   loggingFile("FILE_MODF", path, old, new);	
            printInformation(old, new, path, 0);
            }
        else if (showAll)
            printInformation(old, new, path, 1);


    }
    

    return 0;
}


int main(int argc, char *argv[],char *envp[]) {
    char * argInfo = malloc(strlen(argv[0])+strlen(argv[1])+strlen(argv[2])+strlen(argv[3])+strlen("       ")+1);
    strcpy(argInfo, argv[0]);
    strcat(argInfo, " ");
    strcat(argInfo, argv[1]);
    strcat(argInfo, " ");
    strcat(argInfo, argv[2]);
    strcat(argInfo, " ");
    strcat(argInfo, argv[3]);
    if (getpgid(0) == getpid())
    	{   
    	    char* beg = malloc(50), *beg1=malloc(50);
    	    struct timeval start;
    	    gettimeofday(&start,NULL);
    	    long secStart = start.tv_sec;
    	    long microStart = start.tv_usec;
    	    sprintf(beg,"%ld",secStart);
    	    sprintf(beg1,"%ld",microStart);
    	    setenv("INITIAL1",beg,1);
    	    setenv("INITIAL2",beg1,1);
    	    if(!openfile("w"))
    	    {
    	    fprintf(file, "0 ; %d ; PROC_CREAT ; %s %s %s %s \n" ,getpid(),argv[0],argv[1],argv[2],argv[3]);
            closefile();
            free(beg);
            free(beg1);
    	    }
    	}
    else
        logging("PROC_CREAT",argInfo);
    nfmod = 0;
    nftot = 0;
    arg0 = argv[0];
    arg1 = argv[1];
    arg2 = argv[2];
    dir = argv[3];
    struct info inputInfo;
    processInput(&inputInfo, argv[1], argv[2], argv[3]);
    if (inputInfo.optionV) {
        search_dir(argv[3], 1, &inputInfo);
    } else if (inputInfo.optionC) {
        search_dir(argv[3], 0, &inputInfo);
    } else if (inputInfo.optionR) {
        search_dir_recursive(argv[3], &inputInfo);
    }
    logging("PROC_EXIT", "0");
    return 0;
}
