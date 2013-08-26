#ifndef FLASH_H
#define FLASH_H

#define FLASH_MAIN_BASE ((unsigned char *)0x1000)
#define FLASH_MAIN_SIZE 126

char flash_read(char *, unsigned char);
void flash_write(const char *, unsigned char);

#endif
