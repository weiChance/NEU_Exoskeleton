
/******************** 4（多）路ADC定时串口发送 ********************
 * 文件名  ：main.c
 * 描述    ：通过串口调试软件，向板子发送数据，板子接收到数据后，立即回传给电脑。         
 * 实验平台：MINI STM32开发板 基于STM32F103VET6
 * 库版本  ：ST3.0.0  			
 * 修改时间：2019.08.18	 
*********************************************************/
#include "stm32f10x.h"
#include "usart1.h"
#include "stm32_adc.h"
#include "stm32_dac.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_rcc.h"
#include "stm32_tim2.h"
#include "stm32_led.h"

int  ADC_Value[4];//用来保存经过转换得到的电压值
int Manu_value[8]={0};
//float adc_average[6] = {0};
//int  ADC_Value[6];//用来保存经过转换得到的电压值
float adc_average[4] = {0};
extern u16  AD_Value[20][4];


int force_Value[4] = {0};//存储力







void ADC_average(void);
void ADC_ConvertSend(int* value);
void LED_GPIO_Config(void);
void TIM2_Config(u16 arr,u16 psc);

int main(void)
{ 	
	
	SystemInit();	//配置系统时钟为 72M  
	USART1_Config(); //USART1 配置 	
	ADC1_Init();
	LED_GPIO_Config();/* led 端口配置 */ 
	
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);//开始采集
	
	TIM2_NVIC_Configuration(); /* TIM2 定时配置 */
	TIM2_Config(719,999);//定时器中断时间配置 5ms     (3599+1)*(9999+1)/72000000=1s
  while (1)
  {	 
		//printf("hello the world! \r\n");
			ADC_average();
			ADC_Value[0]=0;//求平均值并转换成电压值
			ADC_Value[1]=(float)adc_average[1]*3300/4096;//求平均值并转换成电压值
			ADC_Value[2]=(float)adc_average[2]*3300/4096;//求平均值并转换成电压值
		  ADC_Value[3]=(float)adc_average[3]*3300/4096;
		
		  force_Value[0]=65534;
		  force_Value[1]=24*(ADC_Value[1]*1000/(3300-ADC_Value[1]));
		  force_Value[2]=24*(ADC_Value[2]*1000/(3300-ADC_Value[2]));
		  force_Value[3]=24*(ADC_Value[3]*1000/(3300-ADC_Value[3]));
		
		
		
		
		
			//ADC_Value[3]=(float)adc_average[3]*3300/4096;//求平均值并转换成电压值
//			ADC_Value[4]=(float)adc_average[4]*3300/4096;//求平均值并转换成电压值
//			ADC_Value[5]=(float)adc_average[5]*3300/4096;//求平均值并转换成电压值
	
		//printf("AD value = %d  %d  %d  %d   %d  %d  mV  \r\n",ADC_Value[0],ADC_Value[1],ADC_Value[2],ADC_Value[3],ADC_Value[4],ADC_Value[5]);
		//printf("AD value = %d  %d  %d  %d  mV  \r\n",ADC_Value[0],ADC_Value[1],ADC_Value[2],ADC_Value[3]);
  }
	
}

void ADC_average(void)
{
		int sum = 0;
		u8 i,j;
		for(i=0;i<4;i++)
	{
		for(j=0;j<10;j++)
		{
			sum+=AD_Value[j][i];
		}			
		adc_average[i] = sum/10;
			sum = 0;
	}
}

 void ADC_ConvertSend(int * value)//发送数据包+ 2个包头
 {
	  unsigned int temp[8];
	  unsigned short i=0,j=0;  

	 
		for(i=0;i<8;i++)  
		{  
			temp[i] = value[i];//float转BYTE
		}  
		
		for( j=0;j<8;j++)
	  {
			UART1_Send_byte(USART1,temp[j]);
	  }
 }
void TIM2_IRQHandler(void)
{
    //判断TIM3更新中断是否发生
    if(TIM_GetITStatus(TIM2,TIM_IT_Update)!=RESET)
    {
        //必须清楚标志位
			TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
			//状态取反
    //  GPIO_WriteBit(GPIOB,GPIO_Pin_13,(BitAction)(!GPIO_ReadOutputDataBit(GPIOB,GPIO_Pin_13)));
		//	GPIO_WriteBit(GPIOB,GPIO_Pin_12,(BitAction)(!GPIO_ReadOutputDataBit(GPIOB,GPIO_Pin_12)));//c8t6灯
		//	GPIO_WriteBit(GPIOB,GPIO_Pin_14,(BitAction)(!GPIO_ReadOutputDataBit(GPIOB,GPIO_Pin_14)));
			
			Manu_value[0]=force_Value[0]%256;// Yu
			Manu_value[1]=force_Value[0]/256;// zheng
			Manu_value[2]=force_Value[1]%256;
			Manu_value[3]=force_Value[1]/256;
			Manu_value[4]=force_Value[2]%256;
			Manu_value[5]=force_Value[2]/256;
			Manu_value[6]=force_Value[3]%256;
			Manu_value[7]=force_Value[3]/256;
//			adc =(Manu_value[3] * 256 + Manu_value[2]);
//			printf("AD value = %d  %d   %d    %d\r\n",adc,ADC_Value[1],Manu_value[2] ,Manu_value[3] );
			ADC_ConvertSend(Manu_value);//串口发送
			//printf("AD value = %d  %d  %d mV  \r\n",ADC_Value[0],ADC_Value[1],ADC_Value[2]);
    }

}



