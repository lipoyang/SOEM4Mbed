#include <mbed.h>
#ifdef __cplusplus
extern "C"
{
#endif
#include "stm32f7xx_hal.h"

// local RX buffer
#define FRAME_LENGTH_MAX 1536
static unsigned char ethernetFrameBuffer[FRAME_LENGTH_MAX];

// Ethernet peripheral handle
static ETH_HandleTypeDef EthHandle;
// Ethernet Rx DMA Descriptor
extern ETH_DMADescTypeDef DMARxDscrTab[ETH_RXBUFNB];
// Ethernet Tx DMA Descriptor
extern ETH_DMADescTypeDef DMATxDscrTab[ETH_TXBUFNB];
// Ethernet Receive Buffer
extern uint8_t Rx_Buff[ETH_RXBUFNB][ETH_RX_BUF_SIZE];
// Ethernet Transmit Buffer
extern uint8_t Tx_Buff[ETH_TXBUFNB][ETH_TX_BUF_SIZE];

// (1) open
// return: 0=SUCCESS / 1=FAILED
int hal_ethernet_open(void)
{
	printf("hal_ethernet_open...\n");
	
	// Initialize Ethernet peripheral
	uint8_t MACAddr[6] = {0x0};								// dummy MAC address
	EthHandle.State = HAL_ETH_STATE_RESET;
	EthHandle.Instance = ETH;
	EthHandle.Init.MACAddr = &MACAddr[0];
	EthHandle.Init.AutoNegotiation = ETH_AUTONEGOTIATION_DISABLE; // Disable auto negotiation
	EthHandle.Init.Speed = ETH_SPEED_100M;
	EthHandle.Init.DuplexMode = ETH_MODE_FULLDUPLEX;
	EthHandle.Init.MediaInterface = ETH_MEDIA_INTERFACE_RMII;
	EthHandle.Init.RxMode = ETH_RXPOLLING_MODE;				// Not use interrupt
	EthHandle.Init.ChecksumMode = ETH_CHECKSUM_BY_HARDWARE;
	EthHandle.Init.PhyAddress = 0x00;
	bool init_ok = false;
	for(int i=0;i<20;i++){
		//wait_ms(100);
		thread_sleep_for(100);
		if (HAL_ETH_Init(&EthHandle) == HAL_OK)
		{
			init_ok = true;
			break;
		}
	}
	if(!init_ok){
		printf("HAL_ETH_Init ERROR!\n");
		return 1;
	}
	// Disable interrupt
	NVIC_DisableIRQ(ETH_IRQn);
	
	// Initialize Tx Descriptors list: Chain Mode
	HAL_ETH_DMATxDescListInit(&EthHandle, DMATxDscrTab, &Tx_Buff[0][0], ETH_TXBUFNB);
	// Initialize Rx Descriptors list: Chain Mode
	HAL_ETH_DMARxDescListInit(&EthHandle, DMARxDscrTab, &Rx_Buff[0][0], ETH_RXBUFNB);
	// Enable MAC and DMA transmission and reception
	HAL_ETH_Start(&EthHandle);
	
	// wait for link up (timeout 2sec)
	for(int i=0;i<20;i++){
		//wait_ms(100);
		thread_sleep_for(100);
		uint32_t status = 0;
		if (HAL_ETH_ReadPHYRegister(&EthHandle, PHY_BSR, &status) == HAL_OK) {
			printf("status = %08X\n", (unsigned int)status);
			if(status & PHY_LINKED_STATUS){
				printf("hal_ethernet_open ok\n");
				return 0;
			}
		}
	}
	printf("hal_ethernet_open failed\n");
	return 1;
}

// (2) close
void hal_ethernet_close(void)
{
	// Disable interrupt
	NVIC_DisableIRQ(ETH_IRQn);
	// Disable MAC and DMA transmission and reception
	HAL_ETH_Stop(&EthHandle);
}

// (3) send
// return: 0=SUCCESS (!!! not sent size)
int hal_ethernet_send(unsigned char *data, int len)
{
	// link check
	bool is_up = false;
	uint32_t status;
	if (HAL_ETH_ReadPHYRegister(&EthHandle, PHY_BSR, &status) == HAL_OK) {
		if(status & PHY_LINKED_STATUS){
			is_up = true;
		}
	}
	if(!is_up){
		hal_ethernet_close();
		if(hal_ethernet_open() == 1){
			printf("hal_ethernet_send error: Not link up\n");
			return -3;
		}
	}
	
	int ret = 0;
	// Is this buffer available?
	if ((EthHandle.TxDesc->Status & ETH_DMATXDESC_OWN) != (uint32_t)RESET){
		printf("hal_ethernet_send error: DMA Busy\n");
		ret = -1;
	}
	else{
		// Copy data to Tx buffer
		uint8_t* buffer = (uint8_t*)(EthHandle.TxDesc->Buffer1Addr);
		memcpy (buffer, data, len);

		// Prepare transmit descriptors to give to DMA
		HAL_ETH_TransmitFrame(&EthHandle, len);
	}

	// When Transmit Underflow flag is set,
	// clear it and issue a Transmit Poll Demand to resume transmission
	if ((EthHandle.Instance->DMASR & ETH_DMASR_TUS) != (uint32_t)RESET) {
		// Clear TUS ETHERNET DMA flag
		EthHandle.Instance->DMASR = ETH_DMASR_TUS;
		// Resume DMA transmission
		EthHandle.Instance->DMATPDR = 0;
	}
	return ret;
}

// (4) receive
// return: received size
int hal_ethernet_recv(unsigned char **data)
{
	// get received frame
	if (HAL_ETH_GetReceivedFrame(&EthHandle) != HAL_OK) {
		//printf("HAL_ETH_GetReceivedFrame ERROR!\n");
		return 0; // no packet received
	}
	// Obtain the size of the packet
	uint16_t len = EthHandle.RxFrameInfos.length;
	if(len <= 0){
		printf("hal_ethernet_recv: No Data or Error\n");
		return 0;
	}
	if(len > FRAME_LENGTH_MAX){
		printf("hal_ethernet_recv: Too long data! (%d byte)\n", len);
		len = FRAME_LENGTH_MAX;
	}
	//printf("hal_ethernet_recv: received data! (%d byte)\n", len);
	
	// DMA buffer pointer
	uint8_t* buffer = (uint8_t*)EthHandle.RxFrameInfos.buffer;
	// Copy data to my buffer
	memcpy(ethernetFrameBuffer, buffer, len);
	// Release descriptors to DMA
	EthHandle.RxFrameInfos.FSRxDesc->Status |= ETH_DMARXDESC_OWN;
	// Clear Segment_Count
	EthHandle.RxFrameInfos.SegCount = 0;
	// When Rx Buffer unavailable flag is set: clear it and resume reception
	if ((EthHandle.Instance->DMASR & ETH_DMASR_RBUS) != (uint32_t)RESET) {
		// Clear RBUS ETHERNET DMA flag
		EthHandle.Instance->DMASR = ETH_DMASR_RBUS;
		// Resume DMA reception
		EthHandle.Instance->DMARPDR = 0;
	}
	// received data
	*data = ethernetFrameBuffer;
	return len;
}

#ifdef __cplusplus
}
#endif
