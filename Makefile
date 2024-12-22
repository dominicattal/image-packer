export TEMP := .
export TMP := .

all: main.c
	@mkdir -p bin
	gcc main.c -o bin/prog -Wall -Wextra 