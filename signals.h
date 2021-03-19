#ifndef SIGNALS_H_
#define SINGNALS_H_ 

int setParent();
int setChild();

void childinthandler(int signo);
void parentinthandler(int signo);

void childcontinue(int signo);
void childend(int signo);

void parentterminate();
void childterminal();

int checkcurrentstat();


#endif