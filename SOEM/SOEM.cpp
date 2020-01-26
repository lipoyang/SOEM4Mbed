#include <mbed.h>

#define FRAME_LENGTH_MAX 1536

static Ethernet s_ethernet;
static unsigned char ethernetFrameBuffer[FRAME_LENGTH_MAX];

#ifdef __cplusplus
extern "C"
{
#endif

// (1) open
// return: 0=SUCCESS
int hal_ethernet_open(void)
{
	s_ethernet.set_link(Ethernet::FullDuplex100);

	// TODO timeout
	do{
		wait_ms(10);
	}while(s_ethernet.link() == 0);

    return 0;
}

// (2) close
void hal_ethernet_close(void)
{
	// do nothing
}

// (3) send
// return: 0=SUCCESS (!!! not sent size)
int hal_ethernet_send(unsigned char *data, int len)
{
	int ret = s_ethernet.write((const char *)data, len);
	if(ret != len){
		return -1;
	}
	ret = s_ethernet.send();
	if(ret == 0){
		return -2;
	}
	return 0;
}

// (4) receive
// return: received size
int hal_ethernet_recv(unsigned char **data)
{
    int size = s_ethernet.receive();
    if(size == 0){
    	return 0;
    }else if(size > FRAME_LENGTH_MAX){
    	return -1;
    }
    int size2 = s_ethernet.read((char*)ethernetFrameBuffer, size);
    if(size2 != size){
    	return -2;
    }
    *data = ethernetFrameBuffer;
    return size;
}

#ifdef __cplusplus
}
#endif
