#include <mcp_can.h>
#include <mcp_can_dfs.h>
#include <SPI.h>
#include <time.h>


//CAN-CH bus ID definitions
#define TCESC_CONTROL       0x384
#define SUSPENSION_CONTROL  0x1FC

//CAN-CH bus data definitions
#define SUSP_SOFT           0x50
#define SUSP_MID            0x40
#define SUSP_FIRM           0x60

//DNA Mode definitions
#define DNA_RACE            0x31
#define DNA_DYNAMIC         0x9
#define DNA_NEUTERED        0x1//might have swapped with A mode
#define DNA_AWFUL           0x11//might have swapped with N mode

#define SPI_CS_CAN          9

MCP_CAN CAN(SPI_CS_CAN);     

byte len = 0;
byte buf[8] = {0};
byte tcesc_buf[8] = {0};

byte last_dna_mode = 0;
byte left_stalk_count = 0;
byte tc_disable = 0;

void print_can_buf(int id, unsigned char len) {
    Serial.print(id, HEX);
    Serial.print(",");
    Serial.print(len);
    Serial.print(",");
    for(int i = 0; i < len; i++) {
      Serial.print(buf[i], HEX);
      Serial.print(",");
    }
    Serial.println();
}

void setup()
{
  Serial.begin(115200);
  while (CAN_OK != CAN.begin(CAN_500KBPS))
  {
    Serial.println("CAN BUS Shield init fail");
    Serial.println(i++);
  }
  Serial.println("CAN BUS Shield init ok!");
  CAN.init_Mask(0, 0, 0x7ff);//did you add the delay(100) to the library per the readme?
  CAN.init_Mask(1, 0, 0x7ff);//did you RTFM?

  CAN.init_Filt(0, 0, TCESC_CONTROL);
  CAN.init_Filt(1, 0, SUSPENSION_CONTROL);
}

void handle_tcesc_control() {
  if(tcesc_buf[1] != DNA_NEUTERED && tcesc_buf[1] != DNA_DYNAMIC && tcesc_buf[1] != DNA_AWFUL && tcesc_buf[1] != DNA_RACE)
    return;//sanity check, don't want to send random shit to module if buffer corrupt/incorrect
  if(tc_disable && tcesc_buf[1] != DNA_RACE)
    tcesc_buf[1] = DNA_RACE;//TCESC disable from Giulia QV in race mode
  else if(!tc_disable && tcesc_buf[1] == DNA_RACE)
    tcesc_buf[1] = DNA_DYNAMIC;//car in race mode wants TC, put TCESC in Dynamic mode
  else
    return;//nothing to do if driver wants TCESC on in non-race mode or off in race-mode.  Car can manage itself
  CAN.sendMsgBuf(TCESC_CONTROL, 0, 8, tcesc_buf);
}


void loop()
{
  unsigned long id = 0;
  if (CAN.checkReceive() == CAN_MSGAVAIL) {//consider switch to interrupt mode
                                          
    CAN.readMsgBufID(&id, &len, buf);
    if(id == TCESC_CONTROL) {
      memcpy(tcesc_buf, buf, 8);
      if(tcesc_buf[3] & 0x40) {//driver pressing left stalk button
        left_stalk_count++;
        if(left_stalk_count > 8) {
          tc_disable ^= 1; //toggle tc_disable between 0 and 1
          left_stalk_count = 0;//reset stalk count for next press event
        }
      }
      else
        left_stalk_count = 0;
      /*
       * let's detect real DNA mode change to/from race for QV cars and change to expected TC/ESC setting when that happens
       */
      if(last_dna_mode == DNA_RACE && tcesc_buf[1] != DNA_RACE)
        tc_disable = 0;
      else if(last_dna_mode != DNA_RACE && tcesc_buf[1] == DNA_RACE)
        tc_disable = 1;
      last_dna_mode = tcesc_buf[1];
    }
    if(id == TCESC_CONTROL || id == SUSPENSION_CONTROL)
      handle_tcesc_control();//added extra check to deal with library/filter bug, don't spam the bus
  }
}
