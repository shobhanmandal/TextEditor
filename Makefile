
output: DriverProgram.o
	gcc DriverProgram.o -o DriverProgram
	
DriverProgram.o: DriverProgram.c
	gcc -c DriverProgram.c


target: dependencies
	action

clean:
	rm *.o ouput
