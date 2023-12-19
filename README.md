# RaspberryPi laser projector


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

#### UI

#### web_ui

#### discord_bot

#### communication

##### lasershow <- pipe

lasershow executable takes commands **from all UI processes** through an **IPC socket**
command format
`COMMAND` + ` ` + `ARG1` + ` ` + `ARG2` + ` ` + ` ` + `ARG4`
-> parse with a simple while loop lol no need for fancy functions

###### basic commands:
any process can send a command into the socket and all processes will read responses from lasershow executable
- `project` args: `<filename>`
  - `<filename>` must have extension `.ild`<!-- or `.lpc`(laserprojector_custom) --> otherwise error `INVALID_ARG`
- `stop` (no args)
- `game` args: `<game_name>`
  - `game_name` is any of the following ``//TODO: game names
- `press` (no args), only handled if game is running
- `release` (no args), only handled if game is running
- `option` args: `<option_name>` `<mode>` `<value>`
  - `mode`: any of `write`/`read`/`reset`
  - `value` only read when using `write` parameter, message won't be processed and `INVALID_CMD` error will be set back through the socket if other modes are used and value is specified

###### responses:
- `ERROR`: any of `INVALID_CMD`/`INVALID_ARG`
- 