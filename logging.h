#ifndef LOGGING_H_
#define LOGGING_H_ 

int open(char *p);

int close();

int logging(char* action, char* info);
int loggingFile(char* action, char* path, int o, int n);


#endif
