#include <SoftwareSerial.h>
SoftwareSerial T(7, 8); // RX, TX
#include <MsTimer2.h>
float Data[4];
unsigned short CRC_CHECK(unsigned char *Buf, unsigned char CRC_CNT)
{
    unsigned short CRC_Temp;
    unsigned char i,j;
    CRC_Temp = 0xffff;

    for (i=0;i<CRC_CNT; i++){
        CRC_Temp ^= Buf[i];
        for (j=0;j<8;j++) {
            if (CRC_Temp & 0x01)
                CRC_Temp = (CRC_Temp >>1 ) ^ 0xa001;
            else
                CRC_Temp = CRC_Temp >> 1;
        }
    }
    return(CRC_Temp);
}

void Plot(float S_Out[])
{
  int temp[4] = {0};
  unsigned int temp1[4] = {0};
  unsigned char databuf[10] = {0};
  unsigned char i;
  unsigned short CRC16 = 0;
 // float SDS_OutData[4];
  /*for(i=0;i<4;i++) {
  SDS_OutData[i]=S_Out[i];
  }*/
  for(i=0;i<4;i++)
   {

    temp[i]  = (int)Data[i];
    temp1[i] = (unsigned int)temp[i];

   }

  for(i=0;i<4;i++)
  {
    databuf[i*2]   = (unsigned char)(temp1[i]%256);
    databuf[i*2+1] = (unsigned char)(temp1[i]/256);
  }

  CRC16 = CRC_CHECK(databuf,8);
  databuf[8] = CRC16%256;
  databuf[9] = CRC16/256;

  //SDS_UART_Init();
  for(i=0;i<10;i++)
  {
    Serial.write(databuf[i]);
  }
}
/************************************************************/
uint16_t AD[4]={0};
uint16_t buff[4][10];
#define ABS(x) ( (x)>0?(x):-(x) )
#define LIMIT( x,min,max ) ( (x) < (min)  ? (min) : ( (x) > (max) ? (max) : (x) ) )
uint16_t SlidingFilter(uint16_t buff[],byte n,uint16_t data){
  byte i=0;
  int sum=0;
  for(i=0;i<n-1;i++)buff[i]=buff[i+1];
  buff[n-1]=data;
  for(i=0;i<n;i++)sum+=buff[i];
  return sum/n;
}
byte Flag=0;
int16_t target=0;
byte Stop=0;
byte Calibrate=0;
void IRQ(void){
  static boolean output = HIGH;
  Flag++;
  if(Flag>=250)Flag=0;
  AD[0]=SlidingFilter(buff[0],10,analogRead(0));
  if(Flag==ABS(target)&&(Stop||Calibrate)){
    Flag=0;
    if(Stop)T.write('s');
    if(Calibrate)T.write('r');
    digitalWrite(13,HIGH);
  }else if(Flag==ABS(target)&&(!Stop&&!Calibrate)){
    Flag=0;
     if(target>0)T.write('+');
    else if(target<0)T.write('-');
    digitalWrite(13, output);
     output = !output;
  }else if(target==0){
    if(Flag==249)T.write('0');
    digitalWrite(13,HIGH);
  }
}
void setup() {
 pinMode(13, OUTPUT);//LED
 Serial.begin(115200);
 T.begin(115200);
 MsTimer2::set(5, IRQ); // 500ms period
 MsTimer2::start();
 sei();
}
void loop() {
  if(AD[0]<20){
    target=-20;
    Stop=1;
    Calibrate=0;
  }else if(AD[0]>=20&&AD[0]<216){
    target=-20;
    Stop=0;
    Calibrate=0;
  }else if(AD[0]>=216&&AD[0]<413){
    target=-130;
    Stop=0;
    Calibrate=0;
  }else if(AD[0]>=413&&AD[0]<610){
    target=0;
    Stop=0;
    Calibrate=0;
  }else if(AD[0]>=610&&AD[0]<806){
    target=130;
    Stop=0;
    Calibrate=0;
  }else if(AD[0]>=806&&AD[0]<=1004){
    target=20;
    Stop=0;
    Calibrate=0;
  }else if(AD[0]>1004){
    Stop=0;
    Calibrate=1;
    target=20;
  }
  Data[0]=AD[0];
  Plot(Data);
  delay(5);
}
