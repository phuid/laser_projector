var Gpio = require('onoff').Gpio; //include onoff to interact with the GPIO
const { exec } = require("child_process");
const fs = require('fs');

var APButton = new Gpio(6, 'in', 'both');
var wifiButton = new Gpio(5, 'in', 'both');
var sdButton = new Gpio(26, 'in', 'both'); //shutdown

function togglewifi() {
  var waitTill = new Date(new Date().getTime() + 100);
  while (waitTill > new Date()) { }

  console.log('AP=' + (1 - APButton.readSync()) + ' | wifi=' + (1 - wifiButton.readSync()));

  if (APButton.readSync() && wifiButton.readSync()) {
    console.log('wlan down')
    exec('sudo ifconfig wlan0 down');
    exec('sudo systemctl disable hostapd dnsmasq');
    fs.writeFileSync("/home/pi/dhcpcd.conf", "# A sample configuration for dhcpcd.\n# See dhcpcd.conf(5) for details.\n\n# Allow users of this group to interact with dhcpcd via the control socket.\n#controlgroup wheel\n\n# Inform the DHCP server of our hostname for DDNS.\nhostname\n\n# Use the hardware address of the interface for the Client ID.\nclientid\n# or\n# Use the same DUID + IAID as set in DHCPv6 for DHCPv4 ClientID as per RFC4361.\n# Some non-RFC compliant DHCP servers do not reply with this set.\n# In this case, comment out duid and enable clientid above.\n#duid\n\n# Persist interface configuration when dhcpcd exits.\npersistent\n\n# Rapid commit support.\n# Safe to enable by default because it requires the equivalent option set\n# on the server to actually work.\noption rapid_commit\n\n# A list of options to request from the DHCP server.\noption domain_name_servers, domain_name, domain_search, host_name\noption classless_static_routes\n# Most distributions have NTP support.\noption ntp_servers\n# Respect the network MTU. This is applied to DHCP routes.\noption interface_mtu\n\n# A ServerID is required by RFC2131.\nrequire dhcp_server_identifier\n\n# Generate Stable Private IPv6 Addresses instead of hardware based ones\nslaac private\n\n# Example static IP configuration:\n#interface eth0\n#static ip_address=192.168.0.10/24\n#static ip6_address=fd51:42f8:caae:d92e::ff/64\n#static routers=192.168.0.1\n#static domain_name_servers=192.168.0.1 8.8.8.8 fd51:42f8:caae:d92e::1\n\n# It is possible to fall back to a static IP if DHCP fails:\n# define static profile\n#profile static_eth0\n#static ip_address=192.168.1.23/24\n#static routers=192.168.1.1\n#static domain_name_servers=192.168.1.1\n\n# fallback to static profile on eth0\n#interface eth0\n#fallback static_eth0\n\n\n#interface wlan0\n    #static ip_address=192.168.4.1/24\n    #nohook wpa_supplicant\n");

  }
  else {
    console.log('wlan up')
    exec('sudo ifconfig wlan0 up');
    var waitTill = new Date(new Date().getTime() + 4000);
    while (waitTill > new Date()) { }
  }

  if (1 - APButton.readSync()) {
    console.log('ap before boot on interrupt');
    exec('sudo systemctl enable hostapd dnsmasq', (error, stdout, stderr) => {
      if (error) {
        console.log(`error:\n${error.message}\n`);
      }
      if (stderr) {
        console.log(`stderr:\n${stderr}\n`);
      }
      console.log(stdout);
    }).on('close', () => {
      fs.writeFileSync("/etc/dhcpcd.conf", "# A sample configuration for dhcpcd.\n# See dhcpcd.conf(5) for details.\n\n# Allow users of this group to interact with dhcpcd via the control socket.\n#controlgroup wheel\n\n# Inform the DHCP server of our hostname for DDNS.\nhostname\n\n# Use the hardware address of the interface for the Client ID.\nclientid\n# or\n# Use the same DUID + IAID as set in DHCPv6 for DHCPv4 ClientID as per RFC4361.\n# Some non-RFC compliant DHCP servers do not reply with this set.\n# In this case, comment out duid and enable clientid above.\n#duid\n\n# Persist interface configuration when dhcpcd exits.\npersistent\n\n# Rapid commit support.\n# Safe to enable by default because it requires the equivalent option set\n# on the server to actually work.\noption rapid_commit\n\n# A list of options to request from the DHCP server.\noption domain_name_servers, domain_name, domain_search, host_name\noption classless_static_routes\n# Most distributions have NTP support.\noption ntp_servers\n# Respect the network MTU. This is applied to DHCP routes.\noption interface_mtu\n\n# A ServerID is required by RFC2131.\nrequire dhcp_server_identifier\n\n# Generate Stable Private IPv6 Addresses instead of hardware based ones\nslaac private\n\n# Example static IP configuration:\n#interface eth0\n#static ip_address=192.168.0.10/24\n#static ip6_address=fd51:42f8:caae:d92e::ff/64\n#static routers=192.168.0.1\n#static domain_name_servers=192.168.0.1 8.8.8.8 fd51:42f8:caae:d92e::1\n\n# It is possible to fall back to a static IP if DHCP fails:\n# define static profile\n#profile static_eth0\n#static ip_address=192.168.1.23/24\n#static routers=192.168.1.1\n#static domain_name_servers=192.168.1.1\n\n# fallback to static profile on eth0\n#interface eth0\n#fallback static_eth0\n\n\ninterface wlan0\n    static ip_address=192.168.4.1/24\n    nohook wpa_supplicant\n");
      console.log('reboot')
      exec('sudo reboot');
    })
  }

  if (1 - wifiButton.readSync()) {
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
          fs.writeFileSync("/etc/dhcpcd.conf", "# A sample configuration for dhcpcd.\n# See dhcpcd.conf(5) for details.\n\n# Allow users of this group to interact with dhcpcd via the control socket.\n#controlgroup wheel\n\n# Inform the DHCP server of our hostname for DDNS.\nhostname\n\n# Use the hardware address of the interface for the Client ID.\nclientid\n# or\n# Use the same DUID + IAID as set in DHCPv6 for DHCPv4 ClientID as per RFC4361.\n# Some non-RFC compliant DHCP servers do not reply with this set.\n# In this case, comment out duid and enable clientid above.\n#duid\n\n# Persist interface configuration when dhcpcd exits.\npersistent\n\n# Rapid commit support.\n# Safe to enable by default because it requires the equivalent option set\n# on the server to actually work.\noption rapid_commit\n\n# A list of options to request from the DHCP server.\noption domain_name_servers, domain_name, domain_search, host_name\noption classless_static_routes\n# Most distributions have NTP support.\noption ntp_servers\n# Respect the network MTU. This is applied to DHCP routes.\noption interface_mtu\n\n# A ServerID is required by RFC2131.\nrequire dhcp_server_identifier\n\n# Generate Stable Private IPv6 Addresses instead of hardware based ones\nslaac private\n\n# Example static IP configuration:\n#interface eth0\n#static ip_address=192.168.0.10/24\n#static ip6_address=fd51:42f8:caae:d92e::ff/64\n#static routers=192.168.0.1\n#static domain_name_servers=192.168.0.1 8.8.8.8 fd51:42f8:caae:d92e::1\n\n# It is possible to fall back to a static IP if DHCP fails:\n# define static profile\n#profile static_eth0\n#static ip_address=192.168.1.23/24\n#static routers=192.168.1.1\n#static domain_name_servers=192.168.1.1\n\n# fallback to static profile on eth0\n#interface eth0\n#fallback static_eth0\n\n\n#interface wlan0\n#    static ip_address=192.168.4.1/24\n#    nohook wpa_supplicant\n");
          console.log('reboot')
          exec('sudo reboot');
        })
      }
      else {
        console.log('wifi already connected');
      }
    });
  }

}

