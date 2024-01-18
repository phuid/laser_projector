#!/bin/bash

#TODO:
# * !verbose - use more echos instead of tee and condition them

SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"

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

Debug="${On_Cyan}DBG: "
Error=" ${On_IRed}!!${Color_Off} ${Red}"
Warning=" ${On_IYellow}!!${Color_Off} ${Yellow}"

cd "$(dirname "$0")"

err=""

userauth=1

while getopts 'hy' OPTION
do
    case "$OPTION" in
        h)
            echo "script usage: $(basename \$0) [-l] [-h] [-a somevalue]" >&2
            exit 1
        ;;
        y)
            echo -e "-y flag provided, skipping userauth for each command"
            userauth=0
        ;;
        ?)
            echo "script usage: $(basename \$0) [-l] [-h] [-a somevalue]" >&2
            exit 1
        ;;
    esac
done
shift "$(($OPTIND -1))"

if [ $userauth == 1 ]
then
    echo -e "${IWhite}do you wish to review every step of instalation that could change your system settings? (if you answer no the installer will complete all its steps without asking for permission)(answer q to quit)"
    while true
    do
        read -p "[y/n/q] (y): " userauth
        case $userauth in
            [Yy]* ) userauth=1; break;;
            [Nn]* ) userauth=0; break;;
            [Qq]* ) exit;;
            * ) userauth=1; break;;
        esac
    done
fi

# read -p "[y/n]: " userauth
# if [ "$userauth" != "${userauth#[Yy]}" ];
# then
#     userauth=1
# else
#     userauth=0
# fi

echo -e "userauth: $userauth"

echo -e "${On_IWhite}##${Color_Off} updating your packages ${On_IWhite}##${Color_Off}"
if [ $userauth == 1 ]
then
    echo -e "${On_IBlack}++${Color_Off} sudo apt-get update && sudo apt-get upgrade ${On_IBlack}++${Color_Off}"
    read -p "[y/n]: " currentauth
    if [ "$currentauth" != "${currentauth#[Yy]}" ];
    then
        sudo apt-get update && sudo apt-get upgrade
    fi
    unset currentauth
else
    echo -e "${On_IBlack}++${Color_Off} sudo apt-get update -y && sudo apt-get upgrade -y ${On_IBlack}++${Color_Off}"
    sudo apt-get update -y && sudo apt-get upgrade -y
fi
if which node > /dev/null
then
    echo -e "${On_IWhite}##${Color_Off} ${Yellow}node${Color_Off} is already installed, ${UWhite}skipping${Color_Off}...${Purple}(cant check version, give me a way in issues pls)${Color_Off} ${On_IWhite}##${Color_Off}"
else
    echo -e "${On_IWhite}##${Color_Off} ${UWhite}installing${Color_Off} ${Yellow}node${Color_Off}... ${On_IWhite}##${Color_Off}"
    echo -e "${On_IBlack}++${Color_Off} sudo curl -fsSL https://deb.nodesource.com/setup_16.x | sudo bash - ${On_IBlack}++${Color_Off}"
    echo -e "${On_IBlack}++${Color_Off} sudo apt-get install nodejs -y ${On_IBlack}++${Color_Off}"
    
    if [ $userauth == 1 ];
    then
        read -p "[y/n]: " currentauth
        if [ "$currentauth" != "${currentauth#[Yy]}" ];
        then
            sudo curl -fsSL https://deb.nodesource.com/setup_16.x | sudo bash -
            sudo apt-get install nodejs
        fi
        unset currentauth
    else
        sudo curl -fsSL https://deb.nodesource.com/setup_16.x | sudo bash -
        sudo apt-get install nodejs -y
    fi
fi

if which npm > /dev/null
then
    echo -e "${On_IWhite}##${Color_Off} ${Yellow}npm${Color_Off} is already installed, ${UWhite}skipping${Color_Off}... ${On_IWhite}##${Color_Off}"
