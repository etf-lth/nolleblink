#ifndef FLASH_H
#define FLASH_H

#define FLASH_MAIN_BASE ((unsigned char *)0x1040)
#define FLASH_MAIN_SIZE 62

#define FLASH_RT_BASE ((unsigned short *)0x1080)

char flashRead(char *, char);
void flashWrite(const char *, char);
int flashGetTime(void);
void flashUpdateTime(void);

#endif
