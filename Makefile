PROJECT = cantaveria
CFLAGS = -std=c99 -Wall -g -O2
OBJ = util.o backend.o loader.o game.o title.o intro.o sound.o graphics.o text.o
CC = gcc
LIBS = -lSDL -lGL -lzzip

$(PROJECT): main.o $(OBJ) levedit
	$(CC) -o $(PROJECT) $(LIBS) main.o $(OBJ)

levedit: editor.o $(OBJ)
	$(CC) -o levedit $(LIBS) editor.o $(OBJ)

game.o: util.h loader.h game.h intro.h
title.o: util.h game.h backend.h title.h
intro.o: util.h game.h backend.h graphics.h intro.h title.h
util.o: util.h
main.o: util.h game.h backend.h intro.h loader.h graphics.h text.h
editor.o: util.h game.h backend.h loader.h graphics.h text.h
backend.o: game.h backend.h util.h loader.h sound.h
loader.o: loader.h util.h
sound.o: sound.h
graphics.o: graphics.h backend.h loader.h util.h game.h
text.o : text.h backend.h loader.h util.h

backend.h: util.h

clean:
	rm $(PROJECT) main.o editor.o $(OBJ)
