const { execSync } = require("child_process");
const fs = require("fs");
const path = require("path");
var zmq = require("zeromq"),
  publisher = zmq.socket("pub"),
  command_receiver = zmq.socket("sub");

publisher.bindSync("tcp://*:5558");

command_receiver.bindSync("tcp://*:5559");
command_receiver.subscribe("");

var save = JSON.parse(fs.readFileSync("./wifi_setting.json"));

const iw_operation_mode = [
  "Auto",
  "Ad-Hoc",
  "Managed",
  "Master",
  "Repeater",
  "Secondary",
  "Monitor",
  "Unknown/bug",
];

function sendState() {
  try {
    var ssid = execSync("iwgetid --raw");
    console.log("wifi_ssid: " + ssid);
    publisher.send("wifi_ssid: " + ssid);
  } catch (err) {
    console.log(err);
    try {
      var ssid = "" + execSync("iw dev | grep ssid");
      console.log("ssid", ssid);
      ssid = ssid.substring(ssid.indexOf("ssid") + 5);
      console.log("wifi_ssid: " + ssid);
      publisher.send("wifi_ssid: " + ssid);
    } catch (errr) {
      console.log(errr);
    }
  }

  try {
    var mode_raw = execSync("iwgetid --mode --raw");
    publisher.send("mode_raw: " + mode_raw);
    console.log("mode_raw: " + mode_raw);
    publisher.send("mode: " + iw_operation_mode[Number(mode_raw)]);
    console.log("mode: " + iw_operation_mode[Number(mode_raw)]);
  } catch (err) {
    console.log(err);
  }

  try {
    var hostname = execSync("hostname -I");
    publisher.send("hostname: " + hostname);
    console.log("hostname: " + hostname);
  } catch (err) {
    console.log(err);
  }

  console.log("save: ", save);
  publisher.send("wifi_setting: " + save.wifi_setting.toString());
  // publisher.send("wifi_state: " + );
}

function isAPmode() {
  try {
    var out = (ssid = execSync('iw dev | grep "type AP"'));
    if (out.length > 0) {
      console.log("yup, it is indeed AP :)");
      return true;
    } else {
      console.log("noap :(");
      return false;
    }
  } catch (error) {
    console.log("errRRRRRRRRRR:", error.message);
    if (error.message.substring(0, 14) == "Command failed") {
      return false;
    } else {
      throw error;
    }
  }
}

function togglewifi(mode) {
  // var waitTill = new Date(new Date().getTime() + 100);
  // while (waitTill > new Date()) {}

  console.log(mode);
  publisher.send("wifi_setting: " + mode);
  save.wifi_setting = mode;
  console.log(JSON.stringify(save));
  fs.writeFileSync("./wifi_setting.json", JSON.stringify(save), (err) => {
    if (err) console.log("Error writing file:", err);
  });

  if (mode == "wlan-down") {
    console.log("wlan down");
    execSync("sudo ifconfig wlan0 down");
    execSync("sudo systemctl disable hostapd dnsmasq");

    execSync(
      "sudo dhcpcd -f " +
        path.join(__dirname, "../config_templates/dhcpcd.conf")
    );
  } else {
    console.log("wlan up");
    execSync("sudo ifconfig wlan0 up");
    var waitTill = new Date(new Date().getTime() + 4000);
    while (waitTill > new Date()) {}

    if (mode == "wifi") {
      //enable wifi
      console.log("wifi");
      execSync(
        "sudo systemctl disable hostapd dnsmasq",
        (error, stdout, stderr) => {
          if (error) {
            console.log(`error:\n${error.message}\n`);
          }
          if (stderr) {
            console.log(`stderr:\n${stderr}\n`);
          }
          console.log(stdout);
        }
      );

      if (isAPmode()) {
        console.log("disabling AP");
        execSync(
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
        );
        console.log("reboot");
        execSync("sudo reboot");
      } else {
        console.log("AP already disabled");
      }
    }

    if (mode == "hotspot") {
      if (isAPmode()) {
        {
          console.log("AP already started");
          //start dnsmasq and dhcpcd with custom config
          //hostapd has been started as a service by system
          execSync(
            "sudo systemctl stop dnsmasq dhcpcd",
            (error, stdout, stderr) => {
              if (error) {
                console.log(`error:\n${error.message}\n`);
              }
              if (stderr) {
                console.log(`stderr:\n${stderr}\n`);
              }
              console.log(stdout);
            }
          );
          execSync(
            "sudo dhcpcd -f " +
              path.join(__dirname, "../config_templates/dhcpcd.conf"),
            (error, stdout, stderr) => {
              if (error) {
                console.log(`error:\n${error.message}\n`);
              }
              if (stderr) {
                console.log(`stderr:\n${stderr}\n`);
              }
              console.log(stdout);
            }
          );
          try {
            execSync("sudo pkill dnsmasq");
          } catch (err) {
            console.log(err);
          }
          execSync(
            "sudo dnsmasq -C " +
              path.join(__dirname, "../config_templates/dnsmasq.conf"),
            (error, stdout, stderr) => {
              if (error) {
                console.log(`error:\n${error.message}\n`);
              }
              if (stderr) {
                console.log(`stderr:\n${stderr}\n`);
              }
              console.log(stdout);
            }
          );

          //            execSync(
          //              "sudo systemctl enable dnsmasq dhcpcd",
          //              (error, stdout, stderr) => {
          //                if (error) {
          //                  console.log(`error:\n${error.message}\n`);
          //                }
          //                if (stderr) {
          //                  console.log(`stderr:\n${stderr}\n`);
          //                }
          //                console.log(stdout);
          //              }
          //            );
        }
      } else {
        //enable AP
        console.log("ap before boot");
        // disable dnsmasq & dhcpcd to start them again after boot with custom config
        execSync(
          "sudo systemctl disable dnsmasq dhcpcd",
          (error, stdout, stderr) => {
            if (error) {
              console.log(`error:\n${error.message}\n`);
            }
            if (stderr) {
              console.log(`stderr:\n${stderr}\n`);
            }
            console.log(stdout);
          }
        );
        execSync("sudo systemctl enable hostapd", (error, stdout, stderr) => {
          //will run it myself after boot
          if (error) {
            console.log(`error:\n${error.message}\n`);
          }
          if (stderr) {
            console.log(`stderr:\n${stderr}\n`);
          }
          console.log(stdout);
        });
        console.log("reboot");
        execSync("sudo reboot");
      }
    }
  }
}

function send_help(sock) {
  sock.send(
    '__wifiman help__\n\rto read state:"read"\n\rto write setting:"write <wifi|hotspot|stealth>"'
  );
}

command_receiver.on("message", (msg) => {
  // console.log("work: %s", msg.toString());
  if (msg.length > 0) {
    const words = msg.toString().split(" ");
    console.log("words: %s", words);
    if (words.length > 0) {
      if (words[0] == "read") {
        sendState();
      } else if (words[0] == "write" && words.length > 1) {
        togglewifi(words[1]);
        sendState();
      } else {
        send_help(publisher);
      }
    } else {
      send_help(publisher);
    }
  }
});

console.log("ready");
togglewifi(save.wifi_setting);

var sendInterval = setInterval(() => {
  sendState();
}, 15000);
