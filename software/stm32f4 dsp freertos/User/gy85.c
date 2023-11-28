
/*******************************************************************************
*  函数功能：串口2发送数据  HMC5883L	+ ADXL345 + BMP085+L3G4200D(串口2在mian初始化完成)
*  波特率：115200
*  时间：2020.7.12
*  修改注释：全
*******************************************************************************/

/*芯片说明
 *ADXL345：三轴加速度计，测量Roll、Pitch（航偏角Yaw无法测量）
 *L3G4200D：三轴陀螺仪，测量Roll、Pitch、Yaw（配合加速度计得出准确温度的三个角度）
 *HMC5883L：电子罗盘，测量与地磁北极的偏转角度
 *BMP085：气压传感器，测量大气压、温度
*/
#include "stm32f4xx.h"                  // Device header

#include "gy85.h"

GPIO_InitTypeDef GPIO_InitStructure;
ErrorStatus HSEStartUpStatus;


#define   FALSE 0
#define   TRUE  1

#define   u8 unsigned char
#define   u32 unsigned int
#define   uchar unsigned char
#define   uint unsigned int	
#define   OSS 0	 //	BMP085使用

//L3G4200D内部寄存器
#define CTRL_REG1 0x20
#define CTRL_REG2 0x21
#define CTRL_REG3 0x22
#define CTRL_REG4 0x23
#define CTRL_REG5 0x24
#define OUT_X_L 0x28
#define OUT_X_H 0x29
#define OUT_Y_L 0x2A
#define OUT_Y_H 0x2B
#define OUT_Z_L 0x2C
#define OUT_Z_H 0x2D

//定义器件在IIC总线中的从地址,根据ALT  ADDRESS地址引脚不同修改
#define	HMC5883L_Addr   0x3C	//磁场传感器器件地址
#define	ADXL345_Addr    0xA6	//加速度传感器器件地址
#define	BMP085_Addr     0xee	//气压传感器器件地址
#define	L3G4200_Addr    0xD2	//陀螺仪传感器器件地址

unsigned char BUF[8];                   //接收数据缓存区，作用是暂时缓存寄存器的值，之后做算法分析
char  test=0; 
int   x,y;
uchar ge,shi,bai,qian,wan,shiwan;           //uart,显示变量


short T_X,T_Y,T_Z;                     //陀螺仪输出数据
float angle;                           //电子罗盘输出角度，与地磁北极的偏转角度

/* 加速度计ADXL345 数据 */
int A_X,A_Y,A_Z;                       //加速度计读取的寄存器数据
short data_xyz[3];                     //加速度计合成数据
float Q,T,K;                           //加速度计x、y、z原始数据
float Roll,Pitch;                      //Roll,Pitch角度


/* BMP085使用的变量 */
short ac1;
short ac2; 
short ac3; 
unsigned short ac4;
unsigned short ac5;
unsigned short ac6;
short b1; 
short b2;
short mb;
short mc;
short md;

long  temperature;   //温度
long  pressure;      //气压


 //************************************++++++++++++++++++++++++++++++++
/*模拟IIC端口输出输入定义
 *SDA:PB9
 *SCL:PB8
*/
#define SCL_H         GPIO_WriteBit(GPIOB, GPIO_Pin_8, (BitAction)(1))
#define SCL_L         GPIO_WriteBit(GPIOB, GPIO_Pin_8, (BitAction)(0))
   
#define SDA_H         GPIO_WriteBit(GPIOB, GPIO_Pin_9, (BitAction)(1))
#define SDA_L         GPIO_WriteBit(GPIOB, GPIO_Pin_9, (BitAction)(0))

#define SCL_read      GPIOB->IDR  & GPIO_Pin_8
#define SDA_read      GPIOB->IDR  & GPIO_Pin_9



/* 转换数据 */
void conversion(long temp_data)  
{     
    shiwan=temp_data/100000+0x30 ;
    temp_data=temp_data%100000;   //取余运算 
    wan=temp_data/10000+0x30 ;
    temp_data=temp_data%10000;   //取余运算
	qian=temp_data/1000+0x30 ;
    temp_data=temp_data%1000;    //取余运算
    bai=temp_data/100+0x30   ;
    temp_data=temp_data%100;     //取余运算
    shi=temp_data/10+0x30    ;
    temp_data=temp_data%10;      //取余运算
    ge=temp_data+0x30; 	
}
/*******************************************************************************
* Function Name  : I2C_GPIO_Config
* Description    : Configration Simulation IIC GPIO
* Input          : None 
* Output         : None
* Return         : None
****************************************************************************** */
void I2C_GPIO_Config(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure; 
 
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_8|GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

}

