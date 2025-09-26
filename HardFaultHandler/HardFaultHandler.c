#include "cmsis_armclang.h"

// SCB (System Control Block) 寄存器定义
#define SCB_CFSR   (*((volatile uint32_t *)0xE000ED28)) // Configurable Fault Status Register
#define SCB_HFSR   (*((volatile uint32_t *)0xE000ED2C)) // HardFault Status Register
#define SCB_MMAR   (*((volatile uint32_t *)0xE000ED34)) // MemManage Address Register
#define SCB_BFAR   (*((volatile uint32_t *)0xE000ED38)) // Bus Fault Address Register

/**
 * @brief HardFault 处理程序的 C 语言部分
 * @param stack_frame 指向异常发生时堆栈的指针
 * 
 * 该函数负责分析硬错误的原因并输出调试信息
 */
void HardFault_Handler_C(uint32_t *stack_frame) {
    // 从堆栈帧中恢复寄存器值
    uint32_t stacked_r0  = stack_frame[0];  // R0 寄存器
    uint32_t stacked_r1  = stack_frame[1];  // R1 寄存器
    uint32_t stacked_r2  = stack_frame[2];  // R2 寄存器
    uint32_t stacked_r3  = stack_frame[3];  // R3 寄存器
    uint32_t stacked_r12 = stack_frame[4];  // R12 寄存器
    uint32_t stacked_lr  = stack_frame[5];  // LR (链接寄存器)，包含 EXC_RETURN 值
    uint32_t stacked_pc  = stack_frame[6];  // PC (程序计数器)，指向发生错误的指令
    uint32_t stacked_psr = stack_frame[7];  // PSR (程序状态寄存器)

    // 读取系统控制块中的故障状态寄存器
    uint32_t cfsr = SCB_CFSR;   // 可配置故障状态寄存器
    uint32_t hfsr = SCB_HFSR;   // 硬错误状态寄存器
    uint32_t mmfar = SCB_MMAR;  // 内存管理故障地址寄存器
    uint32_t bfar = SCB_BFAR;   // 总线故障地址寄存器

    // 输出寄存器状态信息
    printf("\n\n[HardFault Handler - 硬错误处理程序]\n");
    printf("R0  = 0x%08lX\n", stacked_r0);
    printf("R1  = 0x%08lX\n", stacked_r1);
    printf("R2  = 0x%08lX\n", stacked_r2);
    printf("R3  = 0x%08lX\n", stacked_r3);
    printf("R12 = 0x%08lX\n", stacked_r12);
    printf("LR  = 0x%08lX (EXC_RETURN)\n", stacked_lr);
    printf("PC  = 0x%08lX (错误发生地址)\n", stacked_pc);
    printf("PSR = 0x%08lX\n", stacked_psr);

    // 输出故障状态寄存器值
    printf("\n[Fault Status Registers - 故障状态寄存器]\n");
    printf("CFSR = 0x%08lX (可配置故障状态)\n", cfsr);
    printf("HFSR = 0x%08lX (硬错误状态)\n", hfsr);
    printf("MMAR = 0x%08lX (内存管理故障地址)\n", mmfar);
    printf("BFAR = 0x%08lX (总线故障地址)\n", bfar);

    // 分析 CFSR 寄存器中的具体错误类型
    printf("\n[Fault Analysis - 错误分析]\n");
    
    // 内存管理故障 (MemManage Fault)
    if (cfsr & (1 << 0))  printf(" - MemManage: Instruction Access Violation (指令访问违规)\n");
    if (cfsr & (1 << 1))  printf(" - MemManage: Data Access Violation (数据访问违规)\n");
    if (cfsr & (1 << 3))  printf(" - MemManage: Unstacking Fault (异常返回时出栈错误)\n");
    if (cfsr & (1 << 4))  printf(" - MemManage: Stacking Fault (异常进入时压栈错误)\n");
    if (cfsr & (1 << 7))  printf(" - MemManage: MMAR Valid (MMAR 包含有效地址)\n");
    
    // 总线故障 (Bus Fault)
    if (cfsr & (1 << 8))  printf(" - BusFault: Instruction Bus Error (指令总线错误)\n");
    if (cfsr & (1 << 9))  printf(" - BusFault: Precise Data Bus Error (精确数据总线错误)\n");
    if (cfsr & (1 << 10)) printf(" - BusFault: Imprecise Data Bus Error (不精确数据总线错误)\n");
    if (cfsr & (1 << 13)) printf(" - BusFault: Unstacking Fault (异常返回时出栈错误)\n");
    if (cfsr & (1 << 14)) printf(" - BusFault: Stacking Fault (异常进入时压栈错误)\n");
    if (cfsr & (1 << 15)) printf(" - BusFault: BFAR Valid (BFAR 包含有效地址)\n");
    
    // 使用故障 (Usage Fault)
    if (cfsr & (1 << 16)) printf(" - UsageFault: Undefined Instruction (未定义指令)\n");
    if (cfsr & (1 << 17)) printf(" - UsageFault: Invalid State (无效状态)\n");
    if (cfsr & (1 << 18)) printf(" - UsageFault: Invalid PC Load (无效的PC加载)\n");
    if (cfsr & (1 << 19)) printf(" - UsageFault: No Coprocessor (无协处理器)\n");
    if (cfsr & (1 << 24)) printf(" - UsageFault: Divide By Zero (除零错误)\n");
    if (cfsr & (1 << 25)) printf(" - UsageFault: Unaligned Access (非对齐访问)\n");

    // 分析 HFSR 寄存器
    if (hfsr & (1 << 1))  printf(" - HardFault: Vector Table Read Fault (向量表读取错误)\n");
    if (hfsr & (1 << 30)) printf(" - HardFault: Escalated from another fault (由其他故障升级而来)\n");
    if (hfsr & (1 << 31)) printf(" - HardFault: Debug Event (调试事件引起)\n");

    // 如果有有效的故障地址，输出详细信息
    if (cfsr & (1 << 7)) {  // MMARVALID
        printf(" - Memory Fault Address: 0x%08lX\n", mmfar);
    }
    if (cfsr & (1 << 15)) { // BFARVALID
        printf(" - Bus Fault Address: 0x%08lX\n", bfar);
    }

    // 死循环，等待调试器介入或系统复位
    while (1) {
        // 可以在这里添加看门狗复位或系统重启逻辑
        // 例如：NVIC_SystemReset();
    }
}

