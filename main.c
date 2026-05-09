#include <STC8G.H>

// STC8G1K08-38I-SOP16 物理引脚 1, 2, 3 映射
// 物理引脚 1 -> P1.0 (LED)
// 物理引脚 2 -> P1.1 (LED2)
// 物理引脚 3 -> P1.6 (LED1)

sbit PIN1_LED  = P1^0;
sbit PIN2_LED2 = P1^1;
sbit PIN3_LED1 = P1^6;

void main() {
    // 1. 显式配置 P1.0, P1.1, P1.6 为准双向口 (PxM1=0, PxM0=0)
    P1M1 &= ~0x43; 
    P1M0 &= ~0x43; 
    
    // 2. 将引脚 1, 2, 3 置低电平 (点亮/关闭 LED)
    PIN1_LED  = 0;
    PIN2_LED2 = 0;
    PIN3_LED1 = 0;

    while(1) {
        // 你的其他代码
    }
}