/*******************************************************************************
* Function Name  : I2C_delay
* Description    : Simulation IIC Timing series delay
* Input          : None
* Output         : None
* Return         : None
****************************************************************************** */
void I2C_delay(void)
{
		
   u8 i=30; //这里可以优化速度	，经测试最低到5还能写入
   while(i) 
   { 
     i--; 
   }  
}

void delay5ms(void)
{
		
   int i=5000;  
   while(i) 
   { 
     i--; 
   }  
}
/*******************************************************************************
* Function Name  : I2C_Start
* Description    : Master Start Simulation IIC Communication
* Input          : None
* Output         : None
* Return         : Wheather	 Start
****************************************************************************** */
uint8_t I2C_Start(void)
{
	SDA_H;
	SCL_H;
	I2C_delay();
	if(!SDA_read)return FALSE;	//SDA线为低电平则总线忙,退出
	SDA_L;
	I2C_delay();
	if(SDA_read) return FALSE;	//SDA线为高电平则总线出错,退出
	SDA_L;
	I2C_delay();
	return TRUE;
}
/*******************************************************************************
* Function Name  : I2C_Stop
* Description    : Master Stop Simulation IIC Communication
* Input          : None
* Output         : None
* Return         : None
****************************************************************************** */
void I2C_Stop(void)
{
	SCL_L;
	I2C_delay();
	SDA_L;
	I2C_delay();
	SCL_H;
	I2C_delay();
	SDA_H;
	I2C_delay();
} 
/*******************************************************************************
* Function Name  : I2C_Ack
* Description    : Master Send Acknowledge Single
* Input          : None
* Output         : None
* Return         : None
****************************************************************************** */
void I2C_Ack(void)
{	
	SCL_L;
	I2C_delay();
	SDA_L;
	I2C_delay();
	SCL_H;
	I2C_delay();
	SCL_L;
	I2C_delay();
}   
/*******************************************************************************
* Function Name  : I2C_NoAck
* Description    : Master Send No Acknowledge Single
* Input          : None
* Output         : None
* Return         : None
****************************************************************************** */
void I2C_NoAck(void)
{	
	SCL_L;
	I2C_delay();
	SDA_H;
	I2C_delay();
	SCL_H;
	I2C_delay();
	SCL_L;
	I2C_delay();
} 
/*******************************************************************************
* Function Name  : I2C_WaitAck
* Description    : Master Reserive Slave Acknowledge Single
* Input          : None
* Output         : None
* Return         : Wheather	 Reserive Slave Acknowledge Single
****************************************************************************** */
uint8_t I2C_WaitAck(void) 	 //返回为:=1有ACK,=0无ACK
{
	SCL_L;
	I2C_delay();
	SDA_H;			
	I2C_delay();
	SCL_H;
	I2C_delay();
	if(SDA_read)
	{
      SCL_L;
	  I2C_delay();
      return FALSE;
	}
	SCL_L;
	I2C_delay();
	return TRUE;
}
/*******************************************************************************
* Function Name  : I2C_SendByte
* Description    : Master Send a Byte to Slave
* Input          : Will Send Date
* Output         : None
* Return         : None
****************************************************************************** */
void I2C_SendByte(u8 SendByte) //数据从高位到低位//
{
    u8 i=8;
    while(i--)
    {
        SCL_L;
        I2C_delay();
      if(SendByte&0x80)
        SDA_H;  
      else 
        SDA_L;   
        SendByte<<=1;
        I2C_delay();

		SCL_H;
        I2C_delay();
    }
    SCL_L;
}  
/*******************************************************************************
* Function Name  : I2C_RadeByte
* Description    : Master Reserive a Byte From Slave
* Input          : None
* Output         : None
* Return         : Date From Slave 
****************************************************************************** */
unsigned char I2C_RadeByte(void)  //数据从高位到低位//
{ 
    u8 i=8;
    u8 ReceiveByte=0;

    SDA_H;				
    while(i--)
    {
      ReceiveByte<<=1;      
      SCL_L;
      I2C_delay();
	  SCL_H;
      I2C_delay();	
      if(SDA_read)
      {
        ReceiveByte|=0x01;
      }
    }
    SCL_L;
    return ReceiveByte;
} 
//ZRX          
//单字节写入*******************************************

