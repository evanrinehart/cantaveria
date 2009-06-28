PROJECT = cantaveria
CFLAGS = -std=c99 -Wall -g -O2
OBJ = main.o util.o backend.o loader.o game.o title.o intro.o sound.o
CC = gcc
LIBS = -lSDL -lGL -lzzip

$(PROJECT): $(OBJ)
	$(CC) -o $(PROJECT) $(LIBS) $(OBJ)



game.o: game.h intro.h
title.o: game.h backend.h title.h
intro.o: game.h backend.h intro.h title.h
util.o: util.h
main.o: game.h backend.h intro.h loader.h util.h
backend.o: game.h backend.h util.h loader.h sound.h
loader.o: loader.h util.h
sound.o: sound.h


clean:
	rm $(PROJECT) $(OBJ)
