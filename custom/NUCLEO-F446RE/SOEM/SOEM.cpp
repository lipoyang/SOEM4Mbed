#include <mbed.h>
#include "W5500/w5500.h"

// W5500 RAW socket
static SOCKET sock;
// W5500 RAW socket buffer
static unsigned char socketBuffer[1536];

// Ethernet Shield 2
// nCS for SD Card
static DigitalOut *SPI_CS_SD;

#ifdef __cplusplus
extern "C"
{
#endif

// (1) open
// return: 0=SUCCESS / 1=FAILED
int hal_ethernet_open(void)
{
    // Ethernet Shield 2
    // disable nCS for SD Card
	SPI_CS_SD = new DigitalOut(D4);
    *SPI_CS_SD = 1;

    w5500.init();
    w5500.writeSnMR(sock, SnMR::MACRAW); 
    w5500.execCmdSn(sock, Sock_OPEN);
    return 0;
}

// (2) close
void hal_ethernet_close(void)
{
    // w5500 doesn't have close() or terminate() method
    w5500.swReset();
}

// (3) send
// return: 0=SUCCESS (!!! not sent size)
int hal_ethernet_send(unsigned char *data, int len)
{
    w5500.send_data_processing(sock, (uint8_t *)data, len);
    w5500.execCmdSn(sock, Sock_SEND_MAC);
    return 0;
}

// (4) receive
// return: received size
int hal_ethernet_recv(unsigned char **data)
{
    unsigned short packetSize;
    int res = w5500.getRXReceivedSize(sock);
    if(res > 0){
        // first 2byte shows packet size
        w5500.recv_data_processing(sock, (uint8_t*)socketBuffer, 2);
        w5500.execCmdSn(sock, Sock_RECV);
        // packet size
        packetSize  = socketBuffer[0];
        packetSize  <<= 8;
        packetSize  &= 0xFF00;
        packetSize  |= (unsigned short)( (unsigned char)socketBuffer[1] & 0x00FF);
        packetSize  -= 2;
        // get received data
        memset(socketBuffer, 0x00, 1536);
        w5500.recv_data_processing(sock, (uint8_t *)socketBuffer, packetSize);
        w5500.execCmdSn(sock, Sock_RECV);
        *data = socketBuffer;
    }
    return packetSize;
}

#ifdef __cplusplus
}
#endif

