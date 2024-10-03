#include <STC8G.H>
#include "intrins.h"

#define FOSC            11059200UL
#define BRT             (65536 - FOSC / 115200 / 4)


int *BGV;                                       //�ڲ�1.19V�ο��ź�Դֵ�����idata��
                                                //idata��EFH��ַ��Ÿ��ֽ�
                                                //idata��F0H��ַ��ŵ��ֽ�
                                                //��ѹ��λΪ����(mV)
bit busy;
bit int0_isr_flag = 0;

sbit PMU_EN = P3^3;

sbit PIN_KEY = P3^2;
sbit PIN_CHARGING = P5^4;											// Charging indicator
sbit USB_SWITCH1 = P5^5;											// Unused
sbit USB_SWITCH2 = P5^6;											// Ununed

bit adc_req = 0;
bit charging_req = 0;
bit poweroff_req = 0;

#define USB_MODE_NORMAL 3
#define USB_MODE_WIFI_CAM 1
#define USB_MODE_DIRECT 2

//void setUSBMode(char dat)
//{
//	return;
	/*
	switch(dat)
	{
		case 1:
			USB_SWITCH1 = 0;
			USB_SWITCH2 = 0;
			break;
		case 2:
			USB_SWITCH1 = 0;
			USB_SWITCH2 = 1;
			break;
		case 3:
			USB_SWITCH1 = 1;
			USB_SWITCH2 = 0;
			break;
		default:
			break;
	}
	*/
//}

void UartIsr() interrupt 4
{
		char c;
    if (TI)
    {
        TI = 0;
        busy = 0;
    }
    if (RI)
    {
        RI = 0;
				c = SBUF;
				switch(c)
				{
					case 0x58:
						adc_req = 1;
						break;
					case 0x59:
						charging_req = 1;
						break;
					case 0x6a:
						poweroff_req = 1;
						break;
					case 0x11:
						//setUSBMode(USB_MODE_NORMAL);
						break;
					case 0x12:
						//setUSBMode(USB_MODE_WIFI_CAM);
						break;
					case 0x13:
						//setUSBMode(USB_MODE_DIRECT);
						break;
				}
    }
}
void INT0_Isr() interrupt 0
{
    ;
}

void UartInit()
{
    SCON = 0x50;
    TMOD = 0x00;
    TL1 = BRT;
    TH1 = BRT >> 8;
    TR1 = 1;
    AUXR = 0x40;
    busy = 0;
}

void UartSend(char dat)
{
    while (busy);
    busy = 1;
    SBUF = dat;
}

void ADCInit()
{
    P_SW2 |= 0x80;
    ADCTIM = 0x3f;                              //����ADC�ڲ�ʱ��
    P_SW2 &= 0x7f;

    ADCCFG = 0x2f;                              //����ADCʱ��Ϊϵͳʱ��/2/16
    ADC_CONTR = 0x8f;                           //ʹ��ADCģ��,��ѡ���15ͨ��
}

int ADCRead()
{
    int res;

    ADC_CONTR |= 0x40;                          //����ADת��
    _nop_();
    _nop_();
    while (!(ADC_CONTR & 0x20));                //��ѯADC��ɱ�־
    ADC_CONTR &= ~0x20;                         //����ɱ�־
    res = (ADC_RES << 8) | ADC_RESL;            //��ȡADC���

    return res;
}

void doADC()
{
	int res;
	int vcc;
	int i;

  ADCRead();
  ADCRead();                                  //ǰ�������ݽ��鶪��

    res = 0;
    for (i=0; i<8; i++)
    {
        res += ADCRead();                       //��ȡ8������
    }
    res >>= 3;                                  //ȡƽ��ֵ

    vcc = (int)(1024L * *BGV / res);            //(10λADC�㷨)����VREF�ܽŵ�ѹ,����ص�ѹ
//  vcc = (int)(4096L * *BGV / res);            //(12λADC�㷨)����VREF�ܽŵ�ѹ,����ص�ѹ
                                                //ע��,�˵�ѹ�ĵ�λΪ����(mV)
    UartSend(vcc >> 8);                         //�����ѹֵ������
    UartSend(vcc);
}
void Delay5ms(void)	//@11.0592MHz
{
	unsigned char data i, j;

	_nop_();
	_nop_();
	i = 72;
	j = 205;
	do
	{
		while (--j);
	} while (--i);
}
void Delay50ms(void)	//@11.0592MHz
{
	unsigned char data i, j, k;

	i = 3;
	j = 207;
	k = 28;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}
void Delay1000ms(void)	//@11.0592MHz
{
	unsigned char data i, j, k;

	i = 57;
	j = 27;
	k = 112;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}


void main()
{
	unsigned char keytime = 0;
		PMU_EN = 0;
		PIN_CHARGING = 1;
	  //setUSBMode(USB_MODE_NORMAL);
    P3M0 = 0x03; P3M1 = 0x03;
    P5M0 = 0x00; P5M1 = 0x00; 

    BGV = (int idata *)0xef;
		IT0 = 1;                                    //ʹ��INT0�½����ж�
    EX0 = 1;                                    //ʹ��INT0�ж�
    EA = 1;
		goto forcesleep;
    while (1)
		{
	sleep:
			poweroff_req = 0;
			Delay1000ms();
			Delay1000ms();
			Delay1000ms();
	forcesleep:
			ES = 0;
			TR1 = 0;
			ADC_CONTR = 0;
			PMU_EN = 0;;
			Delay50ms();
			if(PIN_KEY != 0)
			{
				_nop_();
				_nop_();
				PCON = 0x02;
				_nop_();
				_nop_();
			}
			// ��鰴��ʱ��
			keytime = 0;
			while(PIN_KEY == 0)
			{
				Delay5ms();
				++keytime;
				if(keytime == 180)
					break;
			}
			if(keytime != 180)
				goto forcesleep;
			PMU_EN = 1;
			//setUSBMode(USB_MODE_NORMAL);
			ADCInit();                                  //ADC��ʼ��
			UartInit();                                 //���ڳ�ʼ��
			ES = 1;
			keytime = 0;
loop:
			if(poweroff_req)
				goto sleep;
			if(adc_req)
			{
				adc_req = 0;
				doADC();
			}
			if (charging_req)
			{
				charging_req = 0;
				if (PIN_CHARGING)
				{
					UartSend(0x00);
				}
				else
				{
					UartSend(0x01);
				}
			}
			if(PIN_KEY == 0)
			{
				keytime = 0;
				while(PIN_KEY == 0)
				{
					Delay50ms();
					++keytime;
					if(poweroff_req)
						goto sleep;
					if(adc_req)
					{
						adc_req = 0;
						doADC();
					}
					if(keytime == 190)
						break;
				}
				if(keytime == 190)
					goto forcesleep;
			}
			goto loop;
		}
}

