/***************************************************************************
* 因为此版本的IIC通信没有ACK信号返回，所以现成的关于IIC通信的库不能用于本品。
*
* 第一次使用Arduino，也不太熟悉C++，所以这个库在使用起来可能不如Adafruit或
* U8glib的库那样方便……理解万岁。比如设置字体大小、显示任意尺寸BMP图片等功能，
* 后续会完善功能的！！！！
*
* 最新版本的“IIC_wtihout_ACK”会发布在 http://www.14blog.com/archives/1358
* 
* 建议和问题反馈，请发邮件至 hello14blog@gmail.com
***************************************************************************/

#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#include "IIC_without_ACK.h"
#include "oledfont.c"

//定义类
IIC_without_ACK::IIC_without_ACK(int sda, int scl)
{
  _sda = sda;
  _scl = scl;
  pinMode(sda, OUTPUT);
  pinMode(scl, OUTPUT);
}


//开启IIC总线
void IIC_without_ACK::IIC_Start()
{
  digitalWrite(_scl, HIGH);
  digitalWrite(_sda, HIGH);
  digitalWrite(_sda, LOW);
  digitalWrite(_scl, LOW);
}


//停止IIC总线
void IIC_without_ACK::IIC_Stop()
{
  digitalWrite(_scl, LOW);
  digitalWrite(_sda, LOW);
  digitalWrite(_scl, HIGH);
  digitalWrite(_sda, HIGH);
}


//通过IIC写一个8位的数据,比如0xff
void IIC_without_ACK::Write_IIC_Byte(unsigned char IIC_Byte)
{
  unsigned char i;
  for(i=0;i<8;i++)
  {
    if((IIC_Byte << i) & 0x80)digitalWrite(_sda, HIGH);
    else
      digitalWrite(_sda, LOW);
    digitalWrite(_scl, HIGH);
    digitalWrite(_scl, LOW);
//    IIC_Byte<<=1;
   }
  digitalWrite(_sda, HIGH);
  digitalWrite(_scl, HIGH);
  digitalWrite(_scl, LOW);
}


//IIC写命令 -- 命令操作很少，对速度影响不大
void IIC_without_ACK::Write_IIC_Command(unsigned char IIC_Command)
{
   IIC_Start();
   Write_IIC_Byte(0x78);  //Slave address,SA0=0
   Write_IIC_Byte(0x00);	//write command
   Write_IIC_Byte(IIC_Command);
   IIC_Stop();
}


//开始IIC写数据 -- 这样可以让一组数据发送完成后再关闭IIC，能很大程度提速
void IIC_without_ACK::Begin_IIC_Data()
{
   IIC_Start();
   Write_IIC_Byte(0x78);
   Write_IIC_Byte(0x40);	//write data
}


//设置起始点坐标
void IIC_without_ACK::IIC_SetPos(unsigned char x, unsigned char y)
{
  IIC_Start();
  Write_IIC_Byte(0x78);  //Slave address,SA0=0
  Write_IIC_Byte(0x00);	//write command
  
  Write_IIC_Byte(0xb0+y);
  Write_IIC_Byte(((x&0xf0)>>4)|0x10);//|0x10
  Write_IIC_Byte((x&0x0f)|0x01);//|0x01
  
  IIC_Stop();//SetPos函数经常被使用,所以采用了这种发送一组命令再关闭IIC总线的方式
}


//全屏显示 -- Fill_Screen(0x00)可用作清屏
void IIC_without_ACK::Fill_Screen(unsigned char fill_Data)
{
  unsigned char m,n;
  for(m=0;m<8;m++)
  {
    Write_IIC_Command(0xb0+m);	//page0-page1
    Write_IIC_Command(0x00);		//low column start address
    Write_IIC_Command(0x10);		//high column start address
    Begin_IIC_Data();
    for(n=0;n<128;n++)
    {
      Write_IIC_Byte(fill_Data);
    }
    IIC_Stop();
  }
}


//显示6x8的ASCII字符
void IIC_without_ACK::Char_F6x8(unsigned char x, unsigned char y, const char ch[])
{
  unsigned char c,i,j=0;
  while(ch[j] != '\0')
  {
    c = ch[j] - 32;
    if(x>126)
    {
      x=0;
      y++;
    }
    IIC_SetPos(x,y);
    Begin_IIC_Data();//SetPos函数有IIC_Stop的操作
    for(i=0;i<6;i++)
    {
      Write_IIC_Byte(font6x8[c*6+i]);
    }
    IIC_Stop();
    x += 6;
    j++;
  }
}


