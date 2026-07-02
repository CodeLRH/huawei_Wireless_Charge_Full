/**
  * @file    qi.c
  * @brief   Qi无线充电协议 - 简化版
  * @note    主函数只需调用 Qi_Process()，内部自动完成:
  *          1. 低功率Ping探测设备
  *          2. 识别设备(功率>50mW)
  *          3. 扫描最佳占空比(20%-80%)
  *          4. 功率闭环调节(每100ms)
  */
#include "qi.h"
#include "adc.h"
#include <stdio.h>

/* 内部变量 */
static Qi_State_t qi_state = QI_IDLE;
static uint8_t    qi_bestDuty = 50;
static uint8_t    qi_curDuty = 20;
static uint32_t   qi_lastTick = 0;
static uint32_t   qi_stageTick = 0;

/* 引用main.c的全局变量 */
extern volatile float    g_batVoltage;
extern volatile float    g_chargeCurrent;
extern volatile uint32_t g_outputPower_mW;
extern volatile uint32_t g_sysTick_ms;

/* 引用main.c的函数 */
extern void PWM_SetDuty(uint8_t percent);
extern void ADC_SampleAll(void);

/* ========== 状态查询 ========== */

Qi_State_t Qi_GetState(void) { return qi_state; }
uint8_t    Qi_GetBestDuty(void) { return qi_bestDuty; }

/* ========== 内部函数 ========== */

/* 采样并计算输出功率 */
static uint32_t samplePower(void) {
    ADC_SampleAll();
    return (uint32_t)(g_batVoltage * g_chargeCurrent);
}

/* 从20%到80%扫描，找最大功率点 */
static uint8_t findBestDuty(void) {
    uint8_t best = 20;
    uint32_t maxP = 0;

    for (uint8_t d = 20; d <= 80; d += 10) {
        PWM_SetDuty(d);
        HAL_Delay(300);
        uint32_t p = samplePower();
        printf("[QI] sweep: duty=%d%% P=%lumW\r\n", d, p);
        if (p > maxP) { maxP = p; best = d; }
    }
    printf("[QI] best duty=%d%% P=%lumW\r\n", best, maxP);
    return best;
}

/* 功率闭环: 目标功率vs实际功率→调占空比 */
static void powerLoop(void) {
    uint32_t target = (uint32_t)(g_batVoltage * 1000.0f);  // 目标1A
    uint32_t actual = g_outputPower_mW;

    if (actual < target * 90 / 100 && qi_curDuty < 90) {
        qi_curDuty += 2;
        PWM_SetDuty(qi_curDuty);
    } else if (actual > target * 110 / 100 && qi_curDuty > 10) {
        qi_curDuty -= 2;
        PWM_SetDuty(qi_curDuty);
    }
}

/* ========== 主函数调用这两个即可 ========== */

/**
  * @brief  启动Qi握手 - 检测到设备后调用一次
  */
void Qi_Start(void) {
    qi_state = QI_PING;
    qi_curDuty = CHARGE_POWER_START;
    printf("[QI] Start\r\n");
}

/**
  * @brief  Qi协议处理 - 充电循环中每10ms调用
  * @note   内部自动管理状态:
  *   QI_IDLE    → 等待(不做事)
  *   QI_PING    → 低功率探测设备
  *   QI_IDENTIFY→ 确认设备+扫描最佳占空比
  *   QI_TRANSFER→ 功率闭环调节(每100ms)
  *   QI_DONE    → 完成(不做事)
  */
void Qi_Process(void) {
    switch (qi_state) {

    case QI_IDLE:
        /* 空闲，等待外部启动 */
        break;

    case QI_PING:
        /* 低功率探测 */
        printf("[QI] Ping...\r\n");
        PWM_SetDuty(10);
        qi_stageTick = g_sysTick_ms;
        qi_state = QI_IDENTIFY;
        break;

    case QI_IDENTIFY:
        /* 等500ms让功率稳定，然后判断有没有设备 */
        if ((g_sysTick_ms - qi_stageTick) < 500) break;

        samplePower();
        printf("[QI] Identify: P=%lumW\r\n", g_outputPower_mW);

        if (g_outputPower_mW > 50) {
            /* 有设备 → 扫描最佳占空比 */
            printf("[QI] Device found, scanning...\r\n");
            qi_bestDuty = findBestDuty();
            qi_curDuty = qi_bestDuty;
            PWM_SetDuty(qi_curDuty);
            qi_state = QI_TRANSFER;
            qi_lastTick = g_sysTick_ms;
            printf("[QI] Transfer start, duty=%d%%\r\n", qi_curDuty);
        } else {
            /* 没设备 → 回空闲 */
            printf("[QI] No device\r\n");
            qi_state = QI_IDLE;
        }
        break;

    case QI_TRANSFER:
        /* 功率闭环: 每100ms调节一次 */
        if ((g_sysTick_ms - qi_lastTick) < 100) break;
        qi_lastTick = g_sysTick_ms;

        samplePower();
        powerLoop();
        break;

    case QI_DONE:
        /* 完成 */
        PWM_SetDuty(0);
        break;
    }
}
