#include "w5500.h"
#include "w5500_spi.h"
#include <stdio.h>

#define W5500_SPI_READ      0x00
#define W5500_SPI_WRITE     0x04

uint8_t W5500_ReadByte(uint16_t address, uint8_t block)
{
    uint8_t control;
    uint8_t data;

    control = block | W5500_SPI_READ;

    W5500_CS_Select();

    W5500_SPI_Transfer(address >> 8);
    W5500_SPI_Transfer(address & 0xFF);

    W5500_SPI_Transfer(control);

    data = W5500_SPI_Transfer(0xFF);

    W5500_CS_Unselect();

    return data;
}


void W5500_WriteByte(uint16_t address, uint8_t block, uint8_t data)
{
    uint8_t control;

    control = block | W5500_SPI_WRITE;

    W5500_CS_Select();

    W5500_SPI_Transfer(address >> 8);
    W5500_SPI_Transfer(address & 0xFF);

    W5500_SPI_Transfer(control);

    W5500_SPI_Transfer(data);

    W5500_CS_Unselect();
}

void W5500_ReadBuffer(uint16_t address,
                      uint8_t block,
                      uint8_t *buffer,
                      uint16_t length)
{
    uint8_t control;

    control = block | W5500_SPI_READ;

    W5500_CS_Select();

    W5500_SPI_Transfer(address >> 8);
    W5500_SPI_Transfer(address & 0xFF);

    W5500_SPI_Transfer(control);

    while(length--)
    {
        *buffer++ = W5500_SPI_Transfer(0xFF);
    }

    W5500_CS_Unselect();
}

void W5500_WriteBuffer(uint16_t address,
                       uint8_t block,
                       uint8_t *buffer,
                       uint16_t length)
{
    uint8_t control;

    control = block | W5500_SPI_WRITE;

    W5500_CS_Select();

    W5500_SPI_Transfer(address >> 8);
    W5500_SPI_Transfer(address & 0xFF);

    W5500_SPI_Transfer(control);

    while(length--)
    {
        W5500_SPI_Transfer(*buffer++);
    }

    W5500_CS_Unselect();
}

void W5500_SetNetworkConfig(W5500_NetworkConfig_t *config)
{
    // Set Gateway
    W5500_WriteBuffer(
        W5500_GAR,
        W5500_COMMON_REG,
        config->gateway,
        4
    );

    // Set Subnet Mask
    W5500_WriteBuffer(
        W5500_SUBR,
        W5500_COMMON_REG,
        config->subnet,
        4
    );

    // Set MAC Address
    W5500_WriteBuffer(
        W5500_SHAR,
        W5500_COMMON_REG,
        config->mac,
        6
    );

    // Set Source IP
    W5500_WriteBuffer(
        W5500_SIPR,
        W5500_COMMON_REG,
        config->ip,
        4
    );
}

void W5500_SetMAC(uint8_t *mac)
{
    W5500_WriteBuffer(
        W5500_SHAR,
        W5500_COMMON_REG,
        mac,
        6
    );
}


void W5500_SetIP(uint8_t *ip)
{
    W5500_WriteBuffer(
        W5500_SIPR,
        W5500_COMMON_REG,
        ip,
        4
    );
}


void W5500_SetGateway(uint8_t *gateway)
{
    W5500_WriteBuffer(
        W5500_GAR,
        W5500_COMMON_REG,
        gateway,
        4
    );
}


void W5500_SetSubnet(uint8_t *subnet)
{
    W5500_WriteBuffer(
        W5500_SUBR,
        W5500_COMMON_REG,
        subnet,
        4
    );
}

uint8_t W5500_GetLinkStatus(void)
{
    uint8_t phycfgr;

    phycfgr = W5500_ReadByte(
        W5500_PHYCFGR,
        W5500_COMMON_REG
    );

    return (phycfgr & (1U << 2)) ? 1 : 0;
}

