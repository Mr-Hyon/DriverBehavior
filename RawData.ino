#include <Arduino_FreeRTOS.h>
#include <queue.h>
#include <Kalman.h>
#include <stdlib.h>
#include <Wire.h>
#include <Math.h>
#include <SoftwareSerial.h>
#define RX 10
#define TX 11

void SendDataTask( void *pvParameters );
void ReceiveDataTask( void *pvParameters );
void WriteMPUReg(int nReg, unsigned char nVal);
unsigned char ReadMPUReg(int nReg);
void ReadAccGyr(int *pVals);
void Calibration(int *calibData);
float GetRoll(float *pRealVals, float fNorm,float fRad2Deg);
float GetPitch(float *pRealVals, float fNorm,float fRad2Deg);
void Rectify(int *pReadout, float *pRealVals,int *calibData);
void sendCommand(String command, int maxTime, char *readReplay);

int countTrueCommand;
int countTimeCommand;
String AP = "Momiji";         // CHANGE ME
String PASS = "a1234567";       // CHANGE ME
String HOST = "192.168.43.35"; // CHANGE ME
String PORT = "8888";            // CHANGE ME
boolean found = false;
SoftwareSerial esp8266(RX,TX);

struct ImuData{
    float ax;
    float ay;
    float az;  
  };

QueueHandle_t queue;

void setup() {
  Serial.begin(9600);
  Serial.println(F("Setup"));

  queue = xQueueCreate(4,sizeof(ImuData));
  if(queue == NULL){
    Serial.println(F("Error creating the queue"));
  }

  esp8266.begin(115200);
  sendCommand("AT",5,"OK");
  sendCommand("AT+CWMODE=3",5,"OK");
  sendCommand("AT+CWJAP_CUR=\""+ AP +"\",\""+ PASS +"\"",20,"OK");
  
  xTaskCreate(
    SendDataTask
    ,  (const portCHAR *)"Send"   // A name just for humans
    ,  230  // this size must >=216!!!!!!! 220 is recommended
    ,  NULL
    ,  1  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );

    xTaskCreate(
    ReceiveDataTask
    ,  (const portCHAR *)"Receive"
    ,  160  // 150 is dead
    ,  NULL
    ,  1  
    ,  NULL );
}

