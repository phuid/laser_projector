#!/bin/bash

# Reset
Color_Off='\033[0m'       # Text Reset

# Regular Colors
Black='\033[0;30m'        # Black
Red='\033[0;31m'          # Red
Green='\033[0;32m'        # Green
Yellow='\033[0;33m'       # Yellow
Blue='\033[0;34m'         # Blue
Purple='\033[0;35m'       # Purple
Cyan='\033[0;36m'         # Cyan
White='\033[0;37m'        # White

# Bold
BBlack='\033[1;30m'       # Black
BRed='\033[1;31m'         # Red
BGreen='\033[1;32m'       # Green
BYellow='\033[1;33m'      # Yellow
BBlue='\033[1;34m'        # Blue
BPurple='\033[1;35m'      # Purple
BCyan='\033[1;36m'        # Cyan
BWhite='\033[1;37m'       # White

# Underline
UBlack='\033[4;30m'       # Black
URed='\033[4;31m'         # Red
UGreen='\033[4;32m'       # Green
UYellow='\033[4;33m'      # Yellow
UBlue='\033[4;34m'        # Blue
UPurple='\033[4;35m'      # Purple
UCyan='\033[4;36m'        # Cyan
UWhite='\033[4;37m'       # White

# Background
On_Black='\033[40m'       # Black
On_Red='\033[41m'         # Red
On_Green='\033[42m'       # Green
On_Yellow='\033[43m'      # Yellow
On_Blue='\033[44m'        # Blue
On_Purple='\033[45m'      # Purple
On_Cyan='\033[46m'        # Cyan
On_White='\033[47m'       # White

# High Intensity
IBlack='\033[0;90m'       # Black
IRed='\033[0;91m'         # Red
IGreen='\033[0;92m'       # Green
IYellow='\033[0;93m'      # Yellow
IBlue='\033[0;94m'        # Blue
IPurple='\033[0;95m'      # Purple
ICyan='\033[0;96m'        # Cyan
IWhite='\033[0;97m'       # White

# Bold High Intensity
BIBlack='\033[1;90m'      # Black
BIRed='\033[1;91m'        # Red
BIGreen='\033[1;92m'      # Green
BIYellow='\033[1;93m'     # Yellow
BIBlue='\033[1;94m'       # Blue
BIPurple='\033[1;95m'     # Purple
BICyan='\033[1;96m'       # Cyan
BIWhite='\033[1;97m'      # White

# High Intensity backgrounds
On_IBlack='\033[0;100m'   # Black
On_IRed='\033[0;101m'     # Red
On_IGreen='\033[0;102m'   # Green
On_IYellow='\033[0;103m'  # Yellow
On_IBlue='\033[0;104m'    # Blue
On_IPurple='\033[0;105m'  # Purple
On_ICyan='\033[0;106m'    # Cyan
On_IWhite='\033[0;107m'   # White

cd "$(dirname "$0")"

if which node > /dev/null
then
  echo -e "${BIWhite}##${Color_Off} ${Yellow}node${Color_Off} is already installed, ${UWhite}skipping${Color_Off}...${Purple}(cant check version, give me a way in issues pls)${Color_Off}"
else
  echo -e "${BIWhite}##${Color_Off} ${UWhite}installing${Color_Off} ${Yellow}node${Color_Off}..."
  sudo curl -fsSL https://deb.nodesource.com/setup_16.x | sudo bash -
  echo -e "${BIWhite}++${Color_Off} sudo curl -fsSL https://deb.nodesource.com/setup_16.x | sudo bash -"
  sudo apt-get install nodejs -y
  echo -e "${BIWhite}++${Color_Off} sudo apt-get install nodejs -y"
fi

if which npm > /dev/null
then
  echo -e "${BIWhite}##${Color_Off} ${Yellow}npm${Color_Off} is already installed, ${UWhite}skipping${Color_Off}..."
