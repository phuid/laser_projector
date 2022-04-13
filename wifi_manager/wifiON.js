const fs = require('fs');
const { exec } = require("child_process");
console.log('wlan up')
    exec('sudo ifconfig wlan0 up');
    var waitTill = new Date(new Date().getTime() + 4000);
    while (waitTill > new Date()) { }

console.log('wifi');
    exec('sudo systemctl stop hostapd dnsmasq');
    exec('iwconfig', (error, stdout, stderr) => {
      if (error) {
        console.log(`error:\n${error.message}\n`);
      }
      if (stderr) {
        console.log(`stderr:\n${stderr}\n`);
      }
      console.log(stdout);
      if (stdout.indexOf('ESSID:"') == -1) {
        console.log('disabling AP')
        exec('sudo systemctl disable hostapd dnsmasq', (error, stdout, stderr) => {
          if (error) {
            console.log(`error:\n${error.message}\n`);
          }
          if (stderr) {
            console.log(`stderr:\n${stderr}\n`);
          }
          console.log(stdout);
        }).on('close', () => {
          fs.writeFileSync("/etc/dhcpcd.conf", "# A sample configuration for dhcpcd.\n# See dhcpcd.conf(5) for details.>");
          console.log('reboot')
exec('sudo reboot');
        })
      }
      else {
        console.log('wifi already connected');
      }
    });
