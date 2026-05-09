// STC8G1K08 五引脚推挽输出 2秒同步翻转
// 翻转引脚：P1.0 / P1.1 / P1.6 / P3.3 / P5.5
// 系统时钟：11.0592MHz
// P3.2 → 约100.5kHz 方波（Timer1中断翻转，50%占空比）
// P3.4 = 固定高电平, P3.7 = 固定低电平
#include "STC8G.H"

#define P55_MASK 0x20
#define T1_RELOAD_100K 0xC9    // 1T模式: (256-0xC9)=55个时钟，半周期约4.97us

// ========== 定时器0初始化 (设置125us中断，保留备用) ==========
void Timer0_Init(void)
{
    AUXR |= 0x80;       // 定时器时钟1T模式
    TMOD &= 0xF0;       // 设置定时器为模式0(16位自动重装载)
    TL0 = 0x9A;         // 65536-1382=0xFFCA, 125us @11.0592MHz
    TH0 = 0xFA;
    TF0 = 0;
    TR0 = 1;
    ET0 = 1;
    EA = 1;
}

// ========== 软件延时函数 (约10ms @ 11.0592MHz) ==========
void Delay10ms(void)
{
    unsigned int i, j;
    for(i = 0; i < 100; i++)
        for(j = 0; j < 110; j++);
}

// ========== Timer1 初始化 (P3.2 输出约100.5kHz) ==========
void Timer1_P32_100k_Init(void)
{
    AUXR |= 0x40;       // Timer1 1T模式
    TMOD &= 0x0F;       // 清T1模式位
    TMOD |= 0x20;       // T1模式2: 8位自动重装
    TH1 = T1_RELOAD_100K;
    TL1 = T1_RELOAD_100K;
    TF1 = 0;
    ET1 = 1;
    EA = 1;
    TR1 = 1;
}

#ifdef __C51__
void Timer1_ISR(void) interrupt 3
#else
void Timer1_ISR(void)
#endif
{
    P32 = !P32;
}

// ========== PCA PWM 配置 (P3.5 输出准800Hz，保留备用) ==========
void PWM_Init(void)
{
    AUXR |= 0x80;
    TMOD &= 0xF0;
    TL0 = 0xCA;         // 65536-54=0xFFCA
    TH0 = 0xFF;
    TR0 = 1;

    P_SW1 = (P_SW1 & ~0x30) | 0x10;

    CCON = 0;
    CMOD = 0x04;        // PCA时钟 = 定时器0溢出
    CL = 0; 
    CH = 0;
    
    CCAPM0 = 0x42;
    PCA_PWM0 = 0x00;
    CCAP0H = 128;
    CCAP0L = 128;
    
    CR = 1;
}

void main(void)
{
    unsigned int cnt20ms = 0;

    // ========== 所有I/O初始化（先于PCA，避免干扰）==========
    // P1.0/P1.1/P1.6：推挽输出，初始低
    P1M0 |= 0x01 | 0x02 | 0x40;
    P1M1 &= ~(0x01 | 0x02 | 0x40);
    P10 = 0;
    P11 = 0;
    P16 = 0;

    // P3.3：推挽输出，初始低（翻转引脚）
    P3M0 |= 0x08;
    P3M1 &= ~0x08;
    P33 = 0;

    // P5.5：推挽输出，初始低 —— 关键修正：用位操作代替整字节操作
    P5M0 |= P55_MASK;
    P5M1 &= ~P55_MASK;
    P55 = 0;            // 直接用位操作，绝不碰 P5 其他位

    // P3.5：推挽输出（蜂鸣器，本例未用，注释保留）
   // P3M0 |= 0x20;
   // P3M1 &= ~0x20;

    // P3.2：推挽输出（100kHz输出）
    P3M0 |= 0x04;
    P3M1 &= ~0x04;
    P32 = 0;

    // P3.4/P3.7：推挽输出，固定电平
    P3M0 |= 0x10 | 0x80;
    P3M1 &= ~(0x10 | 0x80);
    P34 = 1;            // 固定高，后续不再改变
    P37 = 0;            // 固定低

    // 初始化Timer1输出（在引脚模式设置完成后调用）
    Timer1_P32_100k_Init();

    // 以下函数保留，未调用
    // PWM_Init();
    // Timer0_Init();

    // ========== 主循环：5个引脚每2秒同步翻转 ==========
    while(1)
    {
        // P3.4/P3.7 的电平已在初始化时设定，这里无需再写
        // 若担心后续被意外更改，可保留下面两句（位操作，安全）
        P34 = 1;
        P37 = 0;

        Delay10ms();        // 约10ms
         P34 = 0;
        P37 = 1;
			 Delay10ms();
        cnt20ms++;
        if(cnt20ms >= 200)  // 200 * 10ms = 2秒
        {
            P1 ^= 0x43;      // 翻转 P1.0, P1.1, P1.6
            P33 = ~P33;      // 翻转 P3.3（位操作，不影响P3.2）
            P55 = ~P55;      // 翻转 P5.5（位操作）
            cnt20ms = 0;
        }
    }
}