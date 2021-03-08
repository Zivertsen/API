#include "API.h"

//--------------------- Function declaration -------------
boolean readSerial(int *readS);

void writeSerial(int *writeS, int len);

//Removes the END and ESC characters from the packet
int unslipPacket(int *packet, int *frame);

//Make a frame ready for being sent, by adding END and ESC charaters
int slipPacket(int *frame, int *packet);

// checks the checksum and remove it 
boolean confirmCheckSum(int *frame);

// calculate the checksum by XOR the values and adds it to the frame
void addCheckSum(int *frame);


// ------------------ Function bodies ------------------------

void buildFrame(uint8_t *pFrame, uint8_t command,uint8_t ch, int data )
{
    uint8_t tempFrame[30];

    switch (command)
    {
    case 0x01:
        ST_GW_VOLTAGE tempStructure;
        tempStructure.Voltage = data;
        tempStructure.FrameHeader.Command = command;
        tempStructure.FrameHeader.Channel = ch;
        tempStructure.FrameHeader.Length = 0;
        memcpy(pframe, &tempStructure, sizeof(ST_GW_VOLTAGE));
        break;
    case 0x02:
        ST_GW_CURRENT tempStructure;
        tempStructure.Voltage = data;
        tempStructure.FrameHeader.Command = command;
        tempStructure.FrameHeader.Channel = ch;
        tempStructure.FrameHeader.Length = 0;
        memcpy(pframe, &tempStructure, sizeof(ST_GW_CURRENT));
        break;
    case 0x03:
        ST_GW_SET_CURRENT tempStructure;
        tempStructure.Voltage = data;
        tempStructure.FrameHeader.Command = command;
        tempStructure.FrameHeader.Channel = ch;
        tempStructure.FrameHeader.Length = 0;
        memcpy(pframe, &tempStructure, sizeof(ST_GW_SET_CURRENT));
        break;
    case 0x04:
        ST_GW_ACTIVATION tempStructure;
        tempStructure.Voltage = data;
        tempStructure.FrameHeader.Command = command;
        tempStructure.FrameHeader.Channel = ch;
        tempStructure.FrameHeader.Length = 0;
        memcpy(pframe, &tempStructure, sizeof(ST_GW_ACTIVATION));
        break;
    default:
        Serial.print("Warring, %d is not a command", command);
        break;
    }
    


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

boolean sendFrame(int *pFrame)
{
    int tempPFrame[25];
    int pPacket[25] = {0};
    int len = 0;
    memcpy(tempPFrame, pFrame, sizeof(pFrame));
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

void writeSerial(int *writeS, int len)
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

int slipPacket(int *f, int *p){
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

void addCheckSum(int *frame){
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