uint8_t Single_Write(unsigned char SlaveAddress,unsigned char REG_Address,unsigned char REG_data)		     //void
{
  	if(!I2C_Start())return FALSE;
    I2C_SendByte(SlaveAddress);   //发送设备地址+写信号//I2C_SendByte(((REG_Address & 0x0700) >>7) | SlaveAddress & 0xFFFE);//设置高起始地址+器件地址 
    if(!I2C_WaitAck()){I2C_Stop(); return FALSE;}
    I2C_SendByte(REG_Address );   //设置低起始地址      
    I2C_WaitAck();	
    I2C_SendByte(REG_data);
    I2C_WaitAck();   
    I2C_Stop(); 
    delay5ms();
    return TRUE;
}

//单字节读取*****************************************
unsigned char Single_Read(unsigned char SlaveAddress,unsigned char REG_Address)
{   unsigned char REG_data;     	
	if(!I2C_Start())return FALSE;
    I2C_SendByte(SlaveAddress); //I2C_SendByte(((REG_Address & 0x0700) >>7) | REG_Address & 0xFFFE);//设置高起始地址+器件地址 
    if(!I2C_WaitAck()){I2C_Stop();test=1; return FALSE;}
    I2C_SendByte((u8) REG_Address);   //设置低起始地址      
    I2C_WaitAck();
    I2C_Start();
    I2C_SendByte(SlaveAddress+1);
    I2C_WaitAck();

	REG_data= I2C_RadeByte();
    I2C_NoAck();
    I2C_Stop();
    //return TRUE;
	return REG_data;

}						      

/*
********************************************************************************
** 函数名称 ： Delay(vu32 nCount)
** 函数功能 ： 延时函数
** 输    入	： 无
** 输    出	： 无
** 返    回	： 无
********************************************************************************
*/
 void Delay(u32 nCount)
{
  for(; nCount != 0; nCount--);
}

/*
********************************************************************************
** 函数名称 ： void Delayms(vu32 m)
** 函数功能 ： 长延时函数	 m=1,延时1ms
** 输    入	： 无
** 输    出	： 无
** 返    回	： 无
********************************************************************************
*/
 void Delayms(u32 m)
{
  u32 i;
  
  for(; m != 0; m--)	
       for (i=0; i<50000; i++);
}




 //************************************************
void  USART2_SendData(unsigned char SendData)
{
USART_SendData(USART2, SendData);
Delayms(1);
}
	

//********************************************************************
long bmp085ReadTemp(void)
{   short  temp_ut;
	Single_Write(BMP085_Addr,0xF4,0x2E);
	Delayms(5);	// max time is 4.5ms
	temp_ut = Single_Read(BMP085_Addr,0xF6);
	temp_ut = (temp_ut<<8)| Single_Read(BMP085_Addr,0xF7);		
	return (long) temp_ut ;
}
//*************************************************************

long bmp085ReadPressure(void)
{
	long pressure = 0;
	Single_Write(BMP085_Addr,0xF4,0x34);
	Delayms(5);	// max time is 4.5ms
	pressure = Single_Read(BMP085_Addr,0xF6);
	pressure = (pressure<<8)| Single_Read(BMP085_Addr,0xF7);		
	pressure &= 0x0000FFFF;	
	return pressure;	
}

 //******************
void Send_ADXL345_data(int dis_data)
  { 
    float temp ;
	if(dis_data>0x7fff)dis_data-=0xffff;
 	if(dis_data<0){
  	     dis_data=-dis_data;
	     USART_SendData(USART1,'-');
		 Delayms(2);
	}
	else
	  { 
	   USART_SendData(USART1,'+');
	   Delayms(2);
	   }
        temp=(float)dis_data*3.9;  //计算数据和显示,查考ADXL345快速入门第4页
        conversion(temp);          //转换出显示需要的数据	 
	    USART2_SendData(qian);
		USART2_SendData('.');
	    USART2_SendData(bai); 
	    USART2_SendData(shi); 
	    USART2_SendData(ge); 
		USART2_SendData('g'); 
}
//***************************************
 //******************
