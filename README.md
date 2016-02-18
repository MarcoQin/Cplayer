# Cplayer
A simple music player written in C.

####Requirments:

- **mplayer:**
    - ubuntu: `sudo apt-get install mplayer`
    - others: [official site](http://www.mplayerhq.hu/design7/dload.html)

####Lib Dependencies:

- **ncurses:** [click here to find out how to install](http://tldp.org/HOWTO/NCURSES-Programming-HOWTO/intro.html#WHERETOGETIT)
- **CDK(Curses Development Kit):** [download and install](http://invisible-island.net/cdk/#download)

####Make And Install:

`make`

`sudo make install`

####Uninstall:

`sudo make uninstall`

####Format Support:

All music files. Including but not limited to: mp3, flac, ape, wav, ogg...

####Usage:

- **j/k** or **↓/↑ :**  move the cursor down and up
- **h/l** or **←/→ :**  seek left or right about 20 seconds
- **a**: add songs to list
- **d**: delete current selected song from song list
- **Enter**: play current selected song
- **Space**: pause/unpause the current playing song
- **s**: stop playing
- **n**: next song
- **p**: previous song
- **q**: quit

####Screenshot:

![Cplayer-songlist][songlist]


![Cplayer-screenshot][shot]

[songlist]: https://github.com/MarcoQin/gallery/blob/master/Cplayer/songlist.png
[shot]: https://github.com/MarcoQin/gallery/blob/master/Cplayer/screenshot.png