//显示8x16 ASCII字符 -- 使用此函数时，将此处以及IIC_without_ACK.h中相应的注释部分移除。
/*
void IIC_without_ACK::Char_F8x16(unsigned char x, unsigned char y,const char ch[])
{
	unsigned char c=0,i=0,j=0;
	while (ch[j]!='\0')
	{
		c =ch[j]-32;
		if(x>120)
		{
      x=0;
      y++;
    }
		IIC_SetPos(x,y);
		Begin_IIC_Data();
		for(i=0;i<8;i++)
		{
      Write_IIC_Byte(font8X16[c*16+i]);
    }
    IIC_Stop();
		IIC_SetPos(x,y+1);
		Begin_IIC_Data();
		for(i=0;i<8;i++)
		{
      Write_IIC_Byte(font8X16[c*16+i+8]);
    }
    IIC_Stop();
		x+=8;
		j++;
	}
}*/


//显示16x16的中文 -- 使用此函数时，将此处以及IIC_without_ACK.h中相应的注释部分移除。
/*
void IIC_without_ACK::CN_F16x16(unsigned char x, unsigned char y, unsigned char N)
{
	unsigned char wm=0;
	unsigned int adder=32*N;
	IIC_SetPos(x , y);
	Begin_IIC_Data();
	for(wm = 0;wm < 16;wm++)
	{
		Write_IIC_Byte(CN16x16[adder]);
		adder += 1;
	}
	IIC_Stop();
	IIC_SetPos(x,y + 1);
	Begin_IIC_Data();
	for(wm = 0;wm < 16;wm++)
	{
		Write_IIC_Byte(CN16x16[adder]);
		adder += 1;
	}
	IIC_Stop();
}
*/


//显示128x64的BMP图片 -- 使用此函数时，将此处以及IIC_without_ACK.h中相应的注释部分移除。
void IIC_without_ACK::Draw_BMP(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1,const char BMP[])
{
  unsigned int j=0;
  unsigned char x,y;
  
  if(y1%8==0)
    y=y1/8;
  else
    y=y1/8+1;
  for(y=y0;y<y1;y++)
  {
    IIC_SetPos(x0,y);
    Begin_IIC_Data();
    for(x=x0;x<x1;x++)
    {
      Write_IIC_Byte(BMP[j++]);
    }
    IIC_Stop();
  }
}


//SSD1306初始化
void IIC_without_ACK::Initial()
{
  Write_IIC_Command(0xAE);//display off
  Write_IIC_Command(0x00);//Set Memory Addressing Mode
  Write_IIC_Command(0x10);//00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
  Write_IIC_Command(0x40);//Set Page Start Address for Page Addressing Mode,0-7
  Write_IIC_Command(0x81);//Set COM Output Scan Direction
  Write_IIC_Command(0xCF);//---set low column address
  Write_IIC_Command(0xA1);//---set high column address
  Write_IIC_Command(0xC8);//--set start line address
  Write_IIC_Command(0xA6);//--set contrast control register
  Write_IIC_Command(0xA8);
  Write_IIC_Command(0x3F);//--set segment re-map 0 to 127
  Write_IIC_Command(0xD3);//--set normal display
  Write_IIC_Command(0x00);//--set multiplex ratio(1 to 64)
  Write_IIC_Command(0xD5);//
  Write_IIC_Command(0x80);//0xa4,Output follows RAM content;0xa5,Output ignores RAM content
  Write_IIC_Command(0xD9);//-set display offset
  Write_IIC_Command(0xF1);//-not offset
  Write_IIC_Command(0xDA);//--set display clock divide ratio/oscillator frequency
  Write_IIC_Command(0x12);//--set divide ratio
  Write_IIC_Command(0xDB);//--set pre-charge period
  Write_IIC_Command(0x40);//
  Write_IIC_Command(0x20);//--set com pins hardware configuration
  Write_IIC_Command(0x02);
  Write_IIC_Command(0x8D);//--set vcomh
  Write_IIC_Command(0x14);//0x20,0.77xVcc
  Write_IIC_Command(0xA4);//--set DC-DC enable
  Write_IIC_Command(0xA6);//
  Write_IIC_Command(0xAF);//--turn on oled panel 
}