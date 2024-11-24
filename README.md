# TG19264ALibrary
Library to interface with TG19264A Display using ATmega32A.
This is hobby project to create library for interfacing with display given in title.
Datasheet to used display according to which interface is implemented: https://www.manualslib.com/products/Vatronix-Tg19264a-02wa0-8745270.html

Future plans:
  - finish proposed functions (done)
  - refactor code to be better to read and understand (in progress)
  - use of preprocessor for choosing between using simple AVR(tiny/mega) I/O interface or function declared by user. (e.g. outputFun(uint8_t Byte) for sending 1 Byte to display)
    (This future would made library more general, not olny for AVR or specificly Atmega32A)
  - adding new functions if needed (reverse screen for given area [after refactor] )

How to use:

  Add include\*.h and src\*.c files to include path.
  Change macros function and displayInit(), waitBusy(), readByte() function (DDRXs & PORTXs).
  Use functions given in include\TG19264A_atmegaDriver.h only.
