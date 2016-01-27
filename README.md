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

- **demo: **
    `gcc -g -O2  demo.c file_manager.c utils.c -o cplayer -lcdk -lncurses`
- **db_demo: **
    `gcc -g -O2 db_test.c db.c sqlite3/sqlite3.c -o db_test -lpthread -ldl`