uint8_t W5500_GetStateSocket(uint8_t socket)
{
	return W5500_ReadByte(W5500_Sn_SR, W5500_SOCK_REG(socket));
}

void W5500_SocketCommand(uint8_t socket, uint8_t command)
{
    uint8_t block;

    block = W5500_SOCK_REG(socket);

    W5500_WriteByte(W5500_Sn_CR, block, command);

    while(W5500_ReadByte(W5500_Sn_CR, block) != 0x00);
}

uint16_t W5500_ReadWord(uint16_t address, uint8_t block)
{
    uint8_t data[2];

    W5500_ReadBuffer(
        address,
        block,
        data,
        2
    );

    return ((uint16_t)data[0] << 8) | data[1];
}

void W5500_WriteWord( uint16_t address, uint8_t block, uint16_t data)
{
    uint8_t buffer[2];

    buffer[0] = (uint8_t)(data >> 8);
    buffer[1] = (uint8_t)(data & 0xFF);

    W5500_WriteBuffer(
        address,
        block,
        buffer,
        2
    );
}

uint16_t W5500_GetTxFreeSize(uint8_t socket)
{
    uint8_t block;

    block = W5500_SOCK_REG(socket);

    return W5500_ReadWord(
        W5500_Sn_TX_FSR,
        block
    );
}

static void W5500_SetDestination(uint8_t socket, uint8_t *ip, uint16_t port)
{
    uint8_t block;

    block = W5500_SOCK_REG(socket);

    // Destination IP
    W5500_WriteBuffer(
        W5500_Sn_DIPR,
        block,
        ip,
        4
    );

    // Destination Port
    W5500_WriteWord(
        W5500_Sn_DPORT,
        block,
        port
    );
}
uint8_t W5500_UDP_Open( uint8_t socket, uint8_t *ip, uint16_t port)
{
    uint8_t block;
    uint8_t port_data[2];
    uint8_t status;

    block = W5500_S0_REG + (socket * 4);

    // Set UDP mode
    W5500_WriteByte(
        W5500_Sn_MR,
        block,
        W5500_Sn_MR_UDP
    );

    // Set local port
    port_data[0] = (uint8_t)(port >> 8);
    port_data[1] = (uint8_t)(port & 0xFF);

    W5500_WriteBuffer(
        W5500_Sn_PORT,
        block,
        port_data,
        2
    );

    // Open socket
    W5500_SocketCommand(
        socket,
        W5500_Sn_CR_OPEN
    );

    status = W5500_ReadByte(
        W5500_Sn_SR,
        block
    );

    if(status == 0x22)
    {
			W5500_SetDestination(socket, ip, port);
      return 1;
    }

    return 0;
}

uint8_t W5500_UDP_Close(uint8_t socket)
{
    uint8_t status;
    uint8_t block;

    block = W5500_SOCK_REG(socket);

    // Close socket
    W5500_SocketCommand(socket, W5500_Sn_CR_CLOSE);

    // Check socket status
    status = W5500_ReadByte(W5500_Sn_SR, block);

    if(status == W5500_SOCK_CLOSED)
    {
        return 1;
    }

    return 0;
}

