#include "bsp_adc.h"

// #define ADC_DMA

#define ChCount 1      // 采样通道数
#define SampleCount 16 // 采样次数

extern void bsp_delay(int n);

#ifdef ADC_DMA
#define ContinuousConv ENABLE
#define ADC1_DMA_Ch DMA1_Channel1

/**
 * @brief       获取指定通道采样平均值
 * @param ch    指定通道
 * @return u16  采样平均值
 */
u16 adc_Value(u8 ch)
{
    u16 sum, i, max, min;
    sum = i = 0;
    max = min = adcValues[ch][0];
    for (; i < SampleCount; i++)
    {
        sum += adcValues[ch][i];
        if (max < adcValues[ch][i])
            max = adcValues[ch][i];
        else if (min > adcValues[ch][i])
            min = adcValues[ch][i];
    }
    return (sum - max - min) / (SampleCount - 2);
}

void adc_ConfigDMA(void)
{
    DMA_InitTypeDef DMA_InitStructure;
    /* DMA clock enable */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&ADC1->DR;                  // DMA外设ADC基地址
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; // 数据宽度为16位
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;            // 外设地址寄存器不变-1个外设
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;         // 数据宽度为16位
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                     // 内存地址寄存器递增
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                             // 工作在循环缓存模式
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                                // DMA通道x没有设置为内存到内存传输
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;                         // DMA通道x拥有高优先级

    /* DMA RX CHANNEL Config */
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;      // 外设->DMA缓存
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&adcValues; // DMA缓存
    DMA_InitStructure.DMA_BufferSize = sizeof(adcValues);   // DMA通道的DMA缓存的大小
    DMA_DeInit(ADC1_DMA_Ch);                                // 初始化默认值
    DMA_Init(ADC1_DMA_Ch, &DMA_InitStructure);              // 根据DMA_InitStruct中指定的参数初始化DMA的通道
    DMA_Cmd(DMA1_Channel1, ENABLE);                         // 启动DMA通道
    ADC_DMACmd(ADC1, ENABLE);                               // 使能ADC1 DMA功能
}
#else
#define ContinuousConv DISABLE
/**
 * @brief       获取指定通道采样平均值
 * @param ch    指定通道
 * @return u16  采样平均值
 */
u16 adc_Value(u8 ch)
{
    u16 sum, i, max, min;
    u16 adcValues[ChCount][SampleCount] = {0}; // 用来存放ADC转换结果，也是DMA的目标地址
    sum = i = 0;

    for (; i < SampleCount; i++)
    {
        ADC_SoftwareStartConvCmd(ADC1, ENABLE); // 使能指定的ADC1的软件转换启动功能
        while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC))
            ;                                            // 等待转换结束
        adcValues[ch][i] = ADC_GetConversionValue(ADC1); // 返回最近一次ADC1规则组的转换结果
        sum += adcValues[ch][i];
        if (i == 0)
            max = min = adcValues[ch][0];
        else
        {
            if (max < adcValues[ch][i])
                max = adcValues[ch][i];
            else if (min > adcValues[ch][i])
                min = adcValues[ch][i];
        }
        bsp_delay(100);
    }
    return (sum - max - min) / (SampleCount - 2); // 第一次算出来的值有误差
//    return sum / SampleCount;
}
#endif
void adc_ConfigIO(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    /* 配置PA1为模拟输入 PA1 */
    GPIO_InitStructure.GPIO_Pin = ADC_PIN1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(ADC_PORT, &GPIO_InitStructure);
}
void adc_ConfigADC(void)
{
    ADC_InitTypeDef ADC_InitStructure;
    /* 使能 ADC1 clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div6); // 72M/8=9,ADC最大时间不能超过14M

    /* 配置ADC1, 不用DMA, 用软件触发 */
    ADC_DeInit(ADC1);                                                   // 将外设 ADC1 的全部寄存器重设为缺省值
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;                  // ADC1和ADC2工作在独立模式
    ADC_InitStructure.ADC_ScanConvMode = (ChCount > 1);                 // 模数转换工作在扫描模式
    ADC_InitStructure.ADC_ContinuousConvMode = ContinuousConv;          // 模数转换工作在 ENABLE 连续转换模式， DISABLE 单词转换模式
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; // 外部触发转换关闭
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;              // ADC数据右对齐
    ADC_InitStructure.ADC_NbrOfChannel = ChCount;                       // 顺序进行规则转换的ADC通道的数目
    ADC_Init(ADC1, &ADC_InitStructure);

    /* 配置ADC1 规则通道1 channel1 configuration */
    ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_239Cycles5);
    /* 使能 ADC1 */
    ADC_Cmd(ADC1, ENABLE);
	    /* 使能ADC1 复位校准寄存器 */
    ADC_ResetCalibration(ADC1);
    /* 检查ADC1的复位寄存器 */
    while (ADC_GetResetCalibrationStatus(ADC1))
        ;
    /* 启动ADC1校准 */
    ADC_StartCalibration(ADC1);
    /* 检查校准是否结束 */
    while (ADC_GetCalibrationStatus(ADC1))
        ;
}
void adc_Config(void)
{
    adc_ConfigIO();
    adc_ConfigADC();
#ifdef ADC_DMA
    ADC_SoftwareStartConvCmd(ADC1, ENABLE); // 软件启动ADC转换
    adc_ConfigDMA();
#endif
}
