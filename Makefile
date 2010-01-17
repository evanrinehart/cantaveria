PROJECT = cantaveria
CFLAGS = -g -O2 -Wall -Wextra -Wno-unused-parameter
OBJ = util.o backend.o loader.o game.o title.o intro.o sound.o graphics.o text.o splash.o synth.o dsp.o rng.o
CC = gcc
LIBS = -lSDL -lGL -lzzip -lm

$(PROJECT): main.o $(OBJ) data.zip
	$(CC) -o $(PROJECT) $(LIBS) main.o $(OBJ)

#levedit: editor.o $(OBJ)
#	$(CC) -o levedit $(LIBS) editor.o $(OBJ)


data.zip:
	wget http://evanr.infinitymotel.net/cantaveria/data.zip

game.o: util.h loader.h game.h intro.h

splash.o : backend.h intro.h splash.h graphics.h
title.o: util.h game.h backend.h title.h
intro.o: util.h game.h backend.h graphics.h intro.h title.h

util.o: util.h rng.h
main.o: util.h game.h backend.h intro.h loader.h graphics.h text.h
#editor.o: util.h game.h backend.h loader.h graphics.h text.h
backend.o: game.h backend.h util.h loader.h sound.h
loader.o: loader.h util.h

sound.o: sound.h
synth.o: synth.h backend.h util.h
dsp.o: util.h backend.h dsp.h

graphics.o: graphics.h backend.h loader.h util.h game.h
text.o : text.h backend.h loader.h util.h


backend.h: util.h

clean:
	$(RM) $(PROJECT) levedit main.o editor.o $(OBJ)
