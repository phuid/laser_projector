class terminal {
  constructor(container_id) {
    this.container = document.getElementById(container_id);
    this.Term = new Terminal({
      cursorBlink: true,
      allowTransparency: true,
      drawBoldTextInBrightColors: true,
      fontSize:
        1 * parseFloat(getComputedStyle(document.documentElement).fontSize),
      theme: {
        background: "rgba(0, 0, 0, 0)",
      },
    });
    this.sshFitAddon = new FitAddon.FitAddon();
    this.Term.loadAddon(sshFitAddon);
    this.Term.open(TerminalContainer);
    this.sshFitAddon.fit();
  }
}

var terminals = {
  ssh: terminal("ssh-terminal-container"),
  lasershow: terminal("lasershow-terminal-container"),
  wifiman: terminal("wifiman-terminal-container"),
};

var socket = io(); //.connect();
socket.on("connect", function () {
  terminals.forEach((terminal) => {
    terminal.write("\r\n*** WebSocket connection estabilished ***\r\n");
  });
});
socket.on("disconnect", function () {
  terminals.forEach((terminal) => {
    terminal.write("\r\n*** WebSocket connection lost ***\r\n");
  });
});

// Browser -> Backend
terminals.ssh.onData(function (ev) {
  socket.emit("sshdata", ev.toString());
});

// Backend -> Browser
socket.on("sshdata", function (data) {
  terminals.ssh.write(data);
});

// Backend -> Browser
socket.on("LASERSHOWmsg", function (data) {
  terminals.lasershow.write(data);
});

// Backend -> Browser
socket.on("WIFIMANmsg", function (data) {
  terminals.wifiman.write(data);
});

socket.on("alert", (alert) => {
  alert(alert);
});

$("#fileupload").submit(function (e) {
  e.preventDefault(); // prevent actual form submit
  var postData = new FormData(this);
  $.ajax({
    type: "POST",
    url: "/fileupload",
    data: postData,
    contentType: false, //this is requireded please see answers above
    processData: false, //this is requireded please see answers above
    success: function (data) {
      alert(data);
    },
    error: function () {
      alert("HTTP request ERROR\n");
    },
  });
});
$("#projectionform").submit(function (e) {
  e.preventDefault(); // prevent actual form submit
  var postData = new FormData(this);
  $.ajax({
    type: "POST",
    url: "/project",
    data: postData,
    contentType: false, //this is requireded please see answers above
    processData: false, //this is requireded please see answers above
    success: function (data) {
      terminals.ssh.write("\n" + data);
    },
    error: function (data) {
      alert("ERROR\n" + data);
    },
  });
});
$("#projectsvg").on("click", () => {
  var postData = new FormData(document.getElementById("projectionform"));
  $.ajax({
    type: "POST",
    url: "/project",
    data: postData,
    contentType: false, //this is requireded please see answers above
    processData: false, //this is requireded please see answers above
    success: function (data) {
      terminals.ssh.write("\n" + data);
    },
    error: function (data) {
      alert("ERROR\n" + data);
    },
  });
});

function fillProjectForm() {
  fetch("/listIld", { method: "POST" }).then((response) => {
    console.log(response);
    response.json().then((data) => {
      data.forEach((filename) => {
        document.getElementById(
          "lastpatharea"
        ).innerHTML += `<option class=\"IldOption\" value=\"${filename}\">${filename}</option>`;
      });
    });
  });
}

fillProjectForm();

function focusOnTextArea(parent_element) {
  parent_element.getElementsByClassName("xterm-helper-textarea")[0].focus();
}

terminals.forEach(terminal => {
  terminal.container.style.zIndex = "10";
});

window.mySwipe = new Swipe(document.getElementById('slider'), {
  startSlide: 2,
  speed: 400,
  auto: 3000,
  continuous: true,
  disableScroll: false,
  stopPropagation: false,
  callback: function(index, elem) {},
  transitionEnd: function(index, elem) {}
});
