PROJECT=cantaveria
CFLAGS=-g -O2 -Wall -Wextra -Wno-unused-parameter
SRC=video.c audio.c input.c kernel.c main.c gameover.c \
    loader.c graphics.c sfx.c text.c console.c music.c stage.c \
    intro.c title.c splash.c inner.c \
    synth.c seq.c midi.c orc.c dsp.c \
    rng.c util.c list.c zip.c
OBJ:=$(SRC:.c=.o)
CC=gcc
LIBS=-lSDL -lGL -lm -lz


$(PROJECT): $(OBJ) data.zip
	$(CC) -o $(PROJECT) $(LIBS) $(OBJ)

$(OBJ): %o: %c
	$(CC) -c -I. $(CFLAGS) -o $@ $<

data.zip:
	wget http://evanr.infinitymotel.net/cantaveria/data.zip

clean:
	$(RM) $(PROJECT) *{.o,.a}

tarball:
	mkdir -p dist/cantaveria
	cp --parents *{.c,.h} Makefile AUTHORS COPYING data.zip dist/cantaveria/
	cd dist && tar cvzf ../cantaveria.tar.gz cantaveria
	rm -rf dist

depend:
	gcc -MM -I. $(SRC) > depend


include depend
