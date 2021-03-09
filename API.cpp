#include "API.h"


//--------------------- Function declaration -------------
boolean readSerial(int *readS);

void writeSerial(uint8_t *writeS, int len);

//Removes the END and ESC characters from the packet
int unslipPacket(int *packet, int *frame);

//Make a frame ready for being sent, by adding END and ESC charaters
int slipPacket(uint8_t *frame, uint8_t *packet);

// checks the checksum and remove it 
boolean confirmCheckSum(int *frame);

// calculate the checksum by XOR the values and adds it to the frame
void addCheckSum(uint8_t *frame);


// ------------------ Function bodies ------------------------
uint16_t builtinBitswap(uint16_t data)
{
    return (data >> 8) | (data << 8);
}

void buildandSendFrame(uint8_t command, uint8_t ch, uint16_t data )
{

    uint8_t pFrame[10];
    switch (command)
    {
    case 0x01:
        ST_GW_VOLTAGE tempVoltageStructure;
        tempVoltageStructure.Voltage = builtinBitswap(data);
        tempVoltageStructure.FrameHeader.Command = command;
        tempVoltageStructure.FrameHeader.Channel = ch;
        tempVoltageStructure.FrameHeader.Length = sizeof(ST_GW_VOLTAGE);
        memcpy(pFrame, &tempVoltageStructure, sizeof(ST_GW_VOLTAGE));
        break;
    case 0x02:
        ST_GW_CURRENT tempCurrentStructure;
        tempCurrentStructure.Current = builtinBitswap(data);
        tempCurrentStructure.FrameHeader.Command = command;
        tempCurrentStructure.FrameHeader.Channel = ch;
        tempCurrentStructure.FrameHeader.Length = sizeof(ST_GW_CURRENT);
        memcpy(pFrame, &tempCurrentStructure, sizeof(ST_GW_CURRENT));
        break;
    case 0x03:
        ST_GW_SET_CURRENT tempSetCurrentStructure;
        tempSetCurrentStructure.SetCurrent = builtinBitswap(data);
        tempSetCurrentStructure.FrameHeader.Command = command;
        tempSetCurrentStructure.FrameHeader.Channel = ch;
        tempSetCurrentStructure.FrameHeader.Length = sizeof(ST_GW_NO_DATA);
        memcpy(pFrame, &tempSetCurrentStructure, sizeof(ST_GW_SET_CURRENT));
        break;
    case 0x04:
        ST_GW_ACTIVATION tempActivationStructure;
        tempActivationStructure.Activation = data;
        tempActivationStructure.FrameHeader.Command = command;
        tempActivationStructure.FrameHeader.Channel = ch;
        tempActivationStructure.FrameHeader.Length = sizeof(ST_GW_ACTIVATION);
        memcpy(pFrame, &tempActivationStructure, sizeof(ST_GW_ACTIVATION));
        break;
    default:
        // pFrame = NULL;
        break;
    }

  
    sendFrame(pFrame);
}

boolean getFrame(int *pFrame){
    int index; 
    static int pPacket[25];
    int tempFrame[20];
    if(readSerial(pPacket))
    {
        index = unslipPacket(pPacket,tempFrame);
        if(confirmCheckSum(tempFrame))
        {
            memcpy(pFrame, tempFrame, index);
            return true;
        }
        else{
            return false;
        }

    }
    return false;
}

boolean sendFrame(uint8_t *pFrame)
{
    uint8_t tempPFrame[25] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    uint8_t pPacket[25] = {0};
    int len = 0;
    if (pFrame == NULL)
    {
        return false;
    }
    memcpy(tempPFrame, pFrame, pFrame[0]);
  
    addCheckSum(tempPFrame);
    len = slipPacket(tempPFrame, pPacket);

    writeSerial(pPacket, len);
    return true;
}

//-------------------- local functions --------------------

boolean readSerial(int *readS)
{
    int16_t readData = 0;
    static int8_t readIndex;

    while (Serial.available())
    {
        readData = Serial.read();   
        
        if(readData == 16 || readIndex > 0)
        {
            
            readS[readIndex] = readData;
            
            // Serial.println(readS[readIndex]);

            if (readData == 16 && readIndex > 0)
            {
                Serial.println("Serial done");
                readIndex = 0;
                return true;
            }
            readIndex++;
        }
    }
    return false;
}

void writeSerial(uint8_t *writeS, int len)
{
    int index = 0;
    Serial.write(writeS[index]);
    while (len--)
    {
        index++;
        Serial.write(writeS[index]);
    }
}

int unslipPacket(int *packet, int *frame)
{
    int index = 0;
    int received = 0;
    int c = 0;
    int len = packet[1];

    while(true)
    {
        c = packet[index];
        switch (c)
        {
        case END:

            if(received)
                return received;
            else 
                index++;
                break;
        case ESC:
            c = packet[index+1];
            switch (c)
            {
            case ESC_END:
                c = END;
                break;
            case ESC_ESC:
                c = ESC;
                break;
            }
        default:
            if(received < len)
            {
                frame[received++] = c;   
            }
            index++;
            
        }
    }
}

int slipPacket(uint8_t *f, uint8_t *p){
    int len = f[0];
    int index = 0;
    *p = END;
    index++;


    while (len--)
    {
        p++;
        switch (*f)
        {
        case END:
            *p = ESC;
            p++;
            *p = ESC_END;
            index += 2;
            break;
        case ESC:
            *p = ESC;
            p++;
            *p = ESC_ESC;
            index += 2;
            break;
        default:
            *p = *f;
            index++;
        }
        f++;
    }
    p++;
    *p = END;
    index++;

    return index;
}

void addCheckSum(uint8_t *frame){
    int len = frame[0]; 
    int sum = 0;
    for (int i = 0; i < len - 1; i++)
    {
        sum ^= frame[i];
    }
    frame[len - 1] = sum;
}

boolean confirmCheckSum(int *frame)
{
    int len = frame[0];
    int sum = 0;
    for (int i = 0; i < len - 1; i++)
    {
        sum ^= frame[i];
    }

    if(sum == frame[len - 1])
    {
        return true;
    }
    return false;
}