/**
 * @brief 硬错误处理程序的汇编部分
 * 
 * 此函数使用 naked 属性，不包含标准的函数序言和结语
 * 负责判断异常发生时使用的是 MSP 还是 PSP，然后跳转到 C 处理程序
 */
__attribute__((naked)) void HardFault_Handler(void) {
    __asm volatile ("mov r0, lr\n");              // 将 LR(EXC_RETURN) 保存到 R0
    __asm volatile ("movs r1, #4\n");           // 将立即数 0x4 加载到 R1
    __asm volatile ("tst r0, r1\n");           // 测试 LR 的 bit2 (判断使用 MSP 还是 PSP)
    __asm volatile ("beq uses_msp\n");           // 如果 LR & 4 == 0，说明使用 MSP，跳转到 uses_msp
    __asm volatile ("mrs r0, psp\n");            // 否则使用 PSP: 将进程堆栈指针(PSP)读取到 R0
    __asm volatile ("b hardfault_continue\n");   // 跳转到公共继续点
        
    __asm volatile ("uses_msp:\n");             // 使用主堆栈指针的标签
    __asm volatile ("mrs r0, msp\n");            // 将主堆栈指针(MSP)读取到 R0
        
    __asm volatile ("hardfault_continue:\n");    // 公共继续点
    __asm volatile ("b HardFault_Handler_C\n");  // 跳转到 C 语言处理程序
}
