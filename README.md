# dtu-ethernet-ble

基于 **CH32V208GBU6** 的 DTU 固件：设备上电后经**以太网**以 **WebSocket (wss)** 连接云平台接入服务，将网口数据与设备（本工程先做 `yed-s710ym` 机型仿真）互相转发。

- MCU：CH32V208GBU6（RISC-V，160K Flash / 32K SRAM）
- 网络：板载 10/100M 以太网（以太网要求系统时钟 60/120MHz，本工程用 120MHz）
- 外设：485 收发器（PA2/PA3/PA4 接 SP485）、BLE（CH32V208 自带）、UART1(PA9) 调试口
- 协议：wss 连接 `ws.dtu.ap.openxiot.cn/v1/yinerda/yed-s710ym/{IMEI}`

---

## 一、功能

| 项目 | 说明 |
|---|---|
| 自动上网 | 上电即试连；默认 DHCP 获取 IP，可用静态 IP（`dtu_config.h` 切换） |
| 域名解析 | 内置 DNS 解析 `ws.dtu.ap.openxiot.cn` |
| wss 连接 | WebSocket 客户端，服务端地址/端口/路径可配 |
| 心跳保活 | TCP 层 keep-alive + 应用层每 20s 心跳帧 `0a 0b 0c 0d 0e 0f` |
| 数据上报 | 周期上报信号强度 `csq=`、定位 `coordinates=经度,纬度` |
| 指令应答 | 收到服务端查询帧 `01 02 03 04` → 回 `55 aa 55 aa` |
| 断线重连 | TCP/链接断开后指数退避自动重连 |
| 仿真 | 地理位置、信号强度、IMEI 先用随机数/模拟值，集中在配置头便于修改 |

---

## 二、目录结构

```
User/                    应用层（可改）
  main.c                 入口：时钟、TIM2/ETH、主循环
  ch32v20x_it.c          TIM2/ETH 中断（WCHNET 节拍 + 毫秒计数）
  dtu_config.h           ★ 所有可配参数（IMEI/服务端/上报/重连/TLS 开关）
  dtu_app.c/h            DTU 应用状态机 + 心跳/上报/应答
  transport_eth.c/h      以太网传输层（DHCP/DNS/TCP/重连，WCHNET 集成）
  wss_tls.c/h            TLS 适配层（wss 唯一需接入的外部组件）
  ws_client.c/h          WebSocket 客户端（握手/分帧/掩码/ping-pong/close）
  ws_security.c/h        SHA-1 + base64（握手 Sec-WebSocket-Accept 校验）
Net/                     以太网协议栈（已拷入 WCH 官方 NetLib）
  NetLib/                wchnet.h / libwchnet.a / eth_driver.c / eth_driver.h
  net_config.h           协议栈配置（TCP 数量、MSS、缓冲等）
Peripheral/ Startup/ Core/ Ld/   CH32V20x 标准库 & 启动/链接脚本
doc/说明.md              板卡说明、WCH-LINK 接线
```

---

## 三、主要配置（`User/dtu_config.h`）

按需修改，无需改其它代码：

```c
#define DTU_IMEI            "869999042529385"   // 设备 IMEI（URL 路径用）
#define DTU_MODEL           "yed-s710ym"        // 机型：yed-s710ym / yed-dg720ym
#define DTU_HOST            "ws.dtu.ap.openxiot.cn"
#define DTU_PORT            443                 // 443=wss, 80=ws
#define DTU_USE_DHCP        1                   // 1=DHCP, 0=静态 IP
#define DTU_TLS             1                   // 1=wss,   0=明文 ws 联调
// 定位（1e7 缩放，基准±抖动随机）
#define DTU_LON_BASE        1165583130L
#define DTU_LAT_BASE        397858900L
#define DTU_LOC_JITTER      500000L
// 信号强度随机区间
#define DTU_CSQ_MIN         19
#define DTU_CSQ_MAX         31
#define DTU_HEARTBEAT_INTERVAL_S 20
```

---

## 四、构建与烧录

1. **工具**：MounRiverStudio（WCH 官方 IDE，内置 RISC-V 工具链）。
2. **打开工程**：打开本工程目录（`.cproject`/`.project` 已配置好源文件、头文件路径与 `libwchnet.a` 链接，无需手动加）。
3. **构建**：`Project → Clean` 后 `Build`。产物 `obj/dtu-ethernet-ble.hex`。
4. **烧录接线（WCH-Link）**：

   | WCH-LINK | 板子 |
   |---|---|
   | 3V3 | VDD |
   | GND | GND |
   | SWDIO | PA13 |
   | SWCLK | PA14 |
   | RST  | RST（建议）|

5. **串口**：UART1(PA9) 115200 查看日志（连接/上报/心跳/查询应答）。

---

## 五、wss 的 TLS 说明（重要）

WCHNET 协议栈只提供 TCP，**不含 TLS**。`wss`（443）需要一个 TLS 引擎，这是工程里唯一未随仓库提供、需接入的组件：

- `User/wss_tls.c` 提供了统一的 TLS 字节流接口；接入所需的 mbedTLS（建议裁剪配置、为省 32K SRAM 关闭证书链校验）或其它 TLS 实现，在该文件 `DTU_TLS=1` 分支实现即可，其余代码无需改动。
- 先用**明文联调**：把 `DTU_TLS` 设 `0`、`DTU_PORT` 设服务端可用的 ws 端口，验证心跳/上报/应答整条链路，再接 TLS。

便携协议层（SHA-1/base64、WebSocket 握手/分帧/Ping-Pong、查询应答）已在本机脚本对本地 WebSocket 服务端做过联测。

---

## 六、协议摘要（对齐服务端 DeviceYedS710YmProcessor）

- 地址：`wss(s)://host/v1/{org}/{model}/{IMEI}`，`org=yinerda`、`model=yed-s710ym`
- 心跳（二进制，每 20s）：`0a 0b 0c 0d 0e 0f`
- 信号强度（文本帧）：`csq=31`
- 定位（文本帧）：`coordinates=经度,纬度`（第一个值是**经度**）
- 服务端查询（二进制）`01 02 03 04` → 回 `55 aa 55 aa`
- IMEI 已在 URL 路径中，协议内无需单独上报

---

## 七、许可证

见根目录 `LICENSE`（本项目基于 WCH CH32V20x 官方库扩展）。