void Send_L3G420D_data(short dis_data)
  { float temp ;
 	if(dis_data<0){
  	     dis_data=-dis_data;
	     USART_SendData(USART1,'-');
		 Delayms(2);
	}
	else
	  { 
	   USART_SendData(USART1,'+');
	   Delayms(2);
	   }
        temp=(float)dis_data*0.07;  //计算数据和显示,查考ADXL345快速入门第4页
        conversion(temp);           //转换出显示需要的数据	 
	    USART2_SendData(bai); 
	    USART2_SendData(shi); 
	    USART2_SendData(ge); 
 
}
//***************************************


void Send_HMC5883L(void)
{
USART2_SendData('H');
USART2_SendData('M');
USART2_SendData('C');
USART2_SendData('5');
USART2_SendData('8');
USART2_SendData('8');
USART2_SendData('3');
USART2_SendData('L');
USART2_SendData(':');
conversion(angle);
USART2_SendData(bai); 
USART2_SendData(shi); 
USART2_SendData(ge); 
USART2_SendData('`');  
USART2_SendData(0x0d);
USART2_SendData(0x0a);
}
//*************************************************
void  Send_BMP085(void)
{
USART2_SendData('B');
USART2_SendData('M');
USART2_SendData('P');
USART2_SendData('0');
USART2_SendData('8');
USART2_SendData('5');
USART2_SendData(':');

USART2_SendData('t');
USART2_SendData('=');
conversion(temperature);
USART2_SendData(bai); 
USART2_SendData(shi); 
USART2_SendData('.');
USART2_SendData(ge);
USART2_SendData('`'); 
USART2_SendData('C'); 

USART2_SendData(' ');
USART2_SendData('p');
USART2_SendData('=');
conversion(pressure);

USART2_SendData(shiwan);
USART2_SendData(wan);
USART2_SendData(qian);
USART2_SendData('.');
USART2_SendData(bai); 
USART2_SendData(shi); 
USART2_SendData(ge); 
USART2_SendData('K'); 
USART2_SendData('p'); 
USART2_SendData('a');  				
USART2_SendData(0x0d);
USART2_SendData(0x0a);

}
//*****************************************************

void  Send_ADXL345(void)
{
USART2_SendData('A');
USART2_SendData('D');
USART2_SendData('X');
USART2_SendData('L');
USART2_SendData('3');
USART2_SendData('4');
USART2_SendData('5');
USART2_SendData(':');
	   USART2_SendData('X');
	   USART2_SendData('=');
	   Send_ADXL345_data(A_X);

	   USART2_SendData('Y');
	   USART2_SendData('=');
	   Send_ADXL345_data(A_Y);

	   USART2_SendData('Z');
	   USART2_SendData('=');
	   Send_ADXL345_data(A_Z);

	   adxl345_angle();


USART2_SendData(0x0d);
USART2_SendData(0x0a);
}
 //*****************************************************
void Send_L3G4200D(void)
 {
   	USART2_SendData('L');
	USART2_SendData('3');
	USART2_SendData('G');
	USART2_SendData('4');
	USART2_SendData('2');
	USART2_SendData('0');
	USART2_SendData('0');
	USART2_SendData('D');
	USART2_SendData(':');

	   USART2_SendData('X');
	   USART2_SendData('=');
	   Send_L3G420D_data(T_X);

	   USART2_SendData('Y');
	   USART2_SendData('=');
	   Send_L3G420D_data(T_Y);

	   USART2_SendData('Z');
	   USART2_SendData('=');
	   Send_L3G420D_data(T_Z);

	  USART2_SendData(0x0d);
     USART2_SendData(0x0a);
 }
