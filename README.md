# RaspberryPi laser projector

## install
TODO: add sources to install script
[zeromq install](https://github.com/MonsieurV/ZeroMQ-RPi)



## how it works

### hw

#### galvos

##### mcp4822

##### amps

#### laser

##### 3ch-DAC / TTL

#### OLED + encoder


### sw

#### lasershow
- takes socket commands
- min: project ild file on exec()

#### UI
- file select from dir
  - future: fs tree
- start and stop lasershow file projection
- wifi_manager comm

#### web_ui
- ssh console
- file select to project
- stop projection button

#### discord_bot
idk whatever there is time for

#### wifi_manager
- takes socket commands
- AP / wifi / wifi_off

#### communication
##### lasershow <- socket

lasershow executable takes commands **from all UI processes** through an **IPC socket**
command format
`COMMAND` + ` ` + `ARG1` + ` ` + `ARG2` + ` ` + ` ` + `ARG4`
-> parse with a simple while loop lol no need for fancy functions

###### basic commands:
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
  - `value` only read when using `write` parameter, message won't be processed and `INVALID_CMD` error will be set back through the socket if other modes are used and value is specified

###### responses:
lasershow exec can write to the socket at any time
- `ERR: <e> <details>`:
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
