CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS = -lws2_32


main:
	$(CC) $(CFLAGS) -o main.exe main.c  $(LDFLAGS)

clean:
	rm -f main.exe