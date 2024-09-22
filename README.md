# CUBE_NANDO_HAL
NANDO固件分区
/* Flash layout
 * ------------ 0x08000000
 * |bootloader |
 * |14K        |
 * -------------0x08003800
 * |data       |
 * |2K         |
 * ------------ 0x08004000
 * |APP1     |
 * |120K       |
 * -------------0x08022000
 * |APP2     |
 * |120K       |
 * -------------0x08040000

一、源码编译及使用说明
1.bootloader编译及烧写
bootloader——适配STM32F407VET6的bootloader。
使用STM32CUBEIDE编译bootloader，然后使用STLINK V2烧写；

2.APP编译及烧写
2.1APP源码说明
APP源码分为USBFS和USBHS两个版本。USBHS为STM32F407VET6+USB3300实现。
NANDO_USBFS——适配STM32F407VET6 USBFS的NANDO APP源码。
NANDO_USBHS——适配STM32F407VET6 USBHS的NANDO APP源码。
2.2编译与烧写（以NANDO_USBFS为例说明）
a.APP1分区
检查NANDO_USBFS目录下STM32F407VETX_FLASH.ld文件中FLASH    (rx)    : ORIGIN = 0x08004000,   LENGTH = 120K），使用STM32CUBEIDE编译,然后使用STLINK V2烧写.

b.APP2分区
修改STM32F407VETX_FLASH.ld文件中FLASH    (rx)    : ORIGIN = 0x08022000,   LENGTH = 120K），编译后写入APP2分区。

二、其它MCU移植说明
     可使用STM32CUBEMX配置好硬件接口后生成基础代码，将NANDO_USBFS/nado复制到新的基础代码路径下，加入到编译环境；
参考NANDO_USBFS下的对应文件修改usbd_cdc_if.c,main.c文件。