else
  echo -e "${BIWhite}##${Color_Off} ${UWhite}installing${Color_Off} ${Yellow}npm${Color_Off}..."
  echo -e "${BIWhite}++${Color_Off} sudo curl -fsSL https://deb.nodesource.com/setup_16.x | sudo bash -"
  sudo curl -fsSL https://deb.nodesource.com/setup_16.x | sudo bash -
  echo -e "${BIWhite}++${Color_Off} sudo apt-get install npm -y"
  sudo apt-get install npm -y
fi

if which gpio > /dev/null
then
  echo -e "${BIWhite}##${Color_Off} ${Yellow}wiringpi${Color_Off} is already installed, ${UWhite}skipping${Color_Off}..."
else
  echo -e "${BIWhite}##${Color_Off} ${UWhite}installing${Color_Off} ${Yellow}wiringpi${Color_Off}..."
  echo -e "${BIWhite}++${Color_Off} (cd /tmp && wget https://project-downloads.drogon.net/wiringpi-latest.deb && sudo dpkg -i wiringpi-latest.deb)"
  (cd /tmp && wget https://project-downloads.drogon.net/wiringpi-latest.deb && sudo dpkg -i wiringpi-latest.deb)
fi

echo -e "${BIWhite}##${Color_Off} ${UWhite}compiling${Color_Off} lasershow executable..."
echo -e "${BIWhite}++${Color_Off} (cd rpi-lasershow && make)"
if !(out=$(cd rpi-lasershow && make))
then
  echo $out
  echo -e "${BIWhite}##${Color_Off} ${Red}...compilation failed..exiting${Color_Off}"
  exit 1
else
  echo -e "${BIWhite}##${Color_Off}                               ${Green}...success${Color_Off}"
fi

if which pm2 > /dev/null
then
  echo -e "${BIWhite}##${Color_Off} ${Yellow}pm2${Color_Off} is already installed, ${UWhite}skipping${Color_Off}..."
else
  echo -e "${BIWhite}##${Color_Off} ${UWhite}installing${Color_Off} ${Yellow}pm2${Color_Off}..."
  echo -e "${BIWhite}++${Color_Off} sudo npm install pm2 -g"
  sudo npm install pm2 -g
fi

echo -e "${URed}CONFIG TODO${Color_Off}"
echo -e "${URed}CONFIG TODO${Color_Off}"
echo -e "${URed}CONFIG TODO${Color_Off}"
echo -e "${URed}CONFIG TODO${Color_Off}"
echo -e "${URed}CONFIG TODO${Color_Off}"
echo -e "${URed}CONFIG TODO${Color_Off}"
echo -e "${URed}CONFIG TODO${Color_Off}"
echo -e "${URed}CONFIG TODO${Color_Off}"

#npm dependencies
for i in {"discord_bot","web_ui","wifi_manager"}
do
  echo -e "${BIWhite}##${Color_Off} ${UWhite}installing${Color_Off} dependencies for ${Yellow}${i}${Color_Off}..."
  echo -e "${BIWhite}++${Color_Off} (cd $i && npm install)"
  (cd $i && npm install && echo -e "${BIWhite}++${Color_Off} npm audit fix" && npm audit fix)

  echo -e "${BIWhite}##${Color_Off} ${UWhite}deploying${Color_Off} ${Yellow}$i${Color_Off} to pm2..."
  echo -e "${BIWhite}++${Color_Off} (cd $i && pm2 start . --name $i --restart-delay=5000)"
  (cd $i && pm2 start . --name $i --restart-delay=5000)
done

echo -e "${BIWhite}##${Color_Off} ${UWhite}deploying discord commands${Color_Off}..."
echo -e "${BIWhite}++${Color_Off} (cd discord_bot && node deploy_commands.js)"
(cd discord_bot && node deploy_commands.js)

echo -e "${BIWhite}##${Color_Off} ${UWhite}saving pm2 configuration${Color_Off}..."
echo -e "${BIWhite}++${Color_Off} pm2 save && sudo env PATH=$PATH:/usr/local/bin pm2 startup systemd -u pi --hp /home/pi && pm2 restart all"
pm2 save && sudo env PATH=$PATH:/usr/local/bin pm2 startup systemd -u pi --hp /home/pi && pm2 restart all

echo -e "${BIWhite}##${Color_Off} ${Green}instalation finished${Color_Off}"
