CFLAGS= -D__LINUX__ -c -Wall -O2
LIBS = -lm -lpthread

all: ls_i2c

ls_i2c: main.o
	gcc main.o $(LIBS) -o ls_i2c

main.o: main.c
	gcc $(CFLAGS) main.c

clean:
	rm *.o ls_i2c
