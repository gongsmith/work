#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <stdio.h>
#include <math.h>
#include "PAJ7620U2.h"
int fd;

#define UD_PIN     23
#define LR_PIN     1
#define led_u		3
#define led_d		2
#define led_r		4
#define led_l		5

int UD_pwm = 400;
int LR_pwm = 400;
/******************* PAJ7620U2 Driver Interface *****************************/
char I2C_readByte(int reg)
{
	return wiringPiI2CReadReg8(fd, reg);
}

unsigned short I2C_readU16(int reg)
{
	return wiringPiI2CReadReg16(fd, reg);
}
void I2C_writeByte(int reg, int val)
{
	wiringPiI2CWriteReg8(fd, reg, val);
}
unsigned char PAJ7620U2_init()
{
	unsigned char i,State;
	fd=wiringPiI2CSetup(PAJ7620U2_I2C_ADDRESS);
	delay(5);
	State = I2C_readByte(0x00);												//Read State
	if (State != 0x20) return 0;											//Wake up failed
	I2C_writeByte(PAJ_BANK_SELECT, 0);										//Select Bank 0
	for (i=0;i< Init_Array;i++)
	{
		I2C_writeByte(Init_Register_Array[i][0], Init_Register_Array[i][1]);//Power up initialize
	}
	return 1;
}

void pwm_init()
{
	
	pinMode(LR_PIN, PWM_OUTPUT);    // 设置PWM输出
    pwmSetMode(PWM_MODE_MS);        // 设置传统模式
    pwmSetClock(192);                // 设置分频
    pwmSetRange(2000);                // 设置周期分为2000步
	pinMode(UD_PIN, PWM_OUTPUT);    // 设置PWM输出
    pwmSetMode(PWM_MODE_MS);        // 设置传统模式
    pwmSetClock(192);                // 设置分频
    pwmSetRange(2000);                // 设置周期分为2000步
    
    pwmWrite(LR_PIN, LR_pwm);
    pwmWrite(UD_PIN, UD_pwm);
}

void led_init()
{
	pinMode(led_d, OUTPUT);
	pinMode(led_u, OUTPUT);
	pinMode(led_l, OUTPUT);
	pinMode(led_r, OUTPUT);
	digitalWrite(led_d, LOW);
	digitalWrite(led_u, LOW);
	digitalWrite(led_l, LOW);
	digitalWrite(led_r, LOW);
}

int main(int argc, char** argv)
{
	unsigned char i;
	unsigned short  Gesture_Data;
	static int T_count = 0;
	system("./camera.sh &");
	
	printf("\nGesture Sensor Test Program ...\n");
	if (wiringPiSetup() < 0) return 1;
	delay(5);
	if(!PAJ7620U2_init())
	{	printf("\nGesture Sensor Error\n");
		return 0;
	}
	printf("\nGesture Sensor OK\n");
	
	pwm_init();
	led_init();
	printf("pwm/LED init\n");
	
	I2C_writeByte(PAJ_BANK_SELECT, 0);//Select Bank 0
	for (i = 0; i < Gesture_Array_SIZE; i++)
	{
		I2C_writeByte(Init_Gesture_Array[i][0], Init_Gesture_Array[i][1]);//Gesture register initializes
	}
	while (1)
	{
		Gesture_Data = I2C_readU16(PAJ_INT_FLAG1);
		
		if (Gesture_Data)
		{
			switch (Gesture_Data)
			{
				case PAJ_UP:	T_count=1;UD_pwm+=40;digitalWrite(led_u, HIGH);  printf("Up%d\r\n",UD_pwm);		break;
				case PAJ_DOWN:	T_count=1;UD_pwm-=40;digitalWrite(led_d, HIGH);	printf("Down%d\r\n",UD_pwm);	break;
				case PAJ_LEFT:	T_count=1;LR_pwm+=40;digitalWrite(led_l, HIGH);	printf("Left%d\r\n",LR_pwm);	break;
				case PAJ_RIGHT:	T_count=1;LR_pwm-=40;digitalWrite(led_r, HIGH);	printf("Right%d\r\n",LR_pwm); 	break;
				case PAJ_FORWARD:			printf("Forward\r\n");			break;
				case PAJ_BACKWARD:			printf("Backward\r\n"); 		break;
				case PAJ_CLOCKWISE:			printf("Clockwise\r\n"); 		break;
				case PAJ_COUNT_CLOCKWISE:	printf("AntiClockwise\r\n"); 	break;
				case PAJ_WAVE:				printf("Wave\r\n"); 			break;
				default: break;
			}
			Gesture_Data=0;
			delay(50);
		}
		
		if(T_count)  //灯亮1秒就灭
		{
			T_count++;
			if(T_count > 1000)
			{
					digitalWrite(led_d, LOW);
					digitalWrite(led_u, LOW);
					digitalWrite(led_l, LOW);
					digitalWrite(led_r, LOW);
					T_count = 0;
			}
		}
		
		if(UD_pwm > 680) UD_pwm = 680;
		if(UD_pwm < 160) UD_pwm = 160;
		if(LR_pwm > 720) LR_pwm = 720;
		if(LR_pwm < 120) LR_pwm = 120;
		pwmWrite(LR_PIN, LR_pwm);
		pwmWrite(UD_PIN, UD_pwm);
	}
	return 0;
}
