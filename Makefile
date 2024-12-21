all: main.c
	@mkdir -p bin
	gcc main.c -o bin/prog 