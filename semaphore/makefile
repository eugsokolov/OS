a.out: main.o fifo.o sem.o tas.o
	gcc -o a.out main.o fifo.o sem.o tas.o

main.o: main.c
	gcc -c main.c

fifo.o: fifo.c fifo.h
	gcc -c fifo.c

sem.o: sem.c sem.h
	gcc -c sem.c

tas.o: tas.S
	gcc -c tas.S

clean:
	rm *.out *.exe *.o
