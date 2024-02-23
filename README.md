# Raspberry Pi RGB laser projector
A vector laser projector with an easy to understand UI on local network, on an integrated display and on discord.

## table of contents
- [thesis](#thesis)
- [how to install](#how-to-install)
- [hw](#hw)
  - [galvos](#galvos)
    - [DAC - mcp4822](#dac---mcp4822)
    - [amps - TL082](#amps---tl082)
  - [laser](#laser)
  - [OLED + encoder](#oled--encoder)
- [sw](#sw)
  - [lasershow](#lasershow)
  - [UI](#ui)
  - [web\_ui](#web_ui)
  - [discord\_bot](#discord_bot)
  - [wifi\_manager](#wifi_manager)
  - [communication](#communication)
    - [lasershow \<- socket](#lasershow---socket)
      - [basic commands:](#basic-commands)
      - [responses:](#responses)

## thesis
I wrote a whole thesisi in [czech](https://en.wikipedia.org/wiki/Czech_Republic) language about this project. It is available in [this repository](https://github.com/phuid/laser_projector-thesis) [(direct link to the pdf file)](https://github.com/phuid/laser_projector-thesis/blob/master/text.pdf).

## how to install

```bash
git clone https://github.com/phuid/laser_projector.git
cd laser_projector
bash install.sh
```

## supported projection file formats
The projector can read and project **.ild** and (much more popular) **.svg** file formats.
**.ild** files can be generated using specialised software like [laserworld showeditor](https://www.showeditor.com/en).
**.svg** files from any editor are internally converted to **.ild** using [this svg2ild.py script](https://github.com/marcan/openlase/blob/master/tools/svg2ild.py) from [gh/marcan/openlase](https://github.com/marcan/openlase)

## hw

### galvos
take -10 to +10V differential signal between to lines (base and inverted),
The DAC + opamps circuit which generates this signal is nicely described in [this article](https://www.instructables.com/Arduino-Laser-Show-With-Real-Galvos/).
My implemetation of it in kicad files can be found in [pcb/kicad].

#### DAC - mcp4822
DAC controlled by the RPi creates an analog signal between 0 and 5V.

#### amps - TL082
TL082 from texas instruments, each channel has one chip, each chip has two op amps.
One opamp inverts and modifies the signal based on the potenciometers (getting the inverted ILDA signal),
Second opamp inverts it again for the base ILDA signal

### laser
rgb laser module from https://www.laserlands.net/diode-laser-module/rgb-combined-white-laser-module/11010003.html
requires 8.5-12V power
takes 35kHz TTL / PWM on 3 pins, these pins are connected directly to the raspberry pi

pin connections between the laser module and RPi
|LASER|RPi|
|---|---|
|red|GPIO22|
|green|GPIO27|
|blue|GPIO17|

### LCD + encoder
Any 20x4 LCD with an I2C backpack + any rotary encoder with a button 
LCD connections:
|LCD|RPi|
|---|---|
|Vcc|5V|
|GND|GND|
|SCK|GPIO3|
|SDA|GPIO2|
|backlight|GPIO18|

encoder connections:
|encoder|RPi|
|---|---|
|A|GPIO5|
|common|GND|
|B|GPIO6|
|button1|GND|
|button1|GPIO13|

## sw
2 backend programs (lasershow *(c++)* + wifi_manager *(node.js)*) and 3 frontend programs (UI *(c++)* + web_ui *(node.js)* + discord_bot *(node.js)*)

### communication
each backend has its pub/sub sockets
lasershow publishes to `tcp://localhost:5556` and receives commands from `tcp://localhost:5556`
wifi_manager publishes to `tcp://localhost:5558` and receives commands from `tcp://localhost:5559`

### lasershow
- backend program that takes care of the actual projecting through controlling the galvos and the laser module

### UI
- allows control of backend programs through the built-in LCD and encoder

### web_ui
- allows control of backend programs through a web interface available on the local network

### discord_bot
- allows control of backend programs through a discord bot

### wifi_manager
- allows switching between three modes of wifi:
  - wifi off (`stealth`)
  - wifi on (`wifi`) -- RPi wifi on and trying to connect to known nearby networks
  - access point (`hotspot`) -- RPi wifi on transmitting its own wifi which other devices can connect to

### comms format
#### lasershow comms

lasershow executable takes commands **from all UI processes** through a **ZeroMQ TCP socket**
command format
`COMMAND` + ` ` + `ARG1` + ` ` + `ARG2` + ` ` + ` ` + `ARG4`
-> parse with a simple while loop lol no need for fancy functions

##### basic commands:
any process can send a command into the socket and all processes will read responses from lasershow executable
- `PROJECT` args: `<filename>`
  - `<filename>` must have extension `.ild`<!-- or `.lpc`(laserprojector_custom) --> otherwise error `INVALID_ARG`
- `STOP` (no args)
- `PAUSE` (no args)
- `GAME` args: `<game_name>`
  - `game_name` is any of the following ``//TODO: game names (not yes implemented)
- `PRESS` (no args), only handled if game is running
- `RELEASE` (no args), only handled if game is running
- `OPTION` args: `<mode>` `<option_name>` `<value>`
  - `mode`: any of `write`/`read`/`reset`
  - `option_name`: any of the following
    - `progress`
    - `current_frame` - if a projection is active, this will seek to the defined frame of the projection, must not exceed number_of_frames (every pos info gives this argument)
    - `point_delay`
    - `target_frame_time`
    - `repeat`
    - `trapeziod_horizontal`
    - `trapeziod_vertical`
  - `value` only read when using `write` parameter

##### responses:
lasershow exec can write to the socket at any time
- `ERROR: <e> <details>`:
  - `e`: any of the following; returned if received command couldn't be parsed correctly
    - `E2BIG`: too many arguments (option read/reset command probably includes value argument)
    - `EINVAL`: invalid argument (typo in argument or project filename probably has wrong extension)
    - `ENOENT`: file doesn't exist
    - `INVALID_CMD`: probably typo in a command
    - `OTHER`
  - `details`: if error type supports it, details will be included
- there is no success message
- `INFO: <details>`
  - details - one of following:
    - `OPTION <name> <val>`
      - name: name of option being changed; *any of*:
        - `point_delay`
        - `repeat`
        - `target_frame_time`
        - `trapezoid_horizontal`
        - `trapezoid_vertical`
      - val: numeric (float) value of the option
    - `STOP`
    - `PAUSE <val>`
      - val: 1 - paused, 0 - not paused
    - `PROJECT <filename>`
      - filename: filename of the file being projected
    - `POS <pos> OF <max>`
      - pos: position of coursor in projected file
      - max: size of projected file
- `DISPLAY: <text>`: client shall display text to the user and hide it when a new command comes or when user sees it
- `ALERT: <text>`: client shall display text to the user and only hide it and process following commands after user sees the alert

#### wifi_manager comms
takes following commands
- `read`: prints out following parameters:
  - `wifi_setting: <stealth|wifi|hotspot>`
  - `wifi_ssid: <SSID>`: SSID as returned by `iwgetid --raw` or `iw dev` in case hotspot is active
  - `mode_raw: <MODE_RAW>`: MODE_RAW (0-7) as returned by `iwgetid --mode --raw`
  - `mode: <MODE>`: MODE based on MODE_RAW; possible outputs: `Auto|Ad-Hoc|Managed|Master|Repeater|Secondary|Monitor|Unknown/bug`

- `write <stealth|wifi|hotspot>`
  - `stealth`: wifi off
  - `wifi`: connect to nearby known networks
  - `hotspot`: create new network, raspberry pi becomes the access point