uint8_t W5500_UDP_Send(uint8_t socket, uint8_t *data, uint16_t length)
{
    uint8_t reg_block;
    uint8_t tx_block;

    uint8_t ir;

    uint16_t tx_free;
    uint16_t tx_wr;
    uint16_t tx_offset;

    uint16_t first_length;
    uint16_t second_length;

    reg_block = W5500_SOCK_REG(socket);
    tx_block = W5500_SOCK_TX(socket);

    // Check socket status
    if(W5500_ReadByte(W5500_Sn_SR, reg_block) != 0x22)
    {
        return 0;
    }

    // Wait until TX buffer has enough space
    do
    {
        tx_free = W5500_GetTxFreeSize(socket);

    } while(tx_free < length);

    // Get current TX write pointer
    tx_wr = W5500_ReadWord(
        W5500_Sn_TX_WR,
        reg_block
    );

    // Calculate circular buffer offset
    tx_offset = tx_wr & (W5500_TX_BUFFER_SIZE - 1);

    // Write data into TX buffer
    if((tx_offset + length) <= W5500_TX_BUFFER_SIZE)
    {
        W5500_WriteBuffer(
            tx_offset,
            tx_block,
            data,
            length
        );
    }
    else
    {
        // Write first part
        first_length = W5500_TX_BUFFER_SIZE - tx_offset;

        W5500_WriteBuffer(
            tx_offset,
            tx_block,
            data,
            first_length
        );

        // Write remaining part from buffer beginning
        second_length = length - first_length;

        W5500_WriteBuffer(
            0,
            tx_block,
            &data[first_length],
            second_length
        );
    }

    // Update TX write pointer
    tx_wr += length;

    W5500_WriteWord(
        W5500_Sn_TX_WR,
        reg_block,
        tx_wr
    );

    // Send command
    W5500_SocketCommand(
        socket,
        W5500_Sn_CR_SEND
    );

    // Wait for SEND_OK or TIMEOUT
    while(1)
    {
        ir = W5500_ReadByte(
            W5500_Sn_IR,
            reg_block
        );

        if(ir & W5500_Sn_IR_SENDOK)
        {
            // Clear SEND_OK flag
            W5500_WriteByte(
                W5500_Sn_IR,
                reg_block,
                W5500_Sn_IR_SENDOK
            );

            return 1;
        }

        if(ir & W5500_Sn_IR_TIMEOUT)
        {
            // Clear TIMEOUT flag
            W5500_WriteByte(
                W5500_Sn_IR,
                reg_block,
                W5500_Sn_IR_TIMEOUT
            );

            return 0;
        }
    }
}

uint16_t W5500_UDP_Receive(uint8_t socket, uint8_t *buffer, uint16_t max_len)
{
    uint8_t block = W5500_SOCK_REG(socket);
    uint16_t rx_size = W5500_ReadWord(W5500_Sn_RX_RSR, block); // Ð?c dung lu?ng d? li?u trong RX[cite: 2]

    if (rx_size < 8) return 0; // Ph?i có ít nh?t 8 byte header W5500

    uint16_t rx_rd = W5500_ReadWord(W5500_Sn_RX_RD, block);
    uint8_t header[8];

    // 1. Ð?c 8 byte header c?a W5500 (IP, Port, Length)
    W5500_ReadBuffer(rx_rd, W5500_SOCK_RX(socket), header, 8);
    rx_rd += 8;

    uint16_t payload_len = ((uint16_t)header[6] << 8) | header[7];
    uint16_t read_len = (payload_len > max_len) ? max_len : payload_len;

    // 2. Ð?c ph?n d? li?u (Payload) th?c s?
    W5500_ReadBuffer(rx_rd, W5500_SOCK_RX(socket), buffer, read_len);

    // 3. B? qua toàn b? gói UDP này trong b? d?m W5500
    rx_rd += payload_len;
    W5500_WriteWord(W5500_Sn_RX_RD, block, rx_rd);
    W5500_SocketCommand(socket, W5500_Sn_CR_RECV);

    return read_len;
}
// =============== TCP Client==========
uint8_t W5500_TCP_Open(uint8_t socket, uint16_t port)
{
    uint8_t status;
    uint8_t block;

    block = W5500_SOCK_REG(socket);

    // Close socket first
    W5500_SocketCommand(socket, W5500_Sn_CR_CLOSE);

    // Set TCP mode
    W5500_WriteByte(
        W5500_Sn_MR,
        block,
        W5500_Sn_MR_TCP
    );

    // Set local port
    W5500_WriteWord(
        W5500_Sn_PORT,
        block,
        port
    );

    // Open socket
    W5500_SocketCommand(
        socket,
        W5500_Sn_CR_OPEN
    );

    // Check socket status
    status = W5500_ReadByte(
        W5500_Sn_SR,
        block
    );

    if(status == W5500_SOCK_INIT)
    {
        return 1;
    }

    return 0;
}