//******************************************************
void read_HMC5883L(void)
{
       Single_Write(HMC5883L_Addr,0x00,0x14);   //
       Single_Write(HMC5883L_Addr,0x02,0x00);   //
  	   Delayms(10);

       BUF[1]=Single_Read(HMC5883L_Addr,0x03);//OUT_X_L_A
       BUF[2]=Single_Read(HMC5883L_Addr,0x04);//OUT_X_H_A

	   BUF[3]=Single_Read(HMC5883L_Addr,0x07);//OUT_Y_L_A
       BUF[4]=Single_Read(HMC5883L_Addr,0x08);//OUT_Y_H_A

       x=(BUF[1] << 8) | BUF[2]; //Combine MSB and LSB of X Data output register
       y=(BUF[3] << 8) | BUF[4]; //Combine MSB and LSB of Z Data output register

       if(x>0x7fff)x-=0xffff;	  
       if(y>0x7fff)y-=0xffff;	  
       angle= atan2(y,x) * (180 / 3.14159265) + 180; // angle in degrees
}
//****************************************
void read_ADXL345(void)
{
       BUF[0]=Single_Read(ADXL345_Addr,0x32);//OUT_X_L_A
       BUF[1]=Single_Read(ADXL345_Addr,0x33);//OUT_X_H_A

	   BUF[2]=Single_Read(ADXL345_Addr,0x34);//OUT_Y_L_A
       BUF[3]=Single_Read(ADXL345_Addr,0x35);//OUT_Y_H_A

	   BUF[4]=Single_Read(ADXL345_Addr,0x36);//OUT_Z_L_A
       BUF[5]=Single_Read(ADXL345_Addr,0x37);//OUT_Z_H_A

	   A_X=(BUF[1]<<8)+BUF[0];  //合成数据  
	   A_Y=(BUF[3]<<8)+BUF[2];  //合成数据
	   A_Z=(BUF[5]<<8)+BUF[4];  //合成数据
}
//*****************************************
void read_BMP085(void)
{

   	long ut;
	long up;
	long x1, x2, b5, b6, x3, b3, p;
	unsigned long b4, b7;

	ut = bmp085ReadTemp();	   // 读取温度
//	ut = bmp085ReadTemp();	   // 读取温度
	up = bmp085ReadPressure();  // 读取压强
//	up = bmp085ReadPressure();  // 读取压强
	x1 = ((long)ut - ac6) * ac5 >> 15;
	x2 = ((long) mc << 11) / (x1 + md);
	b5 = x1 + x2;
	temperature = (b5 + 8) >> 4;
	
	 //****************

	b6 = b5 - 4000;
	x1 = (b2 * (b6 * b6 >> 12)) >> 11;
	x2 = ac2 * b6 >> 11;
	x3 = x1 + x2;
	b3 = (((long)ac1 * 4 + x3) + 2)/4;
	x1 = ac3 * b6 >> 13;
	x2 = (b1 * (b6 * b6 >> 12)) >> 16;
	x3 = ((x1 + x2) + 2) >> 2;
	b4 = (ac4 * (unsigned long) (x3 + 32768)) >> 15;
	b7 = ((unsigned long) up - b3) * (50000 >> OSS);
	if( b7 < 0x80000000)
	     p = (b7 * 2) / b4 ;
           else  
		    p = (b7 / b4) * 2;
	x1 = (p >> 8) * (p >> 8);
	x1 = (x1 * 3038) >> 16;
	x2 = (-7357 * p) >> 16;
	 pressure = p + ((x1 + x2 + 3791) >> 4);
}
//***************************
void  Init_ADXL345(void)
{
   Single_Write(ADXL345_Addr,0x31,0x0B);   //测量范围,正负16g，13位模式
  // Single_Write(ADXL345_Addr,0x2C,0x0e);   //速率设定为100hz 参考pdf13页
   Single_Write(ADXL345_Addr,0x2D,0x08);   //选择电源模式   参考pdf24页
   Single_Write(ADXL345_Addr,0x2E,0x80);   //使能 DATA_READY 中断
  // Single_Write(ADXL345_Addr,0x1E,0x00);   //X 偏移量 根据测试传感器的状态写入pdf29页
  // Single_Write(ADXL345_Addr,0x1F,0x00);   //Y 偏移量 根据测试传感器的状态写入pdf29页
  // Single_Write(ADXL345_Addr,0x20,0x05);   //Z 偏移量 根据测试传感器的状态写入pdf29页
}
//****************************
 void  Init_BMP085(void)
{
	ac1 = Single_Read(BMP085_Addr,0xAA);
	ac1 = (ac1<<8)|Single_Read(BMP085_Addr,0xAB);

    ac2 = Single_Read(BMP085_Addr,0xAC);
	ac2 = (ac2<<8)| Single_Read(BMP085_Addr,0xAD);

	ac3 = Single_Read(BMP085_Addr,0xAE);
	ac3 = (ac3<<8)| Single_Read(BMP085_Addr,0xAF);

	ac4 = Single_Read(BMP085_Addr,0xB0);
	ac4 = (ac4<<8)| Single_Read(BMP085_Addr,0xB1);

	ac5 = Single_Read(BMP085_Addr,0xB2);
	ac5 = (ac5<<8)| Single_Read(BMP085_Addr,0xB3);

	ac6 = Single_Read(BMP085_Addr,0xB4);
	ac6 = (ac6<<8)| Single_Read(BMP085_Addr,0xB5);

	b1 = Single_Read(BMP085_Addr,0xB6);
	b1 = (b1<<8)| Single_Read(BMP085_Addr,0xB7);

	b2 = Single_Read(BMP085_Addr,0xB8);
	b2 = (b2<<8)| Single_Read(BMP085_Addr,0xB9);

	mb = Single_Read(BMP085_Addr,0xBA);
	mb = (mb<<8)| Single_Read(BMP085_Addr,0xBB);

	mc = Single_Read(BMP085_Addr,0xBC);
	mc = (mc<<8)| Single_Read(BMP085_Addr,0xBD);

	md = Single_Read(BMP085_Addr,0xBE);
	md = (md<<8)| Single_Read(BMP085_Addr,0xBF);

}
//****************************
 void  Init_HMC5883L(void)
{
   Single_Write(HMC5883L_Addr,0x00,0x14);   //
   Single_Write(HMC5883L_Addr,0x02,0x00);   //
}
//*****************************************

 //************初始化L3G4200D*********************************
