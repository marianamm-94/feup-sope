xmod: xmod.c logging.c signals.c
	gcc -Wall xmod.c logging.c signals.c -o xmod 
		

clean:
	rm -f xmod *.o