else
    echo -e "${On_IWhite}##${Color_Off} ${UWhite}installing${Color_Off} ${Yellow}npm${Color_Off}... ${On_IWhite}##${Color_Off}"
    echo -e "${On_IBlack}++${Color_Off} sudo curl -fsSL https://deb.nodesource.com/setup_16.x | sudo bash - ${On_IBlack}++${Color_Off}"
    echo -e "${On_IBlack}++${Color_Off} sudo apt-get install npm -y ${On_IBlack}++${Color_Off}"
    if [ $userauth == 1 ];
    then
        read -p "[y/n]: " currentauth
        if [ "$currentauth" != "${currentauth#[Yy]}" ];
        then
            sudo curl -fsSL https://deb.nodesource.com/setup_16.x | sudo bash -
            sudo apt-get install npm
        fi
        unset currentauth
    else
        sudo curl -fsSL https://deb.nodesource.com/setup_16.x | sudo bash -
        sudo apt-get install npm -y
    fi
fi

# if which gpio > /dev/null
# then
#     echo -e "${On_IWhite}##${Color_Off} ${Yellow}wiringpi${Color_Off} is already installed, ${UWhite}skipping${Color_Off}... ${On_IWhite}##${Color_Off}"
# else
#     echo -e "${On_IWhite}##${Color_Off} ${UWhite}installing${Color_Off} ${Yellow}wiringpi${Color_Off}... ${On_IWhite}##${Color_Off}"
#     echo -e "${On_IBlack}++${Color_Off} (cd && git clone https://github.com/WiringPi/WiringPi.git && cd WiringPi && ./build && cd \"$(dirname \"$0\")\") ${On_IBlack}++${Color_Off}"
#     if [ $userauth == 1 ];
#     then
#         read -p "[y/n]: " currentauth
#         if [ "$currentauth" != "${currentauth#[Yy]}" ];
#         then
#             (cd && git clone https://github.com/WiringPi/WiringPi.git && cd WiringPi && ./build && cd "$(dirname "$0")")
#         fi
#         unset currentauth
#     else
#             (cd && git clone https://github.com/WiringPi/WiringPi.git && cd WiringPi && ./build && cd "$(dirname "$0")")
#     fi
# fi
##pigpio
wget https://github.com/joan2937/pigpio/archive/master.zip
unzip master.zip
cd pigpio-master
make
sudo make install

if which dnsmasq > /dev/null
then
    echo -e "${On_IWhite}##${Color_Off} ${Yellow}dnsmasq${Color_Off} is already installed, ${UWhite}skipping${Color_Off}... ${On_IWhite}##${Color_Off}"
else
    echo -e "${On_IWhite}##${Color_Off} ${UWhite}installing${Color_Off} ${Yellow}dnsmasq${Color_Off}... ${On_IWhite}##${Color_Off}"
    echo -e "${On_IBlack}++${Color_Off} sudo apt-get install dnsmasq -y ${On_IBlack}++${Color_Off}"
    if [ $userauth == 1 ];
    then
        read -p "[y/n]: " currentauth
        if [ "$currentauth" != "${currentauth#[Yy]}" ];
        then
            sudo apt-get install dnsmasq
        fi
        unset currentauth
    else
        sudo apt-get install dnsmasq -y
    fi
fi

if which hostapd > /dev/null
then
    echo -e "${On_IWhite}##${Color_Off} ${Yellow}hostapd${Color_Off} is already installed, ${UWhite}skipping${Color_Off}... ${On_IWhite}##${Color_Off}"
else
    echo -e "${On_IWhite}##${Color_Off} ${UWhite}installing${Color_Off} ${Yellow}hostapd${Color_Off}... ${On_IWhite}##${Color_Off}"
    echo -e "${On_IBlack}++${Color_Off} sudo apt-get install hostapd -y ${On_IBlack}++${Color_Off}"
    if [ $userauth == 1 ];
    then
        read -p "[y/n]: " currentauth
        if [ "$currentauth" != "${currentauth#[Yy]}" ];
        then
            sudo apt-get install hostapd
        fi
        unset currentauth
    else
        sudo apt-get install hostapd -y
    fi
fi

echo -e "${On_IWhite}##${Color_Off} configuring AP ${On_IWhite}##${Color_Off}"

# echo -e "${On_IWhite}###${Color_Off} diverting ${Yellow}dhcpcd${Color_Off} config to custom file ${On_IWhite}##${Color_Off}"
# echo -e "${On_IBlack}++${Color_Off} dhcpcd -f \"$(dirname $0)/config_templates/dhcpcd.conf\" ${On_IBlack}++${Color_Off}"
# dhcpcd -f "$(dirname $0)/config_templates/dhcpcd.conf"
# echo -e "${On_IBlack}++${Color_Off} sudo systemctl reload dhcpcd ${On_IBlack}++${Color_Off}"
# sudo systemctl reload dhcpcd