uint8_t W5500_TCP_Connect(uint8_t socket, uint8_t *server_ip, uint16_t server_port)
{
    uint8_t status;
    uint8_t block;
    uint16_t timeout = 100;

    block = W5500_SOCK_REG(socket);

    // Set destination IP
    W5500_WriteBuffer(W5500_Sn_DIPR, block, server_ip, 4);

    // Set destination port
    W5500_WriteWord(W5500_Sn_DPORT, block, server_port);

    // Send CONNECT command
    W5500_SocketCommand(socket, W5500_Sn_CR_CONNECT);

    while(timeout--)
    {
        status = W5500_ReadByte(W5500_Sn_SR, block);

        if(status == W5500_SOCK_ESTABLISHED)
        {
            return 1;
        }

        if(status == W5500_SOCK_CLOSED)
        {
            return 0;
        }

        // Delay 1 ms
        for(int i =0; i<5000; i++);
    }

    // Connection timeout
    return 0;
}

uint8_t W5500_TCP_Close(uint8_t socket)
{
    uint8_t status;
    uint8_t block;

    block = W5500_SOCK_REG(socket);

    // Disconnect TCP connection
    status = W5500_ReadByte(W5500_Sn_SR, block);

    if(status == W5500_SOCK_ESTABLISHED || status == W5500_SOCK_CLOSE_WAIT)
    {
        W5500_SocketCommand(socket, W5500_Sn_CR_DISCON);
    }

    // Close socket
    W5500_SocketCommand(socket, W5500_Sn_CR_CLOSE);

    // Check socket status
    status = W5500_ReadByte(W5500_Sn_SR, block);

    if(status == W5500_SOCK_CLOSED)
    {
        return 1;
    }

    return 0;
}

uint8_t W5500_TCP_Send(uint8_t socket, uint8_t *data, uint16_t length)
{
    uint8_t block;
    uint8_t ir;
    uint8_t status;

    uint16_t tx_free_size;
    uint16_t tx_write_pointer;


    block = W5500_SOCK_REG(socket);


    // Check socket status

    status = W5500_ReadByte(
        W5500_Sn_SR,
        block
    );

    if(status != W5500_SOCK_ESTABLISHED)
    {
        return 0;
    }


    // Wait until TX buffer has enough free space

    while(1)
    {
        tx_free_size = W5500_ReadWord(W5500_Sn_TX_FSR, block);

        if(tx_free_size >= length)
        {
            break;
        }


        // Check socket status while waiting

        status = W5500_ReadByte(W5500_Sn_SR, block);

        if(status != W5500_SOCK_ESTABLISHED)
        {
            return 0;
        }
    }


    // Get current TX write pointer

    tx_write_pointer = W5500_ReadWord(W5500_Sn_TX_WR, block);


    // Write data into TX buffer

    W5500_WriteBuffer(tx_write_pointer, W5500_SOCK_TX(socket), data, length);


    // Update TX write pointer

    tx_write_pointer += length;

    W5500_WriteWord(W5500_Sn_TX_WR, block, tx_write_pointer);


    // Clear previous SEND_OK and TIMEOUT flags

    W5500_WriteByte(W5500_Sn_IR, block, W5500_Sn_IR_SENDOK | W5500_Sn_IR_TIMEOUT);


    // Send data

    W5500_SocketCommand(socket, W5500_Sn_CR_SEND);


    // Wait for SEND_OK or TIMEOUT

    while(1)
    {
        ir = W5500_ReadByte(
            W5500_Sn_IR,
            block
        );


        // Send successful

        if(ir & W5500_Sn_IR_SENDOK)
        {
            W5500_WriteByte(
                W5500_Sn_IR,
                block,
                W5500_Sn_IR_SENDOK
            );

            return 1;
        }


        // Send timeout

        if(ir & W5500_Sn_IR_TIMEOUT)
        {
            W5500_WriteByte(
                W5500_Sn_IR,
                block,
                W5500_Sn_IR_TIMEOUT
            );

            return 0;
        }


        // Check connection

        status = W5500_ReadByte(
            W5500_Sn_SR,
            block
        );

        if(status != W5500_SOCK_ESTABLISHED)
        {
            return 0;
        }
    }
}

