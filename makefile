xmod: xmod.o
	gcc -Wall xmod.c -o xmod -lm
	

clean:
	rm -f xmod *.o
