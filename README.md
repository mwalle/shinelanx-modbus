# ShineLAN-X Modbus/TCP gateway firmware

## A little bit of background

Why all this? My parents recently got a Growatt inverter. Out-of-the box
you'll need the proprietary Growatt app together with their monitor adapter
and all data is stored on their servers. The adapter is available in three
different flavors: ShineLAN-X, ShineWifi-X and an LTE version. Needless, to
say I've done some prior research how to monitor the inverter by yourself.
There is an [alternative
firmware](https://github.com/OpenInverterGateway/OpenInverterGateway) for
the ShineWifi-X stick and there is also
a [daemon](https://github.com/johanmeijer/grott) which intercept the
traffic and sniff the data.

My parents got the ShineLAN-X adpater - which to my dismay - isn't based on
the ESP8266 SoC but on an STM32F103. Therefore, porting the alternative
firmware for the ShineWifi-X wasn't an option (besides me being not an
Arduino guy). I've tried
[Zephyr](https://github.com/zephyrproject-rtos/zephyr), but it's rather
resource hungry esp. if used with the ENC28J60 ethernet MAC. The STM32F103
on this board only has 48kB of RAM. Also Zephyr doesn't support all POSIX
interfaces, you'll see in a bit why this is important.

Fortunately, there is another small POSIX compatible (RT)OS:
[NuttX](https://github.com/apache/nuttx). The board support is somewhat
clunky, because most files are just copy and paste and adapted to the
actual board, whereas in Zephyr you just need a device tree describing your
board. But it is manageable, I don't think it will change that often once
the basic support is there. Also NuttX is using a proper Kconfig interface
(which looks like the one from the linux kernel). And one very important
detail of NuttX is that it offers POSIX interfaces.

There is an [open-source Modbus TCP to Modbus RTU gateway
daemon](https://github.com/3cky/mbusd) which is exactly what we need.
Because NuttX supports most POSIX interfaces, mbusd is able to compile for
NuttX without *any* changes. Impressive.

Here is how it should work: NuttX will provide the necessary hardware
drivers, like SPI controller, the ENC28J60 network adapter, SoC timers and
watchdog and everything. Luckily for us, that is already there. All we need
is a glue layer (called board support package) which describes the
ShineLAN-X board, like where the GPIOs are connected to or which bus the
ethernet controller is connected to.

NuttX also comes with ready to use applications, like a DHCP client or a
network initialization method and an interactive NuttX shell (nsh). What we
need is a package for the mbusd as well as an application which will
initialize the board like setting MAC address and setting the onboard LED,
serving the button etc. and finally starting the mbusd.

## The ShineLAN-X board hardware specs

* STM32F103RC3, 256kB flash, 48kB RAM
* USB device mode, exposing a CDCACM device
* 16MiB SPI-NOR flash on SPI1
* ENC28J60 on SPI2 (PB12-PB15), with RST# on PC8 and INT# on PC6
* RGB LED on PB1 (red), PB0 (green) and PC5 (blue)
* (internal) blue debug LED PC7
* (internal) red power LED
* SWD debugging header (used for initial programming), the pinout is on
  the silkscreen of the board.

## USB DFU Loader

The external USB port is directly connected to the SoC and therefore, we
can use it to program the flash without disassembling the whole adapter
again. We use [dapboot](https://github.com/devanlai/dapboot) which resides
in the first 8kB of the flash and implements the DFU protocol.

Unfortunately, you'll need to use the SWD debugging header to initially
program dapboot.

### Compiling dapboot

Just clone the [my repository](https://github.com/mwalle/dapboot/) which
has support for the ShineLAN-X board. As the time of this writing, the
[pull request](https://github.com/devanlai/dapboot/pull/56) to get this
upstream is still open.

```
git clone https://github.com/mwalle/dapboot
cd dapboot
make -C src TARGET=SHINELANX
```
### Precompiled dapboot binary

For your convenience, there is a pre-compiled binary in the
`contrib/dapboot/` folder of this repository.

### Flashing dapboot

You'll need a SWD debug probe to connect to the board. I'll be using the
Dangerous Prototypes
[BusBlaster](http://dangerousprototypes.com/docs/Bus_Blaster) with
[openocd](https://openocd.org/).

Contents of the openocd.cfg:

```
source [find interface/ftdi/dp_busblaster_kt-link.cfg]
transport select swd
source [find target/stm32f1x.cfg]
adapter speed 2000
```

Then you can program dapboot with the following command:

```
openocd -f openocd-swd.cfg \
  -c "init" \
  -c "reset halt" \
  -c "flash protect 0 0 last off" \
  -c "reset halt" \
  -c "program dapboot.bin 0x08000000 verify" \
  -c "flash protect 0 0 1 on" \
  -c "reset run" \
  -c "shutdown"
```

This will flash dapboot and protect the first two 4kB sectors from further
programming. The protection is *not* permanent and can be reversed by using
`flash protect 0 0 1 off`.

Of course you can use any other debug probe which supports programming the
STM32.

### Using dapboot

`dapboot` will expose an USB DFU interface if it is started. Once flashed,
you can plug your adapter into your computer[^1] and use standard USB utils
to flash a new image.

By default, `dapboot` will check if there is a valid image on the board and
if that is the case, it will automatically jump to and run that image. If
not, `dapboot` will create the USB DFU interface. Alternatively, you can push
the button while you plug in the adapter into your computer.

Once plugged in, you'll start a new USB device on your computer and you can
use `dfu-util` to download the `nuttx.bin` binary:

```
# list available devices
dfu-util -l
# download the image to the device
dfu-util -D nuttx.bin
```

This will download and automatically reset the device. If flashed
correctly, the green LED should turn on, indicating that the application is
started.

[^1]: The USB twist lock of the adapter will probably prevent you from
    plugging it into an USB socket directly. You can use an USB extenstion
    cable.

## Compiling NuttX

Compiling an out-of-tree board and applications for NuttX has some
pecularities. Esp. there are hardcoded paths in the board defconfig,
therefore, you have to use the exact paths as shown below.

```
git clone -b nuttx-12.3.0 --depth 1 https://github.com/apache/nuttx.git
git clone -b nuttx-12.3.0 --depth 1 https://github.com/apache/nuttx-apps.git apps
git clone https://github.com/mwalle/shinelanx-modbus.git external

ln -s ../external/apps apps/external
cd nuttx
tools/configure.sh -a ../apps ../external/board/shinelanx/configs/mbusd/
make -j$(nproc)
```

## CDCACM device

After the adapter is plugged into the inverter, the adpater will serve as a
CDCACM device which is then opened by the inverter. The TTY device is then
connected to a Modbus server on the inverter. Because this is a purely
virtual TTY (COM) port, there is no baudrate (any setting will work).
Please note, that you will have to disable flow control, because the
inverter doesn't support it and if enabled, no communication is possible.

This is also a difference to the ShineWifi-X where there is a dedicated
USB-to-serial chip on the board. As far as I know, they use a special chip
which supports the CDCACM USB class. That is quite unusual because most
USB-to-serial chips use a proprietary interface.

## IP configuration

By default, DHCP is used to obtain an IP address. Alternatively, you can
hardcode the IP configuration. Pleae note, that is is not a runtime
configuration, but it is compiled into the binary itself.

```
cd nuttx

kconfig-tweak --enable NETUTILS_NETINIT
kconfig-tweak --disable NETINIT_DHCPC
# set IP address to 192.168.1.200
kconfig-tweak --set-val NETINIT_IPADDR 0xc0a801c8
# set netmask to 255.255.255.0
kconfig-tweak --set-val NETINIT_NETMASK 0xffffff00
# set default gateway to 192.168.1.254
kconfig-tweak --set-val NETINIT_DRIPADDR 0xc0a801fe

make olddefconfig
make -j$(nproc)
```

## Proprietary commands support

Some inverter support command 32 (20h) which acts just like the *Read Input
Register* command, but to access a different address space. The arguments
and response is the exactly the same as command 4. This is usually used to
read the regsiters connected smart meter.

Because most Modbus clients don't support custom commands, the firmware
will remap addresses from F000h to FFFFh to this command. Keep in mind that
this means, that the largest register you can read with the normal *Read
Input Register* is EFFFh and the largest register you can read with the
proprietary command is 0FFFh.

## ShineLAN-X I/O connections

The SoC has the following connection to its peripherals:

|  Pin | Alternate Function | Connection         |
| ---- | ------------------ | ------------------ |
| PD0  | OSC_IN             | 8MHz crystal       |
| PD1  | OSC_OUT            | 8MHz crystal       |
| PA3  | -                  | BUTTON#            |
| PA4  | SPI1_NSS           | SPI Flash CS       |
| PA5  | SPI1_SCK           | SPI Flash SCLK     |
| PA6  | SPI1_MISO          | SPI Flash SO       |
| PA7  | SPI1_MOSI          | SPI Flash SI       |
| PC4  | -                  | SPI Flash WP#      |
| PC5  | -                  | RGB LED (blue)     |
| PB0  | -                  | RGB LED (green)    |
| PB1  | -                  | RGB LED (red)      |
| PB12 | -                  | ENC28J60 CS#       |
| PB13 | -                  | ENC28J60 SCK       |
| PB14 | -                  | ENC28J60 SO        |
| PB15 | -                  | ENC28J60 SI        |
| PC6  | -                  | ENC28J60 INT#      |
| PC7  | -                  | Blue LED           |
| PC8  | -                  | ENC28J60 RST#      |
| PA8  | -                  | USB re-enumeration |
| PA9  | USART1_TX          | TXD1               |
| PA10 | USART1_RX          | RXD1               |
| PA11 | USB_D-             | RXD1               |
| PA12 | USB_D+             | RXD1               |
| PA13 | SWDIO              | SWDIO              |
| PA14 | SWCLK              | SWCLK              |

# Inverter Support

This alternative firmware was tested successfully with the following
inverters.

## (TBD) MOD-7000 with HV battery ARK-...

The original firmware will poll the follwing registers of the inverter:

* Holding Registers from 0h to 124h
* Holding Registers from 3000h to 3xxxh
* Input Registers from 3000h to 3xxxh

Additionally, the Growatt server and ShinePhone app will write to the
following (undocumented) registers:

| Number | Description | Values |
| --- | --- | --- |
| Holding Registers | |
| 533 | Connected Smart Meter | |

## Licenses

Different parts of this repository are goverened by different licenses:

* `board/` Apache 2.0 License
* `apps/mbusd/` 3-Clause BSD License
* `apps/shinelanx-modbus-gw/` 2-Clause BSD License
