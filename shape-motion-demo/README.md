## Contents

This project containts the following files:

bbmain.c: Main program

buzzer.c: Initialize buzzer and set period

buzzer.h Header file of buzzer.c

led.c: Initialize LED's and some implementation of how LED's react to certain events

led.h: Header file of led.c

wdInterruptHandler.c: Interrupt handler and state machines for msp430 behaviour.

## Description
The Music Box 2.0 comes with 4 buttons and a green and red LED, the Music Box 2.0 comes with two games:

1. Music player
2. Piano

The Music Box 2.0 start in the Music Player mode, indicated by the red light on. To start playing songs just press either button 1,2 or 3. the songs including in this version are:

1. Epona's Song - The Legend of Zelda
2. Zelda's Lullaby - The Legend of Zelda
3. El Sonidito - Hechizeros Band

To go to piano mode you would just need to press the button number 4 and the light would change to color green indicating that Piano mode is on. The flashing speed of the green light is indicated by the note in the piano.

To load code into msp430, try:
~~~
$ make load
~~~
To clean repository, try:
~~~
$ make clean
~~~

Credits:

-Jose Andres Cabrera - Understanding notes

https://answers.yahoo.com/question/index?qid=20101201100813AAwDHyl - To know the notes in The Legend of Zelda songs
