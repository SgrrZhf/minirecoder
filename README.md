# minirecoder

A serial port tool for recording serial port data. A low resource consumption tool writen by C.

## Build and Install

1. cmake -B build
1. cd build
1. make
1. make install

> It will be installed into /usr/local/bin

## Uninstall

1. cd build
1. args rm < install_manifest.txt

## Usage

 `minirecoder -b 115200 -t -C log.txt /dev/ttyUSB0`

open /dev/ttyUSB0 and print data to terminal and add timestamp in every new line beginning and save all data to log.txt

-b : baudrate
-t : enable timestamp
-C : enable save data to a file.

output looks like this:

```bash
$ minirecoder -b 115200 -t /dev/ttyUSB1  
[2020-07-05 09:53:04] 重启 in 0 seconds...
[2020-07-05 09:53:05] ets Jun  8 2016 00:22:57
[2020-07-05 09:53:05] 
[2020-07-05 09:53:05] rst:0xc (SW_CPU_RESET),boot:0x12 (SPI_FAST_FLASH_BOOT)
[2020-07-05 09:53:05] configsip: 0, SPIWP:0xee
[2020-07-05 09:53:05] clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
[2020-07-05 09:53:05] mode:DIO, clock div:2
[2020-07-05 09:53:05] load:0x3fff0030,len:4
[2020-07-05 09:53:05] load:0x3fff0034,len:7140
[2020-07-05 09:53:05] load:0x40078000,len:13696
[2020-07-05 09:53:05] load:0x40080400,len:4000
[2020-07-05 09:53:05] entry 0x40080688
[2020-07-05 09:53:05] I (29) boot: ESP-IDF v4.1-dev-3856-g517948b91 2nd stage bootloader
[2020-07-05 09:53:05] I (29) boot: compile time 22:57:31
```