# echo -e "${On_IWhite}###${Color_Off} diverting ${Yellow}dnsmasq${Color_Off} config to custom file ${On_IWhite}##${Color_Off}"
# echo -e "${On_IBlack}++${Color_Off} dnsmasq -C \"$(dirname $0)/config_templates/dnsmasq.conf\" ${On_IBlack}++${Color_Off}"
# dnsmasq -C "$(dirname $0)/config_templates/dnsmasq.conf"
# echo -e "${On_IBlack}++${Color_Off} sudo systemctl reload dnsmasq ${On_IBlack}++${Color_Off}"
# sudo systemctl reload dnsmasq

#locate hostapd.service

#TODO: fix hostapd unit is masked when aquiring location
sudo systemctl unmask hostapd
sudo systemctl enable hostapd
sudo systemctl start hostapd

loc=$(systemctl cat hostapd | head -n 1)
loc=${loc:2}
hostapdstopped=0

echo -e "\n${On_IWhite}##${Color_Off} stop hostapd and set up its config path ${On_IWhite}##${Color_Off}"

echo -e "${On_IBlack}++${Color_Off} sudo systemctl stop hostapd ${On_IBlack}++${Color_Off}"
if [ $userauth == 1 ];
then
    read -p "[y/n]: " currentauth
    if [ "$currentauth" != "${currentauth#[Yy]}" ];
    then
        sudo systemctl stop hostapd
        hostapdstopped=1
    fi
    unset currentauth
else
    sudo systemctl stop hostapd
    hostapdstopped=1
fi
echo -e "${Debug}stoppd: $hostapdstopped${Color_Off}"

if [ $hostapdstopped == 1 ];
then
    
    echo -e "${On_IWhite}##${Color_Off} changing config path in $loc ${On_IWhite}##${Color_Off}"
    
    out=""
    prev=""
    while read line
    do
        if [[ "$line" == "Environment=DAEMON_CONF="* ]]
        then
            prev="$line"
            out="${out}Environment=DAEMON_CONF=$SCRIPTPATH/config_templates/hostapd.conf\n"
        else
            out="${out}${line}\n"
        fi
    done < $loc
    out=${out%??} #remove last \n
    
    echo -e "${On_IWhite}#+${Color_Off} replace line ${UWhite}$prev${Color_Off} in file ${Yellow}$loc${Color_Off} with ${UWhite}Environment=DAEMON_CONF=$SCRIPTPATH/config_templates/hostapd.conf${Color_Off} ${On_IWhite}+#${Color_Off}"
    
    if [ $userauth == 1 ];
    then
        read -p "[y/n]: " currentauth
        if [ "$currentauth" != "${currentauth#[Yy]}" ];
        then
            echo -e "$out" | sudo tee "$loc" #TODO: replace with echo if it works
        fi
        unset currentauth
    else
        echo -e "$out" | sudo tee "$loc" #TODO: replace with echo if it works
    fi
    
    echo -e "${On_IBlack}++${Color_Off} sudo systemctl reload hostapd ${On_IBlack}++${Color_Off}"
    if [ $userauth == 1 ];
    then
        read -p "[y/n]: " currentauth
        if [ "$currentauth" != "${currentauth#[Yy]}" ];
        then
            sudo systemctl reload hostapd
        fi
        unset currentauth
    else
        sudo systemctl reload hostapd
    fi
else
    echo -e "${Error}skipped setting hostapd.conf path ${Error}${Color_Off}"
    err="${err}${Error}skipped setting hostapd.conf path - user input ${Error}${Color_Off}\n"
fi

LineFound=0
out=""
prev=""
loc="/boot/config.txt"
while read line
do
    if [[ "$line" == *dtparam\=spi\=* ]]
    then
        prev="$line"
        out="${out}dtparam=spi=on\n"
        LineFound=1
        echo -e "${Debug} found dtparam line: ${Color_Off}${UCyan}\"$line\"${Color_Off}"
    else
        out="${out}${line}\n"
    fi
done < "$loc"
out=${out%??} #remove last \n

