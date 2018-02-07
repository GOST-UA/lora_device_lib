MegaDemo
========

MegaDemo integrates LDL onto an ATMega328p. At the moment this doesn't do
anything other than give an indication of memory useage.

~~~
avr-size --format=avr --mcu=atmega328p mega_demo.elf
AVR Memory Usage
----------------
Device: atmega328p

Program:   20136 bytes (61.5% Full)
(.text + .data + .bootloader)

Data:        526 bytes (25.7% Full)
(.data + .bss + .noinit)

EEPROM:      159 bytes (15.5% Full)
(.eeprom)
~~~

## License

MIT (part of the LoraDeviceLib project)





