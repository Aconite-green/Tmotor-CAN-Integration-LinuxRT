
# Declare variables
CC = gcc
CFLAGS = -Wall -O2
INCLUDE = -I./include
SRCDIR = ./src
BINDIR = ./
OBJFILES = main.o $(SRCDIR)/can_utils.o $(SRCDIR)/error_handling.o $(SRCDIR)/motor.o $(SRCDIR)/realtime_config.o $(SRCDIR)/socket_config.o

# Build target
all: main

main: $(OBJFILES)
	$(CC) $(CFLAGS) $(OBJFILES) -o $(BINDIR)/main $(INCLUDE) -lm


main.o: main.c
	$(CC) $(CFLAGS) -c main.c $(INCLUDE)

$(SRCDIR)/can_utils.o: $(SRCDIR)/can_utils.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/can_utils.c -o $(SRCDIR)/can_utils.o $(INCLUDE)

$(SRCDIR)/error_handling.o: $(SRCDIR)/error_handling.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/error_handling.c -o $(SRCDIR)/error_handling.o $(INCLUDE)

$(SRCDIR)/motor.o: $(SRCDIR)/motor.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/motor.c -o $(SRCDIR)/motor.o $(INCLUDE)

$(SRCDIR)/realtime_config.o: $(SRCDIR)/realtime_config.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/realtime_config.c -o $(SRCDIR)/realtime_config.o $(INCLUDE)

$(SRCDIR)/socket_config.o: $(SRCDIR)/socket_config.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/socket_config.c -o $(SRCDIR)/socket_config.o $(INCLUDE)

clean:
	rm -f $(OBJFILES) $(BINDIR)/main
