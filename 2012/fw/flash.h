#ifndef FLASH_H
#define FLASH_H

#define FLASH_MAIN_BASE ((unsigned char *)0x1000)
#define FLASH_MAIN_SIZE 82

//#define FLASH_RT_BASE ((unsigned short *)0x1080)

char flashRead(char *, char);
void flashWrite(const char *, char);
int flashGetTime(void);
void flashUpdateTime(void);

#endif
