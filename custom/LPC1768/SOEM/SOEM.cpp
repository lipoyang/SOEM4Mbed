#include <mbed.h>

#define FRAME_LENGTH_MAX 1536

static Ethernet *pEthernet;
static unsigned char ethernetFrameBuffer[FRAME_LENGTH_MAX];

#ifdef __cplusplus
extern "C"
{
#endif

// (1) open
// return: 0=SUCCESS / 1=FAILED
int hal_ethernet_open(void)
{
	//printf("hal_ethernet_open...\n");
    pEthernet = new Ethernet();
	pEthernet->set_link(Ethernet::FullDuplex100);

	// wait for link up (timeout 2sec)
	for(int i=0;i<500;i++){
		wait_ms(10);
		if(pEthernet->link() == 1){
			//printf("hal_ethernet_open ok\n");
			return 0;
		}
	}
	//printf("hal_ethernet_open failed\n");
    return 1;
}

// (2) close
void hal_ethernet_close(void)
{
	delete pEthernet;
}

// (3) send
// return: 0=SUCCESS (!!! not sent size)
int hal_ethernet_send(unsigned char *data, int len)
{
	// link check
    if (pEthernet->link() == 0){
        hal_ethernet_close();
        if(hal_ethernet_open() == 1){
            return -3;
        }
    }
    
	int ret = pEthernet->write((const char *)data, len);
	if(ret != len){
		//printf("hal_ethernet_send error 1\n");
		return -1;
	}
	ret = pEthernet->send();
	if(ret == 0){
		//printf("hal_ethernet_send error 2\n");
		return -2;
	}
	return 0;
}

// (4) receive
// return: received size
int hal_ethernet_recv(unsigned char **data)
{
    int size = pEthernet->receive();
    if(size == 0){
    	return 0;
    }else if(size > FRAME_LENGTH_MAX){
		//printf("hal_ethernet_recv error 1\n");
    	return -1;
    }
    int size2 = pEthernet->read((char*)ethernetFrameBuffer, size);
    if(size2 != size){
		//printf("hal_ethernet_recv error 2\n");
    	return -2;
    }
    *data = ethernetFrameBuffer;
    return size;
}

#ifdef __cplusplus
}
#endif
