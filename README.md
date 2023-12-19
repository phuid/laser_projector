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
- min: project ild file on exec()

#### UI
- file select from dir
  - future: fs tree
- start and stop lasershow file projection

#### web_ui
- ssh console
- file select to project
- stop projection button

#### discord_bot
idk whatever there is time for

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
lasershow exec can write to the socket immediately after the command is received or at any moment during the execution
responses immediately on command reception
- `ERROR <e> <details>`:
  - `e`: any of the following; returned if received command couldn't be parsed correctly
    - `E2BIG`: too many arguments (option read/reset command probably includes value argument)
    - `EINVAL`: invalid argument (typo in argument or project filename probably has wrong extension)
    - `ENOENT`: file doesn't exist
    - `INVALID_CMD`: probably typo in a command
  - `details`: if error type supports it, details will be included
- `SUCCESS`: returned if command was parsed correctly, lasershow will begin projecting
