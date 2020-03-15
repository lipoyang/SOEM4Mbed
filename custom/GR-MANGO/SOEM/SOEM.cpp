#include <mbed.h>

#if defined(__cplusplus)
extern "C" {
#endif
#include "r_ether_rza2_if.h"
//#include "FreeRTOS.h"
//#include "task.h"
//#include "r_ether_rx_if.h"
//#include "r_ether_rx_pinset.h"
int32_t callback_ether_regist(void);
#ifdef __cplusplus
}
#endif

#define ETHER_CH    ETHER_CHANNEL_0

#define ETHER_BUFSIZE_MIN 60
#define MY_MAC_ADDR    0x02,0x00,0x00,0x00,0x00,0x00
static uint8_t myethaddr[6] = {MY_MAC_ADDR};

#ifdef __cplusplus
extern "C"
{
#endif

#if 0
static TaskHandle_t my_ether_link_check_task_handle = 0;
static void my_check_ether_link(void * pvParameters)
{
    R_INTERNAL_NOT_USED(pvParameters);

    while(1)
    {
        vTaskDelay(1000);
        R_ETHER_LinkProcess(ETHER_CH);
    }
}
#endif

void callback_ether(void * pparam);
static int32_t my_callback_ether_regist(void)
{
    ether_param_t   param;
    ether_cb_t      cb_func;

    int32_t         ret;
#if 0 // TODO HOGE
    /* Set the callback function (LAN cable connect/disconnect event) */
    cb_func.pcb_func     = &callback_ether;
    param.ether_callback = cb_func;
    ret = R_ETHER_Control(CONTROL_SET_CALLBACK, param);
    if (ETHER_SUCCESS != ret)
    {
        return -1;
    }
#endif
#if 0
    /* Set the callback function (Ether interrupt event) */
    cb_func.pcb_int_hnd     = &EINT_Trig_isr;
    param.ether_callback = cb_func;
    ret = R_ETHER_Control(CONTROL_SET_INT_HANDLER, param);
    if (ETHER_SUCCESS != ret)
    {
        return -1;
    }
#endif
    return 0;
}

// (1) open
// return: 0=SUCCESS
int  hal_ethernet_open(void)
{
    ether_param_t   param;
    ether_return_t eth_ret;
    
    R_ETHER_Initial();
    //my_callback_ether_regist();
    int result = R_ETHER_Open_ZC2(ETHER_CH, myethaddr, ETHER_FLAG_OFF);
    param.channel = ETHER_CH;
    eth_ret = R_ETHER_Control(CONTROL_POWER_ON, param); // PHY mode settings, module stop cancellation
    
    do //TODO allow for timeout
    {
        eth_ret = R_ETHER_CheckLink_ZC(ETHER_CH);
        wait_us(10); // TODO
    }
    while(ETHER_SUCCESS != eth_ret);
    
    R_ETHER_LinkProcess(ETHER_CH);
    
#if 0
    long return_code;
    return_code = xTaskCreate(my_check_ether_link,
                              "CHECK_ETHER_LINK_TIMER",
                              100,
                              0,
                              configMAX_PRIORITIES,
                              &my_ether_link_check_task_handle);
    if (pdFALSE == return_code)
    {
        return pdFALSE;
    }
#endif
    return result;	
}

// (2) close
void hal_ethernet_close(void)
{
    R_ETHER_Close_ZC2(ETHER_CH);
}

// (3) send
// return: 0=SUCCESS (!!! not sent size)
int  hal_ethernet_send(unsigned char *pucBuffer, int length)
{
    ether_return_t ret;
    uint8_t * pwrite_buffer;
    uint16_t write_buf_size;
    
    // get buffer to send data
    ret = R_ETHER_Write_ZC2_GetBuf(ETHER_CH, (void **) &pwrite_buffer, &write_buf_size);
    if (ETHER_SUCCESS != ret)
    {
        //debug_print("R_ETHER_Write_ZC2_GetBuf failed!\n");
        return -1;
    }
    // copy data to buffer if size enough
    if ((write_buf_size >= length) && (write_buf_size >= ETHER_BUFSIZE_MIN))
    {
        memcpy(pwrite_buffer, pucBuffer, length);
    }else{
        return -2;
    }
    // minimum Ethernet frame size is 60 bytes.
    // zero padding and resize if under minimum.
    if (length < ETHER_BUFSIZE_MIN)
    {
        memset((pwrite_buffer + length), 0, (ETHER_BUFSIZE_MIN - length));  // padding
        length = ETHER_BUFSIZE_MIN;                                         // resize
    }
    
    ret = R_ETHER_Write_ZC2_SetBuf(ETHER_CH, (uint16_t)length);
    if(ret != ETHER_SUCCESS){
        return -3;
    }
    
    ret = R_ETHER_CheckWrite(ETHER_CH);
    if(ret != ETHER_SUCCESS){
        return -4;
    }
    
    return 0;
}

// (4) receive
// return: received size
int  hal_ethernet_recv(unsigned char **data)
{
    int result = R_ETHER_Read_ZC2(ETHER_CH, (void **)data);
#if 0
    int i;
    for(i=0;i<result;i++){
        debug_print("%02X " , *data[i]);
    }
    if(result>0) hoge_print("\n");
#endif
    return result;
}

// release received buffer
// don't forget call this after hal_ethernet_recv()
void hal_ethernet_recv_rel(void)
{
    R_ETHER_Read_ZC2_BufRelease(ETHER_CH);
}

#ifdef __cplusplus
}
#endif

