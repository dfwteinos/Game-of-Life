main: simple_main.o simple_GOL.o
	gcc simple_main.o simple_GOL.o -o main

simple_main.o: simple_main.c
	gcc -c simple_main.c

simple_GOL.o: simple_GOL.c simple_GOL.h
	gcc -c simple_GOL.c

clean:
	rm *.o main

