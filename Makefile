CC=gcc
CFLAGS=-O2
NAME=cplayer

ODIR=src

LIBS=-lcdk -lncursesw -lmenuw -lpthread -lavformat -lavcodec -lswscale -lavutil -lswresample -lz -ldl `sdl2-config --cflags --libs`


_OBJ = main.o\
	db.o\
	file_manager.o\
	player_core.o\
	popen2.o\
	ui.o\
	utils.o\
	sqlite3/sqlite3.o\
	core/player.o

OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(NAME): $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o $(NAME)

install:
	cp $(NAME) /usr/local/bin/

uninstall:
	rm -r /usr/local/bin/$(NAME)
