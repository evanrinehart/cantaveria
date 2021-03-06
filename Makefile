PROJECT=cantaveria
CFLAGS=-g -O2 -Wall -Wextra -Wno-unused-parameter -W -Wundef \
       -Wshadow -Wbad-function-cast -Wcast-align -Wwrite-strings \
       -Wnested-externs -Werror -Wno-unused-function -Wno-unused-variable \
       -Wno-cast-align -Wno-unused-parameter
SRC=video.c audio.c input.c kernel.c main.c gameover.c \
    loader.c graphics.c sfx.c text.c console.c music.c stage.c \
    intro.c title.c splash.c  \
    synth.c seq.c midi.c orc.c dsp.c \
    rng.c util.c list.c zip.c \
    hud.c camera.c entity.c ent0.c soundtest.c
OBJ:=$(SRC:.c=.o)
CC=gcc
LIBS=-lSDL -lSDLmain -lm -lz -Wl,-framework,Cocoa -framework OpenGL


$(PROJECT): $(OBJ) data.zip
	$(CC) -o $(PROJECT) $(LIBS) $(OBJ)
editor: $(OBJ) edit.c
	$(CC) -o editor -I. $(LIBS) console.o loader.o list.o \
	video.o graphics.o util.o rng.o zip.o camera.o edit.c


$(OBJ): %o: %c
	$(CC) -c -I. $(CFLAGS) -o $@ $<

data.zip:
	wget http://evanr.infinitymotel.net/cantaveria/data.zip

clean:
	$(RM) $(PROJECT) *{.o,.a} depend editor

tarball:
	mkdir -p dist/cantaveria
	cp --parents *{.c,.h} Makefile AUTHORS COPYING data.zip dist/cantaveria/
	cd dist && tar cvzf ../cantaveria.tar.gz cantaveria
	rm -rf dist

depend:
	gcc -MM -I. $(SRC) > depend


include depend
