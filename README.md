# Cplayer
A simple music player written in C, with ffmpeg surpport.

####Lib Dependencies:

- **ncurses:** [click here to find out how to install](http://tldp.org/HOWTO/NCURSES-Programming-HOWTO/intro.html#WHERETOGETIT)
- **CDK(Curses Development Kit):** [download and install](http://invisible-island.net/cdk/#download)
- **SDL2.0**
- **ffmpeg:**
    - make and install the latest ffmpeg:
        - (maby need: sudo apt-get install yasm)
        - ./configure --enable-shared
        - make
        - sudo make install
        - if can't find lib*.so:

            sudo vi /etc/ld.so.conf
            add follow lines:
                include ld.so.conf.d/*.conf
                /usr/local/libevent-1.4.14b/lib
                /usr/local/lib

            then:
                sudo ldconfig


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

####Screenshots:

![Cplayer-songlist][songlist]


![Cplayer-screenshot][shot]

[songlist]: https://github.com/MarcoQin/gallery/blob/master/Cplayer/songlist.png
[shot]: https://github.com/MarcoQin/gallery/blob/master/Cplayer/screenshot.png
