## Contents

This project containts the following files:

shapemotion.c: Main program

buzzer.c: Initialize buzzer and set period

buzzer.h Header file of buzzer.c

score.s: Assembly part of code for increasing score

wdt_handler.s

## Description
MSP430 PONG! is a symple game which let's you play one of the electronic games in the market right on your msp430, there are two players.

1.Red
2.Blue

The first two buttons of each side move the correspondent paddle up or down. The first player to reach a score of 5 wins and to start playing again you just hit the reset button on the msp430 board.

To load code into msp430, try:
~~~
$ make load
~~~
To clean repository, try:
~~~
$ make clean
~~~
