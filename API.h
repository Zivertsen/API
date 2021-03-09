#ifndef HEADER_API
#define HEADER_API
#include <Arduino.h>
#include "FrameStructure.h"

/* SLIP special character codes*/
#define END             0xC0    /* indicates end of packet */
#define ESC             0xDB    /* indicates byte stuffing */
#define ESC_END         0xDC    /* ESC ESC_END means END data byte */
#define ESC_ESC         0xDD

boolean getFrame(int *pFrame);

boolean sendFrame(uint8_t *pFrame);

void buildandSendFrame(uint8_t command, uint8_t ch, uint16_t data );

#endif
