# Cplayer
A simple music player write in C

####Requirments:

- **ncurses:** [click here to find out how to install](http://tldp.org/HOWTO/NCURSES-Programming-HOWTO/intro.html#WHERETOGETIT)
- **CDK(Curses Development Kit):** [download and install](http://invisible-island.net/cdk/#download)

compile:
gcc -g -O2  demo.c file_manager.c utils.c -o cplayer -lcdk -lncurses
