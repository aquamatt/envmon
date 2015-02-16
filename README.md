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

Usage
-----

To build the sensors firmware, including the i2c dependency::

  $ make all

To clean out build artifacts::

  $ make clean

To deep clean, including artifacts in the sub-projects::

  $ make reallyclean

To deploy to the Atmel AVR chip::

  $ make deploy

``NB: ALL PARAMETERS IN THE MAKEFILE AND CODE ASSUME AN ATMEGA8A uC CHIP``