if [[ $LineFound == 1 ]]
then
    echo -e "${On_IWhite}#+${Color_Off} replace line ${UWhite}$prev${Color_Off} in file ${Yellow}$loc${Color_Off} with ${UWhite}dtparam=spi=on${Color_Off} ${On_IWhite}+#${Color_Off}"
    echo -e "$out" | sudo tee "$loc"
else
    echo -e "${On_IWhite}#+${Color_Off} appending ${UWhite}dtparam=spi=on${Color_Off} to file ${Yellow}$loc${Color_Off}${On_IWhite}+#${Color_Off}"
    echo -e "dtparam=spi=on" | sudo tee -a "$loc"
fi

#TODO: userauth isntalling zeromq
echo -e "${On_IWhite}##${Color_Off} ${UWhite}installing${Color_Off} ZeroMQ library ${On_IWhite}##${Color_Off}"

sudo apt-get install cmake -y

cd ~

# https://gist.github.com/steinwaywhw/a4cd19cda655b8249d908261a62687f8 (onliner do download latest release)
# libzmq
url=$(curl -s https://api.github.com/repos/zeromq/libzmq/releases/latest | grep "browser_download_url.*tar.gz" | cut -d : -f 2,3 | tr -d \")
wget $url -O - | tar -zxvf -
foldername="zeromq-${url#*zeromq-}"
foldername="${foldername%.tar.gz}"
(cd $foldername 
mkdir build
cd build
cmake ..
sudo make -j4 install
)
# cppzmq
version=$(curl -s https://api.github.com/repos/zeromq/cppzmq/releases/latest | grep "\"tag_name\": \"v*" | cut -d : -f 2,3 | tr -d \")
version=${version:2:-1}
url="https://github.com/zeromq/cppzmq/archive/refs/tags/v${version}.tar.gz"
wget $url -O - | tar -zxvf -
foldername="cppzmq-${version}"
cd $foldername
(cd $foldername 
mkdir build
cd build
cmake ..
sudo make -j4 install
)


echo -e "${On_IWhite}##${Color_Off} ${UWhite}compiling${Color_Off} lasershow executable... ${On_IWhite}##${Color_Off}"
echo -e "${On_IBlack}++${Color_Off} (cd rpi-lasershow && make) ${On_IBlack}++${Color_Off}"
# TODO: copy exec to root folder
if !(out="$(cd rpi-lasershow && make)")
then
    echo -e "${Error}compilation resulted in an error${Error}${Color_Off}\ndetails:"
    echo "$out"
    err="${err}${Error}lasershow executable compilation resulted in an error${Error}${Color_Off}\ndetails:${out}\n"
else
    echo -e "${On_IWhite}##${Color_Off}                               ...${Green}success${Color_Off} ${On_IWhite}##${Color_Off}"
    echo -e "${On_IWhite}##${Color_Off} cp rpi-lasershow/lasershow . ${On_IWhite}##${Color_Off}"
    cp rpi-lasershow/lasershow .
fi

if which pm2 > /dev/null
then
    echo -e "${On_IWhite}##${Color_Off} ${Yellow}pm2${Color_Off} is already installed, ${UWhite}skipping${Color_Off}... ${On_IWhite}##${Color_Off}"
else
    echo -e "${On_IWhite}##${Color_Off} ${UWhite}installing${Color_Off} ${Yellow}pm2${Color_Off}... ${On_IWhite}##${Color_Off}"
    echo -e "${On_IBlack}++${Color_Off} sudo npm install pm2 -g ${On_IBlack}++${Color_Off}"
    sudo npm install pm2 -g
fi

echo -e "${URed}pm2 -y confirm todo${Color_Off}"
echo -e "${URed}pm2 -y confirm todo${Color_Off}"
echo -e "${URed}pm2 -y confirm todo${Color_Off}"
echo -e "${URed}pm2 -y confirm todo${Color_Off}"
echo -e "${URed}pm2 -y confirm todo${Color_Off}"
echo -e "${URed}pm2 -y confirm todo${Color_Off}"
echo -e "${URed}pm2 -y confirm todo${Color_Off}\n\n\n"

echo -e "${URed}CONFIG TODO${Color_Off}"
echo -e "${URed}CONFIG TODO${Color_Off}"
echo -e "${URed}CONFIG TODO${Color_Off}"
echo -e "${URed}CONFIG TODO${Color_Off}"
echo -e "${URed}CONFIG TODO${Color_Off}"
echo -e "${URed}CONFIG TODO${Color_Off}"
echo -e "${URed}CONFIG TODO${Color_Off}"
echo -e "${URed}CONFIG TODO${Color_Off}"

echo ""

#npm dependencies
for i in {"discord_bot","web_ui","wifi_manager"}
do
    echo -e "${On_IWhite}##${Color_Off} ${UWhite}installing${Color_Off} dependencies for ${Yellow}${i}${Color_Off}... ${On_IWhite}##${Color_Off}"
    echo -e "${On_IBlack}++${Color_Off} (cd $i && npm install) ${On_IBlack}++${Color_Off}"
    (cd $i && npm install && echo -e "${On_IBlack}++${Color_Off} npm audit fix ${On_IBlack}++${Color_Off}" && npm audit fix)
    
    echo -e "${On_IWhite}##${Color_Off} ${UWhite}deploying${Color_Off} ${Yellow}$i${Color_Off} to pm2... ${On_IWhite}##${Color_Off}"
    echo -e "${On_IBlack}++${Color_Off} (cd $i && pm2 start . --name $i --restart-delay=5000) ${On_IBlack}++${Color_Off}"
    (cd $i && pm2 start . --name $i --restart-delay=5000)
done

echo -e "${On_IWhite}##${Color_Off} ${UWhite}deploying discord commands${Color_Off}... ${On_IWhite}##${Color_Off}"
echo -e "${On_IBlack}++${Color_Off} (cd discord_bot && node deploy_commands.js) ${On_IBlack}++${Color_Off}"
(cd discord_bot && node deploy_commands.js)

echo -e "${On_IWhite}##${Color_Off} ${UWhite}saving pm2 configuration${Color_Off}... ${On_IWhite}##${Color_Off}"
echo -e "${On_IBlack}++${Color_Off} pm2 save && sudo env PATH=$PATH:/usr/local/bin pm2 startup systemd -u pi --hp /home/pi && pm2 restart all ${On_IBlack}++${Color_Off}"
pm2 save && sudo env PATH=$PATH:/usr/local/bin pm2 startup systemd -u pi --hp /home/pi && pm2 restart all

echo -e "${Red}NEED COMMENTS${Color_Off}" #TODO COPY FILE FROM ./config_templates???
echo -e "[Unit]\nDescription=Service to redirect all pm2 logs to /dev/tty1 - RPI HDMI output\nAfter=pm2-pi.service\n[Service]\nUser=pi\nType=simple\nStandardOutput=tty\nTTYPath=/dev/tty1\nTimeoutStartSec=5\nEnvironment=PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/local/games:/usr/games:/usr/local/bin:/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin\nEnvironment=PM2_HOME=/home/pi/.pm2\nRestart=on-failure\nExecStart=/bin/bash -c 'echo \$\$\$\$ > /tmp/pm2LogsToTty1.pid;exec pm2 logs'\nExecStop=/bin/bash -c 'sudo kill \$(cat /tmp/pm2LogsToTty1.pid)'\n[Install]\nWantedBy=default.target\n" | sudo tee /etc/systemd/system/pm2LogsToTty1.service
sudo systemctl daemon-reload
sudo systemctl enable pm2LogsToTty1 # only tested with start

echo -e -n "installation finished "
if [ "$err" == "" ];
then
    echo -e "${Green}without errors${Color_Off}"
else
    echo -e "${UYellow}with following warnings/${URed}errors:${Color_Off}\n$err"
fi

echo ""
echo -e "${On_IWhite}##${Color_Off} ${UYellow}reboot${Color_Off} is required to ${Green}finish instalation${Color_Off}\n${UWhite}do you want to reboot system now?${Color_Off} [Y/n] ${On_IWhite}##${Color_Off}"

while true
    do
        read -p "[y/n]: " reb
        case $reb in
            [Yy]* ) echo -e "${On_IBlack}++${Color_Off} sudo reboot ${On_IBlack}++${Color_Off}";sudo reboot;break;;
            [Nn]* ) echo "exiting without reboot"; break;;
            * ) echo "please answer y/n";;
        esac
    done

