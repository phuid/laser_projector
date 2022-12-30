var Gpio = require("onoff").Gpio; //include onoff to interact with the GPIO
const { exec } = require("child_process");
const fs = require("fs");
const path = require("path");
const { exit } = require("process");

function getLastChangedFile() {
  var lasttime = 0;
  var lastpath = "";

  var fs = require("fs");
  const ildFiles = fs
    .readdirSync("../ild/")
    .filter((file) => file.endsWith(".ild"));

  ildFiles.forEach(function (file) {
    // console.log(path.join(__dirname + '/ild/', file));
    stats = fs.statSync(path.join(__dirname, "../ild/" + file), true);
    if (stats.mtimeMs > lasttime) {
      lasttime = stats.mtimeMs;
      lastpath = path.join(__dirname + "/ild/", file);
    }
  });
  return lastpath;
}

const isAPmode = new Promise(function (resolve, reject) {
  exec('iw dev | grep "type AP"', (error, stdout, stderr) => {
    if (stdout.length > 0) {
      reject("AP enabled");
    } else {
      resolve("AP disabled");
    }
  });
});

var failsafe = new Gpio(4, "in");
if (failsafe.readSync()) {
  console.log(
    "failsafe check didn't pass (pin4 not connected to ground) -> wifi manager exiting"
  );
  exit(0);
} else {
  console.log("headers connected -> wifi manager starting");
}

var APButton = new Gpio(6, "in", "both");
var wifiButton = new Gpio(5, "in", "both");
var sdButton = new Gpio(26, "in", "both"); //shutdown

function togglewifi(err) {
  if (err) {
    console.error(err);
  }

  var waitTill = new Date(new Date().getTime() + 100);
  while (waitTill > new Date()) {}

  console.log(
    `APButton: ${APButton.readSync()}\nwifiButton: ${wifiButton.readSync()}`
  );

  if (APButton.readSync() && wifiButton.readSync()) {
    console.log("wlan down");
    exec("sudo ifconfig wlan0 down");
    exec("sudo systemctl disable hostapd dnsmasq");

    exec("sudo dhcpcd -f " + path.join(__dirname, "../dhcpcd.conf"));

    console.log("projecting last modified file");
    exec(
      path.join(__dirname, "../lasershow") + " 0 " + getLastChangedFile(),
      (error, stdout, stderr) => {
        if (error) {
          console.log(`error:\n${error.message}\n`);
        }
        if (stderr) {
          console.log(`stderr:\n${stderr}\n`);
        }
        console.log(`${stdout}`);
      }
    );
  } else {
    console.log("wlan up");
    exec("sudo ifconfig wlan0 up");
    var waitTill = new Date(new Date().getTime() + 4000);
    while (waitTill > new Date()) {}
  }

  if (!APButton.readSync()) {
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
        exec("sudo dhcpcd -f " + path.join(__dirname, "../dhcpcd.conf")) 
        exec("sudo dnsmasq -C " + path.join(__dirname, "../dnsmasq.conf"));
      });
  }

  if (!wifiButton.readSync()) {
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
}

APButton.watch((err, value) => togglewifi(err));
wifiButton.watch((err, value) => togglewifi(err));

sdButton.watch(function (err, value) {
  //Watch for hardware interrupts on pushButton GPIO, specify callback function
  if (err) {
    //if an error
    console.error("There was an error", err); //output error message to console
    return;
  }
  if (value) {
    console.log("shutdown");
    exec("sudo shutdown now");
  }
});

function unexportOnClose() {
  //function to run when exiting program
  APButton.unexport(); // Unexport Button GPIO to free resources
  wifiButton.unexport(); // Unexport Button GPIO to free resources
  sdButton.unexport(); // Unexport Button GPIO to free resources
  console.log("SIGINT detected - exiting");
}

failsafe.watch((err, value) => {
  if (value) {
    console.log("failsafe unpluged -> exiting");
    unexportOnClose();
  }
});

process.on("SIGINT", unexportOnClose); //function to run when user closes using ctrl+c

togglewifi(0);
