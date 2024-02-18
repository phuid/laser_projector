# RaspberryPi laser projector

[RaspberryPi laser projector](#raspberrypi-laser-projector)
- [RaspberryPi laser projector](#raspberrypi-laser-projector)
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

## how to install

```bash
git clone https://github.com/phuid/laser_projector.git
cd laser_projector
bash install.sh
```

## hw

### galvos
take -10 to +10V differential signal between to lines (base and inverted)

#### DAC - mcp4822
DAC controlled by the RPi creates an analog signal between 0 and 5V

#### amps - TL082
TL082 from texas instruments, each channel has one chip, each chip has two op amps.
One opamp inverts and modifies the signal based on the potenciometers (getting the inverted ILDA signal),
Second opamp inverts it again for the base ILDA signal

### laser
rgb laser module from https://www.laserlands.net/diode-laser-module/rgb-combined-white-laser-module/11010003.html
requires 8.5-12V power
takes 35kHz TTL / PWM on 3 pins

### OLED + encoder


## sw

### lasershow
- takes socket commands
- min: project ild file on exec()

### UI
- file select from dir
  - future: fs tree
- start and stop lasershow file projection
- wifi_manager comm

### web_ui
- ssh console
- file select to project
- stop projection button

### discord_bot
idk whatever there is time for

### wifi_manager
takes following socket commands
- `read`
- `write <stealth|wifi|hotspot>`
  - stealth -- wifi off
  - wifi -- connect to nearby known networks
  - hotspot -- create new network, raspberry pi becomes the access point

### communication
#### lasershow <- socket

lasershow executable takes commands **from all UI processes** through a **TCP socket**
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
  - `game_name` is any of the following ``//TODO: game names
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