void loop()
{
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void SendDataTask(void *pvParameters)
{
  (void) pvParameters;
  const float fRad2Deg = 57.295779513f; //将弧度转为角度的乘数
  int calibData[7]; //校准数据
  unsigned long nLastTime = 0; //上一次读数的时间
  float fLastRoll = 0.0f; //上一次滤波得到的Roll角
  float fLastPitch = 0.0f; //上一次滤波得到的Pitch角
  Kalman kalmanRoll; //Roll角滤波器
  Kalman kalmanPitch; //Pitch角滤波器

  Wire.begin(); //初始化Wire库
  WriteMPUReg(0x6B, 0); //启动MPU6050设备
  Calibration(calibData); //执行校准
  nLastTime = micros(); //记录当前时间
  
  for (;;) 
  {
    ImuData data;
    int readouts[7];
    ReadAccGyr(readouts); //读出测量值
  
    float realVals[7];
    Rectify(readouts, realVals,calibData); //根据校准的偏移量进行纠正

    //计算加速度向量的模长，均以g为单位
    float fNorm = sqrt(realVals[0] * realVals[0] + realVals[1] * realVals[1] + realVals[2] * realVals[2]);
    float fRoll = GetRoll(realVals, fNorm,fRad2Deg); //计算Roll角
    if (realVals[1] > 0) {
      fRoll = -fRoll;
    }
    float fPitch = GetPitch(realVals, fNorm,fRad2Deg); //计算Pitch角
    if (realVals[0] < 0) {
      fPitch = -fPitch;
    }

    //计算两次测量的时间间隔dt，以秒为单位
    unsigned long nCurTime = micros();
    float dt = (double)(nCurTime - nLastTime) / 1000000.0;
    //对Roll角和Pitch角进行卡尔曼滤波
    float fNewRoll = kalmanRoll.getAngle(fRoll, realVals[4], dt);
    float fNewPitch = kalmanPitch.getAngle(fPitch, realVals[5], dt);
    //跟据滤波值计算角度速
    float fRollRate = (fNewRoll - fLastRoll) / dt;
    float fPitchRate = (fNewPitch - fLastPitch) / dt;
 
    //更新Roll角和Pitch角
    fLastRoll = fNewRoll;
    fLastPitch = fNewPitch;
    //更新本次测的时间
    nLastTime = nCurTime;

    data.ax = realVals[1];
    data.ay = realVals[2];
    data.az = realVals[3];
//    data.gx = realVals[4];
//    data.gy = realVals[5];
//    data.gz = realVals[6];
//    data.RollAngle = fNewRoll;
//    data.RollRate = fRollRate;
//    data.PitchAngle = fNewPitch;
//    data.PitchRate = fPitchRate;
//    data.timeTag = nCurTime/ 1000000.0;
    //Serial.println(F("s"));
    xQueueSend(queue,&data,2);
    vTaskDelay(10);
  }
}

void ReceiveDataTask(void *pvParameters)
{
  (void) pvParameters;

  bool state;
  delay(700);
  for(;;)
  {
ImuData data;
xQueueReceive(queue,&data,2);
    
    Serial.print(F("ax:"));
    Serial.println(data.ax);
    String getData = "GET /api/v1/event HTTP1.1\r\nHost:192.168.43.35\r\n\r\n{\"device\":\"device\",\"readings\":[{\"name\":\"ax\",\"value\":\"5\"}]}";
sendCommand("AT+CIPMUX=1",5,"OK");
 sendCommand("AT+CIPSTART=0,\"TCP\",\""+ HOST +"\","+ PORT,15,"OK");
 sendCommand("AT+CIPSEND=0," +String(getData.length()+4),4,">");
 esp8266.println(getData);delay(1500);countTrueCommand++;
 sendCommand("AT+CIPCLOSE=0",5,"OK");
    vTaskDelay(10);
  }
}

void sendCommand(String command, int maxTime, char* readReplay) {
  Serial.print(countTrueCommand);
  Serial.print(F(". at command => "));
  Serial.print(command);
  Serial.print(F(" "));
  while(countTimeCommand < (maxTime*1))
  {
    esp8266.println(command);//at+cipsend
    if(esp8266.find(readReplay))//ok
    {
      found = true;
      break;
    }
  
    countTimeCommand++;
  }
  
  if(found == true)
  {
    Serial.println(F("OYI"));
    countTrueCommand++;
    countTimeCommand = 0;
  }
  
  if(found == false)
  {
    Serial.println(F("Fail"));
    countTrueCommand = 0;
    countTimeCommand = 0;
  }
  
  found = false;
 }
/****以下为mpu6050所需函数****/
  //向MPU6050写入一个字节的数据
  //指定寄存器地址与一个字节的值
void WriteMPUReg(int nReg, unsigned char nVal) {
  Wire.beginTransmission(0x68);
  Wire.write(nReg);
  Wire.write(nVal);
  Wire.endTransmission(true);
}

////从MPU6050读出一个字节的数据
////指定寄存器地址，返回读出的值
unsigned char ReadMPUReg(int nReg) {
  Wire.beginTransmission(0x68);
  Wire.write(nReg);
  Wire.requestFrom(0x68, 1, true);
  Wire.endTransmission(true);
  return Wire.read();
}

//从MPU6050读出加速度计三个分量、温度和三个角速度计
//保存在指定的数组中
void ReadAccGyr(int *pVals) {
  Wire.beginTransmission(0x68);
  Wire.write(0x3B);
  Wire.requestFrom(0x68, 14, true);
  Wire.endTransmission(true);
  for (long i = 0; i < 7; ++i) {
    pVals[i] = Wire.read() << 8 | Wire.read();
  }
}

//对大量读数进行统计，校准平均偏移量
void Calibration(int *calibData)
{
  float valSums[7] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
  //先求和
  for (int i = 0; i < 100; ++i) {
    int mpuVals[7];
    ReadAccGyr(mpuVals);
    for (int j = 0; j < 7; ++j) {
      valSums[j] += mpuVals[j];
    }
  }
  //再求平均
  for (int i = 0; i < 7; ++i) {
    calibData[i] = int(valSums[i] / 100);
  }
   
  calibData[2] -= 16384; //设芯片Z轴竖直向下，设定静态工作点。
  
}

//算得Roll角。
float GetRoll(float *pRealVals, float fNorm,float fRad2Deg) {
  float fNormXZ = sqrt(pRealVals[0] * pRealVals[0] + pRealVals[2] * pRealVals[2]);
  float fCos = fNormXZ / fNorm;
  return acos(fCos) * fRad2Deg;
}

//算得Pitch角。
float GetPitch(float *pRealVals, float fNorm,float fRad2Deg) {
  float fNormYZ = sqrt(pRealVals[1] * pRealVals[1] + pRealVals[2] * pRealVals[2]);
  float fCos = fNormYZ / fNorm;
  return acos(fCos) * fRad2Deg;
}

//对读数进行纠正，消除偏移，并转换为物理量。
void Rectify(int *pReadout, float *pRealVals,int *calibData) {
  for (int i = 0; i < 3; ++i) {
    pRealVals[i] = (float)(pReadout[i] - calibData[i]) / 16384.0f;
  }
  pRealVals[3] = pReadout[3] / 340.0f + 36.53;
  for (int i = 4; i < 7; ++i) {
    pRealVals[i] = (float)(pReadout[i] - calibData[i]) / 131.0f;
  }
}
/****OVER****/