APButton.watch(function (err, value) { //Watch for hardware interrupts on pushButton GPIO, specify callback function
  if (err) { //if an error
    console.error('There was an error', err); //output error message to console
    return;
  }
  togglewifi();
});

wifiButton.watch(function (err, value) { //Watch for hardware interrupts on pushButton GPIO, specify callback function
  if (err) { //if an error
    console.error('There was an error', err); //output error message to console
    return;
  }
  togglewifi();
});

sdButton.watch(function (err, value) { //Watch for hardware interrupts on pushButton GPIO, specify callback function
  if (err) { //if an error
    console.error('There was an error', err); //output error message to console
    return;
  }
  if (value = 1) {
    console.log('shutdown')
    exec('sudo shutdown now');
  }
});


function unexportOnClose() { //function to run when exiting program
  APButton.unexport(); // Unexport Button GPIO to free resources
  wifiButton.unexport(); // Unexport Button GPIO to free resources
  sdButton.unexport(); // Unexport Button GPIO to free resources
  console.log("close");
};

if (APButton.readSync() && wifiButton.readSync()) {
  console.log('wlan down')
  exec('sudo ifconfig wlan0 down');
  exec('sudo systemctl disable hostapd dnsmasq');
  fs.writeFileSync("/home/pi/dhcpcd.conf", "# A sample configuration for dhcpcd.\n# See dhcpcd.conf(5) for details.\n\n# Allow users of this group to interact with dhcpcd via the control socket.\n#controlgroup wheel\n\n# Inform the DHCP server of our hostname for DDNS.\nhostname\n\n# Use the hardware address of the interface for the Client ID.\nclientid\n# or\n# Use the same DUID + IAID as set in DHCPv6 for DHCPv4 ClientID as per RFC4361.\n# Some non-RFC compliant DHCP servers do not reply with this set.\n# In this case, comment out duid and enable clientid above.\n#duid\n\n# Persist interface configuration when dhcpcd exits.\npersistent\n\n# Rapid commit support.\n# Safe to enable by default because it requires the equivalent option set\n# on the server to actually work.\noption rapid_commit\n\n# A list of options to request from the DHCP server.\noption domain_name_servers, domain_name, domain_search, host_name\noption classless_static_routes\n# Most distributions have NTP support.\noption ntp_servers\n# Respect the network MTU. This is applied to DHCP routes.\noption interface_mtu\n\n# A ServerID is required by RFC2131.\nrequire dhcp_server_identifier\n\n# Generate Stable Private IPv6 Addresses instead of hardware based ones\nslaac private\n\n# Example static IP configuration:\n#interface eth0\n#static ip_address=192.168.0.10/24\n#static ip6_address=fd51:42f8:caae:d92e::ff/64\n#static routers=192.168.0.1\n#static domain_name_servers=192.168.0.1 8.8.8.8 fd51:42f8:caae:d92e::1\n\n# It is possible to fall back to a static IP if DHCP fails:\n# define static profile\n#profile static_eth0\n#static ip_address=192.168.1.23/24\n#static routers=192.168.1.1\n#static domain_name_servers=192.168.1.1\n\n# fallback to static profile on eth0\n#interface eth0\n#fallback static_eth0\n\n\n#interface wlan0\n    #static ip_address=192.168.4.1/24\n    #nohook wpa_supplicant\n");
}
else {
  console.log('wlan up')
  exec('sudo ifconfig wlan0 up');
  var waitTill = new Date(new Date().getTime() + 4000);
  while (waitTill > new Date()) { }
}

