CFLAGS+=-Wall

all:main
main:main.o draw.o
	$(CC)  $^ -o $@ $(CFLAGS) 

clean:
	rm *.o main -rf
