clear;
avr-gcc -std=gnu99 -g -mmcu=attiny4313 -Wall -Os -mcall-prologues -Wno-deprecated-declarations -Wno-main -c src/main.c -o led1.o 

avr-gcc -Wl,-Map,led1.map   -Os -mmcu=attiny4313 -o led1.elf led1.o

avr-objdump -h -S led1.elf > led1.lst

avr-objcopy -j .eeprom --change-section-lma .eeprom=0 -O binary led1.elf eeprom.bin

avr-objcopy -j .text -j .data -O ihex led1.elf led1.hex

avr-objcopy -j .eeprom --change-section-lma .eeprom=0 -O ihex led1.elf led1_eeprom.hex

avr-size led1.elf