uint16_t W5500_TCP_Receive(uint8_t socket, uint8_t *buffer, uint16_t max_len)
{
    uint8_t block;
    uint16_t rx_size;
    uint16_t rx_read_pointer;
    uint16_t len_to_read;

    block = W5500_SOCK_REG(socket);

    // Read available data size in RX buffer
    rx_size = W5500_ReadWord(W5500_Sn_RX_RSR, block);

    // Return 0 if no data is available
    if(rx_size == 0)
    {
        return 0;
    }

    // Read only up to requested length or available size
    if(rx_size < max_len)
    {
        len_to_read = rx_size;
    }
    else
    {
        len_to_read = max_len;
    }

    // Read current RX read pointer
    rx_read_pointer = W5500_ReadWord(W5500_Sn_RX_RD, block);

    // Read data from RX buffer
    W5500_ReadBuffer(
        rx_read_pointer,
        W5500_SOCK_RX(socket),
        buffer,
        len_to_read
    );

    // Update RX read pointer
    rx_read_pointer += len_to_read;

    W5500_WriteWord(
        W5500_Sn_RX_RD,
        block,
        rx_read_pointer
    );

    // Issue RECV command to notify W5500 buffer space is freed
    W5500_SocketCommand(socket, W5500_Sn_CR_RECV);

    return len_to_read;
}

// ================== TCP Server =================//
uint8_t W5500_TCP_Listen(uint8_t socket, uint16_t port)
{
    uint8_t status;
    uint8_t block;

    block = W5500_SOCK_REG(socket);

    // Open TCP socket
    if(!W5500_TCP_Open(socket, port))
    {
        return 0;
    }

    // Check socket state
    status = W5500_ReadByte(W5500_Sn_SR, block);

    if(status != W5500_SOCK_INIT)
    {
        return 0;
    }

    // Start listening
    W5500_SocketCommand(socket, W5500_Sn_CR_LISTEN);

    // Check listening state
    status = W5500_ReadByte(W5500_Sn_SR, block);

    if(status != W5500_SOCK_LISTEN)
    {
        return 0;
    }

    return 1;
}

uint8_t W5500_TCP_Server_IsConnected(uint8_t socket)
{
    uint8_t status;

    status = W5500_ReadByte(W5500_Sn_SR, W5500_SOCK_REG(socket));

    if(status == W5500_SOCK_ESTABLISHED)
    {
        return 1;
    }

    return 0;
}

uint8_t W5500_TCP_Server_Check(uint8_t socket, uint16_t port)
{
    uint8_t status;

    status = W5500_ReadByte(W5500_Sn_SR, W5500_SOCK_REG(socket));

    switch(status)
    {
        case W5500_SOCK_CLOSED:
            W5500_TCP_Listen(socket, port);
            return 0;

        case W5500_SOCK_INIT:
            W5500_TCP_Listen(socket, port);
            return 0;

        case W5500_SOCK_LISTEN:
            return 0;

        case W5500_SOCK_ESTABLISHED:
            return 1;

        case W5500_SOCK_CLOSE_WAIT:
            W5500_TCP_Close(socket);
            return 0;

        default:
            W5500_SocketCommand(socket, W5500_Sn_CR_CLOSE);
            return 0;
    }
}