/*********************************************************************
 * File Name          : dtu_config.h
 *
 * Description        : DTU 设备配置头 —— 所有可按需修改的项集中在这里。
 *                      地理位置、信号强度、IMEI 目前用随机数/模拟值仿真，
 *                      修改本文件即可改设备身份、联网方式与上报参数。
 *
 * 说明：
 *   - IMEI 同时用于 WebSocket URL 路径：/v1/{org}/{model}/{IMEI}，
 *     服务端据其建立设备并作为 accessKey。
 *   - 服务端只支持两款 model：yed-s710ym / yed-dg720ym。
 *   - 传输开关 DTU_TLS：1=走 wss(443)，0=走明文 ws(80) 用于联调。
 *******************************************************************************/
#ifndef __DTU_CONFIG_H__
#define __DTU_CONFIG_H__

/* ============ 1. 设备身份 ============ */
/* 设备 IMEI（模拟值，可改成真实 IMEI）。仅数字 0-9，15~17 位。 */
#define DTU_IMEI                    "869999042529385"
/* 组织标识（服务端 URL 的 orgId 段），对应服务端 organizations/{env}/yinerda */
#define DTU_ORG_ID                  "yinerda"
/* 设备型号（model 段），目前服务端只支持这两个：
 *   "yed-s710ym"  /  "yed-dg720ym"   */
#define DTU_MODEL                   "yed-s710ym"

/* ============ 2. 服务端 ============ */
#define DTU_HOST                    "ws.dtu.ap.openxiot.cn"
/* DTU_TLS=1 时用 443(wss)，=0 时用 80(ws)。 */
#define DTU_PORT                    443
/* WebSocket 路径前缀，最终拼成 /v1/{org}/{model}/{IMEI} */
#define DTU_WS_PATH_PREFIX          "/v1/"

/* ============ 3. 网络接入（上网方式） ============ */
/* 1=用 DHCP 自动获取 IP（推荐，能让设备连外网）；0=用下方静态 IP。 */
#define DTU_USE_DHCP                1
/* DTU_USE_DHCP=0 时使用的静态地址；DHCP 时也可留作回退。 */
#define DTU_IP_STATIC               {192,168,1,50}
#define DTU_IP_GW                   {192,168,1,1}
#define DTU_IP_MASK                 {255,255,255,0}
/* 静态方式下可用的 DNS 服务器；DHCP 方式会自动从路由器获取，此宏被忽略。 */
#define DTU_IP_DNS                  {223,5,5,5}
/* MAC 地址：置 0 时用芯片出厂唯一 MAC(WCHNET_GetMacAddr 读取)。 */
#define DTU_MAC                     {0,0,0,0,0,0}

/* ============ 4. 上报参数（随机数模拟） ============ */
/* 心跳二进制帧内容(服务端规定)与周期(秒)。勿改帧内容，服务端依赖它保活。 */
#define DTU_HEARTBEAT_FRAME         {0x0a,0x0b,0x0c,0x0d,0x0e,0x0f}
#define DTU_HEARTBEAT_INTERVAL_S    20

/* 信号强度 csq：取值范围与变换幅度。随机值落在 [CSQ_MIN, CSQ_MAX]。 */
#define DTU_CSQ_MIN                 19
#define DTU_CSQ_MAX                 31
#define DTU_CSQ_INTERVAL_S          30          /* 每次上报间隔(秒) */

/* 定位 coordinates=经度,纬度（服务端约定：第一个值是经度、第二个是纬度）。
 * 以 1e7 缩放的整数表示，随机在其基准附近 ±JITTER 抖动。 */
#define DTU_LON_BASE                1165583130L  /* 经度  116.5583130 */
#define DTU_LAT_BASE                397858900L   /* 纬度  039.7858900 */
#define DTU_LOC_JITTER              500000L      /* 抖动幅度(1e7 级)，约 ±0.05° */
#define DTU_LOC_INTERVAL_S          60          /* 每次上报间隔(秒) */

/* ============ 5. 断线重连（指数退避） ============ */
#define DTU_RECONNECT_MIN_MS        5000L       /* 重连最短延时 */
#define DTU_RECONNECT_MAX_MS        60000L      /* 重连最长延时(退避上限) */

/* ============ 6. 传输开关 ============ */
/* 1=启用 TLS 走 wss（需已接入 mbedTLS 源码，见 User/mbedtls_config.h 与 README 第五节）；
 * 0=明文 ws 联调（不依赖 mbedTLS，可先跑通整条 DTU 链路）。 */
#define DTU_TLS                     1

/* ============ 7. 采样周期：主循环节拍 ============ */
/* WCHNET 协议栈定时中断周期(ms)，与 eth_driver.h 一致，勿改。 */
#define DTU_TICK_PERIOD_MS          10

#endif /* __DTU_CONFIG_H__ */