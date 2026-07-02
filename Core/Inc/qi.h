/**
  * @file    qi.h
  * @brief   Qi无线充电协议 - 简化版(主函数只调一个函数)
  * @note    用法: 在main.c中 #include "qi.h"，然后调用 Qi_Process()
  */
#ifndef __QI_H__
#define __QI_H__

#include "main.h"

/* Qi握手状态 */
typedef enum {
    QI_IDLE = 0,       // 空闲
    QI_PING,           // 探测设备
    QI_IDENTIFY,       // 识别设备
    QI_TRANSFER,       // 功率传输中
    QI_DONE            // 完成
} Qi_State_t;

/* ====== 主函数调这两个 ====== */
void Qi_Start(void);      // 启动握手(检测到设备后调用一次)
void Qi_Process(void);    // 协议处理(充电中每10ms调用)

/* 状态查询 */
Qi_State_t Qi_GetState(void);
uint8_t    Qi_GetBestDuty(void);

#endif
