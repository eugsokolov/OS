test.out: schedtest.o sched.o adjstack.o savectx.o
	gcc -o test.out schedtest.o sched.o adjstack.o savectx.o 

schedtest.o: schedtest.c
	gcc -c schedtest.c

sched.o: sched.c sched.h
	gcc -c sched.c

adjstack.o: adjstack.c
	gcc -c adjstack.c

savectx.o: savectx.S
	gcc -c savectx.S

clean:
	rm *.out *.o
