# Customized versions of SOEM4Mbed

## GR-MANGO

GR-MANGO doesn't support `Ethernet` class for handling RAW Ethernet frames. Therefore the customized SOEM library for GR-MANGO uses the low level Ethernet driver of RZ/A2M. 

[SOEM for GR-MANGO](GR-MANGO)

## NUCLEO-F767ZI

NUCLEO-F767ZI doesn't support `Ethernet` class for handling RAW Ethernet frames. Therefore the customized SOEM library for NUCLEO-F767ZI uses the low level Ethernet driver of STM32F7 (STM32F7 HAL library). 

[SOEM for NUCLEO-F767ZI](NUCLEO-F767ZI)

## mbed LPC1768

LPC1768 has only 64kB RAM. In addition, LPC1768's RAM is divided into 3 blocks (AHBSRAM, AHBSRAM0, AHBSRAM1). Therefore the customized SOEM library reduces RAM usage and allocates buffer variables to the vacant area.

[SOEM for LPC1768](LPC1768)