void Init_L3G4200D(void)
{
	Single_Write(L3G4200_Addr,CTRL_REG1, 0x0f);
	Single_Write(L3G4200_Addr,CTRL_REG2, 0x00);
	Single_Write(L3G4200_Addr,CTRL_REG3, 0x08);
	Single_Write(L3G4200_Addr,CTRL_REG4, 0x30);	//+-2000dps
	Single_Write(L3G4200_Addr,CTRL_REG5, 0x00);
}	
//******读取L3G4200D数据****************************************
void read_L3G4200D(void)
{
   BUF[0]=Single_Read(L3G4200_Addr,OUT_X_L);
   BUF[1]=Single_Read(L3G4200_Addr,OUT_X_H);
   T_X=	(BUF[1]<<8)|BUF[0];
 


   BUF[2]=Single_Read(L3G4200_Addr,OUT_Y_L);
   BUF[3]=Single_Read(L3G4200_Addr,OUT_Y_H);
   T_Y=	(BUF[3]<<8)|BUF[2];
  

   BUF[4]=Single_Read(L3G4200_Addr,OUT_Z_L);
   BUF[5]=Single_Read(L3G4200_Addr,OUT_Z_H);
   T_Z=	(BUF[5]<<8)|BUF[4];

}

 //******************ADXL345计算倾斜角度************
void adxl345_angle(void)
 {

data_xyz[0]=A_X;  //合成数据   
data_xyz[1]=A_Y;  //合成数据   
data_xyz[2]=A_Z;  //合成数据   

//分别是加速度X,Y,Z的原始数据，10位的
Q=(float)data_xyz[0]*3.9;
T=(float)data_xyz[1]*3.9;
K=(float)data_xyz[2]*3.9;
Q=-Q;

  Roll=(float)(((atan2(K,Q)*180)/3.14159265)+180);    //X轴角度值
  Pitch=(float)(((atan2(K,T)*180)/3.14159265)+180);  //Y轴角度值
  //conversion(Pitch);								   //需要显示时候请更换
  conversion(Roll);
        USART2_SendData(' '); 
		USART2_SendData(' '); 
  	    USART2_SendData(bai); 
	    USART2_SendData(shi); 
	    USART2_SendData(ge);
}

int main(void)
{
	DEBUG_USART_Config();
	Init_HMC5883L();
  Init_ADXL345();
  Init_BMP085();
  Init_L3G4200D();
	
	while(1)
	{
		read_HMC5883L();
       Send_HMC5883L();
		read_ADXL345();
	   Send_ADXL345();
		read_L3G4200D();
	    Send_L3G4200D();
		read_BMP085();		   
	   Send_BMP085();
		Delayms(20);
	}
}
///*************结束***************/