if (1 - APButton.readSync()) {
  exec('iwconfig', (error, stdout, stderr) => {
    if (error) {
      console.log(`error:\n${error.message}\n`);
    }
    if (stderr) {
      console.log(`stderr:\n${stderr}\n`);
    }
    console.log(stdout);
    if (stdout.indexOf('Mode:Master') == -1) {
      console.log('ap before boot on start');
      exec('sudo systemctl enable hostapd dnsmasq', (error, stdout, stderr) => {
        if (error) {
          console.log(`error:\n${error.message}\n`);
        }
        if (stderr) {
          console.log(`stderr:\n${stderr}\n`);
        }
        console.log(stdout);
      }).on('close', () => {
        fs.writeFileSync("/etc/dhcpcd.conf", "# A sample configuration for dhcpcd.\n# See dhcpcd.conf(5) for details.\n\n# Allow users of this group to interact with dhcpcd via the control socket.\n#controlgroup wheel\n\n# Inform the DHCP server of our hostname for DDNS.\nhostname\n\n# Use the hardware address of the interface for the Client ID.\nclientid\n# or\n# Use the same DUID + IAID as set in DHCPv6 for DHCPv4 ClientID as per RFC4361.\n# Some non-RFC compliant DHCP servers do not reply with this set.\n# In this case, comment out duid and enable clientid above.\n#duid\n\n# Persist interface configuration when dhcpcd exits.\npersistent\n\n# Rapid commit support.\n# Safe to enable by default because it requires the equivalent option set\n# on the server to actually work.\noption rapid_commit\n\n# A list of options to request from the DHCP server.\noption domain_name_servers, domain_name, domain_search, host_name\noption classless_static_routes\n# Most distributions have NTP support.\noption ntp_servers\n# Respect the network MTU. This is applied to DHCP routes.\noption interface_mtu\n\n# A ServerID is required by RFC2131.\nrequire dhcp_server_identifier\n\n# Generate Stable Private IPv6 Addresses instead of hardware based ones\nslaac private\n\n# Example static IP configuration:\n#interface eth0\n#static ip_address=192.168.0.10/24\n#static ip6_address=fd51:42f8:caae:d92e::ff/64\n#static routers=192.168.0.1\n#static domain_name_servers=192.168.0.1 8.8.8.8 fd51:42f8:caae:d92e::1\n\n# It is possible to fall back to a static IP if DHCP fails:\n# define static profile\n#profile static_eth0\n#static ip_address=192.168.1.23/24\n#static routers=192.168.1.1\n#static domain_name_servers=192.168.1.1\n\n# fallback to static profile on eth0\n#interface eth0\n#fallback static_eth0\n\n\ninterface wlan0\n    static ip_address=192.168.4.1/24\n    nohook wpa_supplicant\n");
        console.log('reboot')
        exec('sudo reboot');
      })

    }
    else {
      console.log('ap after boot on start')
      exec('sudo ifdown wlan0', (error, stdout, stderr) => {
        if (error) {
          console.log(`error:\n${error.message}\n`);
        }
        if (stderr) {
          console.log(`stderr:\n${stderr}\n`);
        }
        console.log(stdout);
      })

      exec('sudo systemctl start dnsmasq');

      exec('sudo hostapd -B /etc/hostapd/hostapd.conf', (error, stdout, stderr) => {
        if (error) {
          console.log(`error:\n${error.message}\n`);
        }
        if (stderr) {
          console.log(`stderr:\n${stderr}\n`);
        }
        console.log(stdout);
        if (stdout.indexOf('wlan0: AP-ENABLED') == -1) {
          exec('sudo hostapd -B /etc/hostapd/hostapd.conf', (error, stdout, stderr) => {
            if (error) {
              console.log(`error:\n${error.message}\n`);
            }
            if (stderr) {
              console.log(`stderr:\n${stderr}\n`);
            }
            console.log(stdout);
          })
        }
      })
      exec('sudo systemctl reload dnsmasq', (error, stdout, stderr) => {
        if (error) {
          console.log(`error:\n${error.message}\n`);
        }
        if (stderr) {
          console.log(`stderr:\n${stderr}\n`);
        }
        console.log(stdout);
      })
    }
  })
}

