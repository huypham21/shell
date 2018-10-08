# makefile

all: memtest

shell.o: shell.cpp 
	g++ -c -g shell.cpp

main.o : Main.cpp
	g++ -c -g main.cpp

memtest: main.o shell.o
	g++ -o memtest main.o shell.o

clean:
	rm *.o
