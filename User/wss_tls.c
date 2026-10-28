/*********************************************************************
 * File Name          : wss_tls.c
 *
 * Description        : wss 的 TLS 适配层。
 *
 * 【重要】本仓库未附带任何 TLS 引擎（WCHNET 只提供 TCP，不含 TLS）。
 *   - DTU_TLS=0：本文件做明文透传，直接桥接 transport_tcp_read/write。
 *     用于先把「WebSocket 握手 + 心跳 + csq/coordinates 上报 + 查询应答」整条
 *     DTU 链路跑通（服务端若提供 ws:// 明文入口，用这种最省）。
 *   - DTU_TLS=1：把 wss_tls_read/write 的明文直通替换为对所选 TLS 引擎
 *     （如 mbedTLS）实例的收发，并在 wss_tls_handshake() 里完成握手。
 *     底层仍是 transport_tcp_read/write；mbedTLS 需裁剪配置以适配 32K SRAM
 *     （例如关闭证书链校验、固定/内嵌服务器证书）。
 *     说明：把 TLS 引擎当作选型项，未随仓库提供；接入后本文件是唯一需要改动处。
 *******************************************************************************/
#include "wss_tls.h"
#include "dtu_config.h"
#include "transport_eth.h"

#if DTU_TLS == 0
/* ---------------- 明文透传（ws://） ---------------- */
void wss_tls_init(void) { }
int  wss_tls_handshake(void) { return 0; }
int  wss_tls_read(uint8_t *buf, uint32_t maxlen) { return transport_tcp_read(buf, maxlen); }
int  wss_tls_write(const uint8_t *buf, uint32_t len) { return transport_tcp_write(buf, len); }
int  wss_tls_is_open(void) { return transport_is_online(); }
void wss_tls_close(void) { }

#else /* DTU_TLS == 1: wss，需 TLS 引擎 */

/* TLS 引擎实例句柄（待接入）。当前未实现，先用明文占位以便编译与联调，
 * 接入后将其替换为真实 TLS 上下文。 */
static int tls_ready;   /* 握手完成后置 1 */

void wss_tls_init(void)
{
    tls_ready = 0;
}

int wss_tls_handshake(void)
{
    /* TODO: 在此调用所选 TLS 引擎，用 transport_tcp_read/write 收发握手包。
     * 例：
     *   配置并调用 mbedtls_ssl_setup()+set_hostname()+setup_bio(指向
     *   transport_tcp_read/write)+mbedtls_ssl_handshake()。
     * 当前为占位：直接返回成功（等价明文），便于先联调 ws/app 逻辑。 */
    tls_ready = 1;
    return 0;
}

int wss_tls_read(uint8_t *buf, uint32_t maxlen)
{
    if (!tls_ready) return -1;
    if (!transport_is_online()) return -1;
    /* 接入后替换为 mbedtls_ssl_read(ssl, buf, maxlen)，其内部会调用
     * transport_tcp_read 补充密文。 */
    return transport_tcp_read(buf, maxlen);
}

int wss_tls_write(const uint8_t *buf, uint32_t len)
{
    if (!tls_ready) return -1;
    if (!transport_is_online()) return -1;
    /* 接入后替换为 mbedtls_ssl_write(ssl, buf, len)。 */
    return transport_tcp_write(buf, len);
}

int wss_tls_is_open(void)
{
    return tls_ready && transport_is_online();
}

void wss_tls_close(void)
{
    tls_ready = 0;
    /* 接入后在此释放 TLS 上下文；底层 TCP 由 transport 关闭。 */
}

#endif /* DTU_TLS */