# Simple Open EtherCAT Master Library for Mbed

This is SOEM (Simple Open EtherCAT Master) library ported for Mbed.

It's just for educational or experimental purposes.

The original SOEM is at https://github.com/OpenEtherCATsociety/SOEM

## Supported hardware

* GR-PEACH (Renesas RZ/A1H, ARM Cortex-A9)
* GR-MANGO (Renesas RZ/A2M, ARM Cortex-A9) ([README](custom/README.md))
* mbed LPC1768 ([README](custom/README.md))

## limitations 
Many of MCUs don't have large memory, and has only one LAN port. SOEM4Mbed reduces memory usage, and does not support redundant LAN ports.

Doe to reducing memory usage, some functions are limited.
| item | constant name | original SOEM | SOEM4Mbed |
| ---- | ---- | ---- | ---- |
| max entries in Object Dictionary list | EC_MAXODLIST | 1024 | 64 |
| max entries in Object Entry list | EC_MAXOELIST | 256 | 64 |
| max number of slaves in array | EC_MAXSLAVE | 200 | 64 |
| number of frame buffers | EC_MAXBUF | 16 | 2 |

## Documentation of original SOEM
See https://openethercatsociety.github.io/doc/soem/
