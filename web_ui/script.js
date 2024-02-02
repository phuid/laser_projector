class terminal {
  constructor(container_id) {
    this.container = document.getElementById(container_id);
    this.term = new Terminal({
      cursorBlink: true,
      allowTransparency: true,
      drawBoldTextInBrightColors: true,
      fontSize:
        1 * parseFloat(getComputedStyle(document.documentElement).fontSize),
      theme: {
        background: "rgba(0, 0, 0, 0)",
      },
    });
    this.fit = new FitAddon.FitAddon();
    this.term.loadAddon(this.fit);
    this.term.open(this.container);
    this.fit.fit();
  }
}

if (screen.availHeight > screen.availWidth) {
  var swiper = new Swiper(".swiper", {
    grabCursor: false,
    effect: "coverflow",
    spaceBetween: 30,
    coverflowEffect: {
      rotate: 0,
      stretch: 0,
      depth: 0,
      modifier: 1,
      slideShadows: false,
    },
    pagination: {
      el: ".swiper-pagination",
      clickable: true,
    },
  });
}
else {
  console.log("landscape detected - removing swiper classes")
  const classes_to_remove_in_landscape = ["swiper", "swiper-wrapper", "swiper-slide", "swiper-pagination", "swiper-button-prev", "swiper-button-next"];
  classes_to_remove_in_landscape.forEach((classname) => {
    for (var i = 0; i < document.getElementsByClassName(classname).length; i++){
      console.log(document.getElementsByClassName(classname)[i]);
      document.getElementsByClassName(classname)[i].classList.remove(classname)
    }
  });
}


var terminals = {
  ssh: new terminal("ssh-terminal-container"),
  lasershow: new terminal("lasershow-terminal-container"),
  wifiman: new terminal("wifiman-terminal-container"),
};

var socket = io(); //.connect();
socket.on("connect", function () {
  Object.entries(terminals).forEach(([key, terminal]) => {
    terminal.term.write("\r\n*** WebSocket connection estabilished ***\r\n");
  });
});
socket.on("disconnect", function () {
  Object.entries(terminals).forEach(([key, terminal]) => {
    terminal.term.write("\r\n*** WebSocket connection lost ***\r\n");
  });
  console.log("socket lost");
});

// Browser -> Backend
terminals.ssh.term.onData(function (ev) {
  socket.emit("sshdata", ev.toString());
});

// Browser -> Backend
lasershow_line = ""
terminals.lasershow.term.onData(function (ev) {
  string = ev.toString();
  lasershow_line += string;
  if (string.match(/\n|\r/gi) != null) {
    lasershow_line.replace(/\n|\r/gi, "")
    socket.emit("LASERSHOWdata", lasershow_line);
    terminals.lasershow.term.write("\n\r> ");
    lasershow_line = "";
  } else {
    terminals.lasershow.term.write(ev);
  }
});
// Browser -> Backend
wifiman_line = ""
terminals.wifiman.term.onData(function (ev) {
  string = ev.toString();
  lasershow_line += string;
  if (string.match(/\n|\r/gi) != null) {
    lasershow_line.replace(/\n|\r/gi, "")
    socket.emit("WIFIMANdata", wifiman_line);
    terminals.lasershow.term.write("\n\r> ");
    lasershow_line = "";
  } else {
    terminals.lasershow.term.write(ev);
  }
});

// Backend -> Browser
socket.on("sshdata", function (data) {
  terminals.ssh.term.write(data);
});
// Backend -> Browser
socket.on("LASERSHOWmsg", function (data) {
  console.log(data);
  terminals.lasershow.term.write(data);
});
// Backend -> Browser
socket.on("WIFIMANmsg", function (data) {
  terminals.wifiman.term.write(data);
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

// Object.entries(terminals).forEach(([key, terminal]) => {
//   terminal.container.style.zIndex = "10";
// });

addEventListener("resize", (event) => {
  Object.entries(terminals).forEach(([key, terminal]) => {
    terminal.fit.fit();
  });
});

// swiper.updateSlides()