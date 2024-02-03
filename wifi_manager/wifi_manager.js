const { exec } = require("child_process");
const config_state = require("../config.json").wifi_state;
var zmq = require("zeromq"),
  publisher = zmq.socket("pub"),
  command_receiver = zmq.socket("sub");

publisher.bindSync("tcp://*:5558");

command_receiver.bindSync("tcp://*:5559");
command_receiver.subscribe("");

const modeMap = new Map();
modeMap.set("stealth", 0);
modeMap.set("wifi", 1);
modeMap.set("hotspot", 2);

const isAPmode = new Promise(function (resolve, reject) {
  exec('iw dev | grep "type AP"', (error, stdout, stderr) => {
    if (stdout.length > 0) {
      reject("AP enabled");
    } else {
      resolve("AP disabled");
    }
  });
});

//0 - stealth, 1 - wifi, 2 - hotspot
function togglewifi(mode) {
  // var waitTill = new Date(new Date().getTime() + 100);
  // while (waitTill > new Date()) {}

  if (mode == 0) {
    console.log("wlan down");
    exec("sudo ifconfig wlan0 down");
    exec("sudo systemctl disable hostapd dnsmasq");

    exec("sudo dhcpcd -f " + path.join(__dirname, "../dhcpcd.conf"));
  } else {
    console.log("wlan up");
    exec("sudo ifconfig wlan0 up");
    var waitTill = new Date(new Date().getTime() + 4000);
    while (waitTill > new Date()) {}

    if (mode == 1) {
      //enable wifi
      console.log("wifi");
      exec("sudo systemctl disable hostapd dnsmasq");

      isAPmode
        .then(function whenOk(res) {
          console.log(res);
          console.log("AP already disabled");
        })
        .catch(function notOk(res) {
          console.log(res);
          console.log("disabling AP");
          exec(
            "sudo systemctl disable hostapd dnsmasq && sudo systemctl unmask dhcpcd && sudo systemctl enable dhcpcd",
            /*after boot restart dhcpcd with default config file in /etc/dhcpcd.conf*/
            (error, stdout, stderr) => {
              if (error) {
                console.log(`error:\n${error.message}\n`);
              }
              if (stderr) {
                console.log(`stderr:\n${stderr}\n`);
              }
              console.log(stdout);
            }
          ).on("close", () => {
            console.log("reboot");
            exec("sudo reboot");
          });
        });
    }

    if (mode == 2) {
      isAPmode
        .then(function whenOk(res) {
          console.log(res);
          //enable AP
          console.log("ap before boot");
          exec("sudo systemctl disable dnsmasq");
          exec("sudo systemctl enable hostapd", (error, stdout, stderr) => {
            //will run it myself after boot
            if (error) {
              console.log(`error:\n${error.message}\n`);
            }
            if (stderr) {
              console.log(`stderr:\n${stderr}\n`);
            }
            console.log(stdout);
          }).on("close", () => {
            console.log("reboot");
            exec("sudo reboot");
          });
        })
        .catch(function notOk(res) {
          console.log(res);
          console.log("AP already started");
          //start dnsmasq and dhcpcd with custom config
          //hostapd has been started as a service by system
          exec("sudo systemctl disable dnsmasq dhcpcd");
          exec("sudo dhcpcd -f " + path.join(__dirname, "../dhcpcd.conf"));
          exec("sudo dnsmasq -C " + path.join(__dirname, "../dnsmasq.conf"));
          //TODO: start again?
        });
    }
  }
}

function send_help(sock) {
  sock.send("__wifiman help__\n");
}

command_receiver.on("message", (msg) => {
  // console.log("work: %s", msg.toString());
  if (msg.length > 0) {
    const words = msg.toString().split(" ");
    console.log("words: %s", words);
    if (words.length > 0) {
      if (words[0] == "read") {
        console.log("sending state: %s", config_state);
        publisher.send(config_state.toString());
      } else if (words[0] == "write" && words.length > 1) {
        togglewifi(modeMap.get(words[1]));
      } else {
        send_help(publisher);
      }
    } else {
      send_help(publisher);
    }
  }
});

console.log("ready");
// togglewifi(modeMap.get(config_state));
