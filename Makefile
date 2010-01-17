PROJECT = cantaveria
CFLAGS = -I. -g -O2 -Wall -Wextra -Wno-unused-parameter
OBJ = util.o backend.o loader.o game.o title.o intro.o \
      sound.o graphics.o text.o splash.o synth.o dsp.o \
      rng.o input.o
CC = gcc
LIBS = -lSDL -lGL -lzzip -lm

$(PROJECT): main.o $(OBJ) data.zip
	$(CC) -o $(PROJECT) $(LIBS) main.o $(OBJ)


data.zip:
	wget http://evanr.infinitymotel.net/cantaveria/data.zip

clean:
	$(RM) $(PROJECT) levedit main.o $(OBJ)

