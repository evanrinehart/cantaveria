PROJECT = cantaveria
CFLAGS = -I. -g -O2 -Wall -Wextra -Wno-unused-parameter
OBJ = video.o audio.o input.o kernel.o \
      loader.o graphics.o sfx.o text.o console.o \
      intro.o title.o splash.o game.o \
      synth.o seq.o dsp.o \
      rng.o util.o

CC = gcc
LIBS = -lSDL -lGL -lzzip -lm

$(PROJECT): main.o $(OBJ) data.zip
	$(CC) -o $(PROJECT) $(LIBS) main.o $(OBJ)


data.zip:
	wget http://evanr.infinitymotel.net/cantaveria/data.zip

clean:
	$(RM) $(PROJECT) levedit main.o $(OBJ)

