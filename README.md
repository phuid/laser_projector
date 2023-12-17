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

lasershow executable takes commands through a pipe
command format
`COMMAND` + `ARG1` + `" "` + `ARG2` + `" "` + `" "` + `ARG4`
-> parse with a simple while loop lol no need for fancy functions

basic commands:
- `project` args: file; file must have extension `.ild` or `.lpc`(laserprojector_custom)
- `stop` (no args)
- `game` args: game_name
- `press` (no args), only handled if game is running
- `release` (no args), only handled if game is running
