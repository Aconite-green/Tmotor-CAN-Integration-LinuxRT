all: main path_manager

main:
	gcc -o main main.c src/*.c -Iinclude -lm

path_manager:
	gcc -o path_manager path_manager.c src/motor.c src/can_utils.c src/error_handling.c -Iinclude -lm

clean:
	rm -f main path_manager main.o src/*.o
