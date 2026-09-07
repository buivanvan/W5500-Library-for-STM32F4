#ifndef W5500_H
#define W5500_H

#include <stdint.h>

#define W5500_COMMON_REG    0x00

#define W5500_MR            0x0000
#define W5500_GAR           0x0001
#define W5500_SUBR          0x0005
#define W5500_SHAR          0x0009
#define W5500_SIPR          0x000F
#define W5500_PHYCFGR       0x002E
#define W5500_VERSIONR      0x0039

// Socket 0
#define W5500_S0_REG        0x08

#define W5500_Sn_MR         0x0000
#define W5500_Sn_CR         0x0001
#define W5500_Sn_IR         0x0002
#define W5500_Sn_SR         0x0003
#define W5500_Sn_PORT       0x0004
#define W5500_Sn_DHAR       0x0006
#define W5500_Sn_DIPR       0x000C
#define W5500_Sn_DPORT      0x0010
#define W5500_Sn_TX_FSR     0x0020
#define W5500_Sn_TX_RD      0x0022
#define W5500_Sn_TX_WR      0x0024
#define W5500_Sn_RX_RSR     0x0026
#define W5500_Sn_RX_RD      0x0028

#define W5500_Sn_MR_CLOSE       0x00
#define W5500_Sn_MR_TCP         0x01
#define W5500_Sn_MR_UDP         0x02
#define W5500_Sn_MR_IPRAW       0x03
#define W5500_Sn_MR_MACRAW      0x04

#define W5500_Sn_CR_OPEN        0x01
#define W5500_Sn_CR_LISTEN      0x02
#define W5500_Sn_CR_CONNECT     0x04
#define W5500_Sn_CR_DISCON      0x08
#define W5500_Sn_CR_CLOSE       0x10
#define W5500_Sn_CR_SEND        0x20
#define W5500_Sn_CR_SEND_MAC    0x21
#define W5500_Sn_CR_SEND_KEEP   0x22
#define W5500_Sn_CR_RECV        0x40

#define W5500_Sn_RXBUF_SIZE     0x001E
#define W5500_Sn_TXBUF_SIZE     0x001F

#define W5500_SOCK_REG(socket) (0x08 + ((socket) * 0x20))
#define W5500_SOCK_TX(socket)  (0x10 + ((socket) * 0x20))
#define W5500_SOCK_RX(socket)  (0x18 + ((socket) * 0x20))
#define W5500_TX_BUFFER_SIZE    2048

#define W5500_Sn_IR_SENDOK      0x10
#define W5500_Sn_IR_TIMEOUT     0x08

#define W5500_SOCK_CLOSED        0x00
#define W5500_SOCK_INIT          0x13
#define W5500_SOCK_LISTEN        0x14
#define W5500_SOCK_SYNSENT       0x15
#define W5500_SOCK_SYNRECV       0x16
#define W5500_SOCK_ESTABLISHED   0x17
#define W5500_SOCK_FIN_WAIT      0x18
#define W5500_SOCK_CLOSING       0x1A
#define W5500_SOCK_TIME_WAIT     0x1B
#define W5500_SOCK_CLOSE_WAIT    0x1C
#define W5500_SOCK_LAST_ACK      0x1D
#define W5500_SOCK_UDP           0x22

typedef struct
{
    uint8_t mac[6];

    uint8_t ip[4];

    uint8_t gateway[4];

    uint8_t subnet[4];

} W5500_NetworkConfig_t;

// =============== Read/ Write ============= //
uint8_t W5500_ReadByte(uint16_t address, uint8_t block);
void W5500_WriteByte(uint16_t address, uint8_t block, uint8_t data);
uint16_t W5500_ReadWord(uint16_t address, uint8_t block);
void W5500_WriteWord(uint16_t address, uint8_t block, uint16_t data);
void W5500_ReadBuffer(uint16_t address, uint8_t block, uint8_t *buffer, uint16_t length);
void W5500_WriteBuffer(uint16_t address, uint8_t block, uint8_t *buffer, uint16_t length);

// =============== Set network ============== //
void W5500_SetNetworkConfig(W5500_NetworkConfig_t *config);
void W5500_SetMAC(uint8_t *mac);
void W5500_SetIP(uint8_t *ip);
void W5500_SetGateway(uint8_t *gateway);
void W5500_SetSubnet(uint8_t *subnet);

// ============ Check connect ========= //
uint8_t W5500_GetLinkStatus(void);
uint8_t W5500_GetStateSocket(uint8_t socket);
void W5500_SocketCommand(uint8_t socket, uint8_t command);

// =============== UDP ================= //
uint8_t W5500_UDP_Open( uint8_t socket, uint8_t *ip, uint16_t port);
uint8_t W5500_UDP_Close(uint8_t socket);
uint8_t W5500_UDP_Send(uint8_t socket, uint8_t *data, uint16_t length);
uint16_t W5500_UDP_Receive(uint8_t socket, uint8_t *buffer, uint16_t max_len);

// =============== TCP Client=================== //
uint8_t W5500_TCP_Open(uint8_t socket, uint16_t port);
uint8_t W5500_TCP_Connect(uint8_t socket, uint8_t *server_ip, uint16_t server_port);
uint8_t W5500_TCP_Close(uint8_t socket);
uint8_t W5500_TCP_Send(uint8_t socket, uint8_t *data, uint16_t length);
uint16_t W5500_TCP_Receive(uint8_t socket, uint8_t *buffer, uint16_t max_len);

// ============== TCP Server ==================//
uint8_t W5500_TCP_Listen(uint8_t socket, uint16_t port);
uint8_t W5500_TCP_Server_IsConnected(uint8_t socket);
uint8_t W5500_TCP_Server_Check(uint8_t socket, uint16_t port);

#endif