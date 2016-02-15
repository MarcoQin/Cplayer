# Cplayer
A simple music player written in C.

####Requirments:

- **mplayer:**
    - ubuntu: `sudo apt-get install mplayer`
    - others: [official site](http://www.mplayerhq.hu/design7/dload.html)

####Lib Dependencies:

- **ncurses:** [click here to find out how to install](http://tldp.org/HOWTO/NCURSES-Programming-HOWTO/intro.html#WHERETOGETIT)
- **CDK(Curses Development Kit):** [download and install](http://invisible-island.net/cdk/#download)

####Compile:

`gcc main.c utils.c ui.c player_core.c popen2.c db.c sqlite3/sqlite3.c file_manager.c -o cplayer -lcdk -lncursesw -lmenuw -lpthread -ldl`

####Screenshot:

![Cplayer-songlist][songlist]

[songlist]: https://github.com/MarcoQin/gallery/blob/master/Cplayer/songlist.png
