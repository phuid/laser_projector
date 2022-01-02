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

then install all the node dependencies

enter every dir where a separate node server is gonna be located (**.**, **./discord_bot**, **./wifi_manager**) and intall its depencencies, then leave it

```console
npm i child_process http formidable fs path
```

```console
cd wifi_manager && npm i onoff child_process fs && cd ..
```

```console
cd discord_bot && npm i discord.js @discordjs/builders @discordjs/rest child_process path request fs && cd ..
```

### Hardware construction

this part is not gonna be added untill around Febuary 2022, since that's when my brother is gonna start working on modeling the case and when we are gonna buy the Galvos and all other components and put them together.

## Progress (everything you need is above this part)

### Software progress

### Hardware progress

for some reason my pi saw the voltage on some pins as HIGH when they werent connected to anything, so i had to detect, wht pins were grounded instead of wht pins were taking power

![first setup](https://github.com/phuid/laser-projector/blob/master/img/progress-first_setup.jpg?raw=true)

![first wiring](https://github.com/phuid/laser-projector/blob/master/img/progress-first_wiring.jpg?raw=true)
