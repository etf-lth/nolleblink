#ifndef PROTOCOL_H
#define PROTOCOL_H

typedef struct
{
  unsigned char reserved;
  unsigned char to[2];
  unsigned char from[2];
  unsigned char seq[2];
  unsigned char type, size;
  unsigned char payload[50];
} packet_t;

enum
  {
    CMD_RESERVED_1,
    CMD_PING,
    CMD_TEMP_MESSAGE,
    CMD_SHOW_ID,
    CMD_SYNC,
    CMD_PONG,
    CMD_RESERVED_2,
    CMD_WRITE_TEXT,
    CMD_WRITE_TEXT_ID_OLD,
    CMD_VIRUS,
    CMD_WRITE_ID,
    CMD_FRAMEBUFFER,
    CMD_HEARTBEAT,
    CMD_READ_TEXT,
  };

#endif
