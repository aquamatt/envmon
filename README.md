Adventures in AVR land
======================

Experiments in using the Atmel AVR microcontroler and peripherals such as the
BMP180 temperature/pressure sensor.

This is all aimed at building a wireless networked environmental monitoring system for
no good reason other than 'because'.

Includes Peter Fleury's I2C library from http://bit.ly/1zL1pMo copied into the tree for convenience.
Find this in the ``i2c`` sub-directory - all code in that directory is (c) Peter Fleury.

Peter Fleury also has a UART library which would be better than using the basic UART ops in
this code probably. Will take a look when we get to sending real data out over radio links.
