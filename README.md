# Raspberry Pi laser projector


not finished, not tested

## Table of contents

- [Tutorial](#tutorial)
  - [Hardware](#hardware)
  - [Software](#software)
    - [installing whole system from my disk image](#installing-whole-system-from-my-disk-image)
    - [installing executables from my repo source code](#installing-executables-from-my-repo-source-code)
      - [cloning my repository](#cloning-my-repository)
      - [Installing lasershow executable from made by https://github.com/tteskac for his /rpi-lasershow project](#installing-lasershow-executable-from-made-by-httpsgithubcomtteskac-for-his-rpi-lasershow-project)
  - [Software Instalation](#software-instalation)
  - [Hardware construction](#hardware-construction)
- [Progress](#progress-everything-you-need-is-above-this-part) (you dont need this, its just the story of this project)
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

- [install the executables and dependencies yourself](#installing-executables-from-my-repo-source-code)
  - objectively harder
  - needs node and npm installed (i think node comes with the normal version of RPI OS preinstalled but i was using RPI OS Lite, since i needed to fit all this into a small .img file and was using a 4GB SD card)
    - **discord.js library requires node v16.6.0 or newer and npm v8.3.0 or newer**
    - using [Node version manager](https://github.com/nvm-sh/nvm) - [install tutorial](https://github.com/nvm-sh/nvm#installing-and-updating), [usage](https://github.com/nvm-sh/nvm#usage)
      - much more flexible
      - you can choose node and npm version
    - [download and install node and npm binaries yourself](https://www.makersupplies.sg/blogs/tutorials/how-to-install-node-js-and-npm-on-the-raspberry-pi)

#### What code from this repo does - what happens when your RPI starts

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

Install lasershow executable from [rpi-lasershow](https://github.com/tteskac/rpi-lasershow) project made by [tteskac](https://github.com/tteskac)

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



### Hardware construction

this part is not gonna be added untill around Febuary 2022, since that's when my brother is gonna start working on modeling the case and when we are gonna buy the Galvos and all other components and put them together.

## Progress (everything you need is above this part)

### Software progress

### Hardware progress

for some reason my pi saw the voltage on some pins as HIGH when they werent connected to anything, so i had to detect, wht pins were grounded instead of wht pins were taking power

still powered by usb
![first setup](https://github.com/phuid/laser-projector/blob/master/img/progress-first_setup.jpg?raw=true)
![first wiring](https://github.com/phuid/laser-projector/blob/master/img/progress-first_wiring.jpg?raw=true)