if (1 - wifiButton.readSync()) {
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
      exec('sudo systemctl disable hostapd dnsmasq');
      fs.writeFileSync("/etc/dhcpcd.conf", "# A sample configuration for dhcpcd.\n# See dhcpcd.conf(5) for details.\n\n# Allow users of this group to interact with dhcpcd via the control socket.\n#controlgroup wheel\n\n# Inform the DHCP server of our hostname for DDNS.\nhostname\n\n# Use the hardware address of the interface for the Client ID.\nclientid\n# or\n# Use the same DUID + IAID as set in DHCPv6 for DHCPv4 ClientID as per RFC4361.\n# Some non-RFC compliant DHCP servers do not reply with this set.\n# In this case, comment out duid and enable clientid above.\n#duid\n\n# Persist interface configuration when dhcpcd exits.\npersistent\n\n# Rapid commit support.\n# Safe to enable by default because it requires the equivalent option set\n# on the server to actually work.\noption rapid_commit\n\n# A list of options to request from the DHCP server.\noption domain_name_servers, domain_name, domain_search, host_name\noption classless_static_routes\n# Most distributions have NTP support.\noption ntp_servers\n# Respect the network MTU. This is applied to DHCP routes.\noption interface_mtu\n\n# A ServerID is required by RFC2131.\nrequire dhcp_server_identifier\n\n# Generate Stable Private IPv6 Addresses instead of hardware based ones\nslaac private\n\n# Example static IP configuration:\n#interface eth0\n#static ip_address=192.168.0.10/24\n#static ip6_address=fd51:42f8:caae:d92e::ff/64\n#static routers=192.168.0.1\n#static domain_name_servers=192.168.0.1 8.8.8.8 fd51:42f8:caae:d92e::1\n\n# It is possible to fall back to a static IP if DHCP fails:\n# define static profile\n#profile static_eth0\n#static ip_address=192.168.1.23/24\n#static routers=192.168.1.1\n#static domain_name_servers=192.168.1.1\n\n# fallback to static profile on eth0\n#interface eth0\n#fallback static_eth0\n\n\n#interface wlan0\n#    static ip_address=192.168.4.1/24\n#    nohook wpa_supplicant\n");
      console.log('reboot')
      exec('sudo reboot');
    }
    else {
      console.log('wifi already connected');
    }
  });
}

process.on('SIGINT', unexportOnClose); //function to run when user closes using ctrl+c 
