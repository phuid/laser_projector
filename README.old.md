# Raspberry Pi laser projector


not finished, not tested

## Table of contents

- [Raspberry Pi laser projector](#raspberry-pi-laser-projector)
  - [Table of contents](#table-of-contents)
  - [Tutorial](#tutorial)
    - [Hardware](#hardware)
    - [Software](#software)
      - [installing whole system from my disk image](#installing-whole-system-from-my-disk-image)
      - [installing executables from my repo source code](#installing-executables-from-my-repo-source-code)
        - [cloning my repository and installing manually](#cloning-my-repository-and-installing-manually)
    - [Hardware construction](#hardware-construction)
  - [What my code does - what happens when your RPI starts](#what-my-code-does---what-happens-when-your-rpi-starts)
  - [Progress (everything you need is above this part)](#progress-everything-you-need-is-above-this-part)
    - [Software progress](#software-progress)
    - [Hardware progress](#hardware-progress)

## Tutorial

*RPI = Raspberry Pi*
### Hardware

### Software

there are two ways of doing this, you can

- [install from a disk image](#installing-whole-system-from-my-disk-image)
  - much much easier
  - you need to download a 4GB file

  or

- [install the executables and dependencies yourself (download source)](#installing-executables-from-my-repo-source-code)
  - objectively harder
  - needs node and npm installed (i think node comes with the normal version of RPI OS preinstalled but i was using RPI OS Lite, since i needed to fit all this into a small .img file and was using a 4GB SD card)
    - **discord.js library requires node v16.6.0 or newer and npm v8.3.0 or newer**
    - using [Node version manager](https://github.com/nvm-sh/nvm) - [install tutorial](https://github.com/nvm-sh/nvm#installing-and-updating), [usage](https://github.com/nvm-sh/nvm#usage)
      - much more flexible
      - you can choose node and npm version
    - [download and install node and npm binaries yourself](https://www.makersupplies.sg/blogs/tutorials/how-to-install-node-js-and-npm-on-the-raspberry-pi)

#### installing whole system from my disk image

#### installing executables from my repo source code

##### cloning my repository and installing manually

download my code run these in the RPIs terminal

```console
git clone https://github.com/phuid/laser-projector.git
```

change your current working directory

```console
cd laser-projector
```

Install lasershow executable from [rpi-lasershow](https://github.com/tteskac/rpi-lasershow) project made by [@tteskac](https://github.com/tteskac)

enter rpi-lasershow dir

```console
cd rpi-lasershow
```

make the executable

```console
make
```

copy the executable to the main dir

```console
cp lasershow ../
```

exit the rpi-lasershow dir

```console
cd ..
```

at this point you dont really need the rpi-lasershow directory so you can delete it

it doesnt hurt to keep it at all this is just a possibility

```console
rm -r rpi-lasershow
```

now install the system dependencies and WiringPi and pm2
//TODO

then install all the node dependencies and set up pm2

enter every dir where a separate node server is gonna be located (**.**, **./discord_bot**, **./wifi_manager**) and intall its depencencies, then leave it

```console
npm i child_process http formidable fs path && pm2 start laser_projector-web-ui.js
``` 

**MAKE SURE, YOU CONNECTED THE AP/STEALTH/WIFI SWITCH CORRECTLY BEFORE STARTING THE wifi_manager.js SCRIPT, I DID IMPLEMENT A FAILSAFE, BUT EVEN I DONT REALLY TRUST IT (the failsafe includes the script not running if a pin isnt connected)

//TODO what fkin pin

```console
cd wifi_manager && npm i onoff child_process fs && pm2 start wifi_manager.js && cd ..
```

```console
cd discord_bot && npm i discord.js @discordjs/builders @discordjs/rest child_process path request fs
```

while you still are in **./discord_bot** you should put your bot credentials into config.json file and register the bots commands by running the deploy_commands.js script

```console
nano config.json
```

```console
node ./deploy_commands.js
```

now configure the pm2 autostart

```console
pm2 start discord_bot.js
```

then you can exit it

```console
cd ..
```

now check that all the pm2 services are online

```console
pm2 status
```

there should be three processes online like you can see below

//TODO

###### possible errors
if wifi manager isn't running and `pm2 logs wifi_manager` displays following
```js
/home/pi/laser_projector/wifi_manager/node_modules/bindings/bindings.js:121
        throw e;
        ^

Error: libnode.so.72: cannot open shared object file: No such file or directory
    at Object.Module._extensions..node (node:internal/modules/cjs/loader:1249:18)
    at Module.load (node:internal/modules/cjs/loader:1043:32)
    at Function.Module._load (node:internal/modules/cjs/loader:878:12)
    at Module.require (node:internal/modules/cjs/loader:1067:19)
    at require (node:internal/modules/cjs/helpers:103:18)
    at bindings (/home/pi/laser_projector/wifi_manager/node_modules/bindings/bindings.js:112:48)
    at /home/pi/laser_projector/wifi_manager/node_modules/epoll/epoll.js:7:31
    at Object.<anonymous> (/home/pi/laser_projector/wifi_manager/node_modules/epoll/epoll.js:15:3)
    at Module._compile (node:internal/modules/cjs/loader:1165:14)
    at Object.Module._extensions..js (node:internal/modules/cjs/loader:1219:10) {
  code: 'ERR_DLOPEN_FAILED'
}
```
run `cd wifi_manager/ && npm clean-install` in the laser_projector directory

### Hardware construction

this part is not gonna be added untill around Febuary 2022, since that's when my brother is gonna start working on modeling the case and when we are gonna buy the Galvos and all other components and put them together.

## What my code does - what happens when your RPI starts

1. pm2 starts ./laser_projector-web_ui.js, ./wifi_manage/wifi_manager.js and ./discord_bot/discord_bot.js

2. wifi_manager configures the wlan0 module based on the AP/stealth/wifi switch  

    *grounded = connected to ground*  
    *AP = access point (hotspot)*  

    1. if neither of pins (GPIO/BCM pin 6, GPIO/BCM pin 5) are grounded my code runs `sudo ifconfig wlan0 down`  
    and  
    `sudo systemctl disable hostapd dnsmasq`  
    else it runs just  
    `sudo ifconfig wlan0 up`

    2. if pin 6 is grounded it starts configuring an AP on the RPI

        1. `sudo systemctl enable hostapd dnsmasq`

        2. removes comment from the last section of my `/etc/dhcpcd.conf` file (add following to the file)  

                interface wlan0
                    static ip_address=192.168.4.1/24
                    nohook wpa_supplicant

        3. reboots the RPI

        4. every time it starts (on boot) it checks if hostapd.service is enabled
            - if it is NOT, it enables it in steps 1., 2. and 3. above
            - if it is, it continues

        5. `sudo ifdown wlan0`

        6. `sudo systemctl start dnsmasq`

        7. `sudo hostapd -B /etc/hostapd/hostapd.conf`

        8. `sudo systemctl reload dnsmasq`

    3. if pin 5 is grounded it starts configuring default wifi connection on the RPI
        1. `sudo systemctl stop hostapd dnsmasq`

        2. checks, if RPI is already connected to a wifi
            - if it is it breaks and doesnt continue
            - if it isn't it continues with steps 

        3. `sudo systemctl disable hostapd dnsmasq`

        4. comments out the last section of my `/etc/dhcpcd.conf` file

                #interface wlan0
                    #static ip_address=192.168.4.1/24
                    #nohook wpa_supplicant

        5. reboots the RPI and doesn't need to do anything after

## Progress (everything you need is above this part)

### Software progress

### Hardware progress

for some reason my pi saw the voltage on some pins as HIGH when they werent connected to anything, so i had to detect, wht pins were grounded instead of wht pins were taking power

still powered by usb
![first setup](https://github.com/phuid/laser-projector/blob/master/img/progress-first_setup.jpg?raw=true)
![first wiring](https://github.com/phuid/laser-projector/blob/master/img/progress-first_wiring.jpg?raw=true)
