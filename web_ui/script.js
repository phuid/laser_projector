function toggledisplay(el) {
  console.log("changing display on", el);
  if (el.style.display != "block") {
    el.style.display = "block";
  } else {
    el.style.display = "none";
  }
}

class terminal {
  constructor(container_id) {
    this.container = document.getElementById(container_id);
    this.term = new Terminal({
      cursorBlink: container_id == "ssh-terminal-container",
      cursorWidth: container_id == "ssh-terminal-container",
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
  let swiper = new Swiper(".swiper", {
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
} else {
  console.log("landscape detected - removing swiper classes");
  const classes_to_remove_in_landscape = [
    "swiper",
    "swiper-wrapper",
    "swiper-slide",
    "swiper-pagination",
    "swiper-button-prev",
    "swiper-button-next",
  ];
  classes_to_remove_in_landscape.forEach((classname) => {
    for (
      let i = 0;
      i < document.getElementsByClassName(classname).length;
      i++
    ) {
      console.log(document.getElementsByClassName(classname)[i]);
      document.getElementsByClassName(classname)[i].classList.remove(classname);
    }
  });
}

let terminals = {
  ssh: new terminal("ssh-terminal-container"),
  lasershow: new terminal("lasershow-terminal-container"),
  wifiman: new terminal("wifiman-terminal-container"),
};

let socket = io(); //.connect();
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
lasershow_line = "";
terminals.lasershow.term.onData(function (ev) {
  string = ev.toString();
  lasershow_line += string;
  if (string.match(/\n|\r/gi) != null) {
    lasershow_line = lasershow_line.replace(/\n|\r/gi, "");
    socket.emit("LASERSHOWdata", lasershow_line);
    terminals.lasershow.term.write("lasershow $ " + lasershow_line + "\n\r");
    document.getElementById("ls_input").innerHTML = "lasershow $ ";
    lasershow_line = "";
  }
  document.getElementById("ls_input").innerHTML =
    "lasershow $ " + lasershow_line;
});
// Browser -> Backend
wifiman_line = "";
terminals.wifiman.term.onData(function (ev) {
  string = ev.toString();
  wifiman_line += string;
  if (string.match(/\n|\r/g) != null) {
    wifiman_line = wifiman_line.replace(/\n|\r/g, "");
    socket.emit("WIFIMANdata", wifiman_line);
    terminals.wifiman.term.write("wifi_manager $ " + wifiman_line + "\n\r");
    wifiman_line = "";
  }
  document.getElementById("wifiman_input").innerHTML =
    "wifi_manager $ " + wifiman_line;
});

// Backend -> Browser
socket.on("sshdata", function (data) {
  terminals.ssh.term.write(data);
});
// Backend -> Browser
socket.on("LASERSHOWmsg", function (data) {
  console.log(data);
  terminals.lasershow.term.write(data.replace(/\n/g, "\n\r"));

  const words = data.split(" ");

  if (words.length > 0) {
    if (words[0] == "INFO:") {
      if (words.length > 1) {
        if (words[1] == "OPTION" && words.length > 3) {
          if (words[2] == "time_accurate_framing") {
            document.getElementById("time_accurate_framing").checked = Number(
              words[3]
            );
          } else {
            document.getElementById(words[2]).value = Number(words[3]);
            document.getElementById(words[2] + "-output").innerHTML = Number(
              words[3]
            );
          }
        } else if (words[1] == "FRAME" && words.length > 4) {
          document.getElementById("current_frame").value = Number(words[2]);
          document.getElementById("current_frame-output").innerHTML = Number(
            words[2]
          );
          document.getElementById("current_frame").max = Number(words[4]);
          document.getElementById("current_frame-max").innerHTML = Number(
            words[4]
          );
        } else if (words[1] == "PROJECT" && words.length > 2) {
          split_dirs = words[2].split("/");
          document.getElementById("current_filename").innerHTML =
            split_dirs[split_dirs.length - 1];
          document.getElementById("play").innerHTML = ">";
          document.getElementById("play").classlist.add("saturate");
        } else if (words[1] == "PAUSE" && words.length > 2) {
          console.log('"' + words[2] + '"');
          if (words[2].trim() == "0") {
            document.getElementById("play").innerHTML = ">";
            document.getElementById("play").classList.add("saturate");
          } else {
            document.getElementById("play").innerHTML = "||";
            document.getElementById("play").classList.remove("saturate");
          }
        }
      }
    }
  }
});
// Backend -> Browser
socket.on("WIFIMANmsg", function (data) {
  terminals.wifiman.term.write(data.replace(/\n/g, "\n\r"));
  console.log(data);

  pos = data.indexOf(":");
  id = data.substring(0, pos);
  data = data.substring(pos + 2);

  if (data.length > 0) {
    document.getElementById(id).value = data.trim();
    console.log(document.getElementById(id));
  }
});

socket.on("alert", (alert) => {
  alert(JSON.stringify(alert));
});

function setMyVal(el) {
  socket.emit(
    "LASERSHOWdata",
    "OPTION write " + el.id + " " + el.value.toString()
  );
  terminals.lasershow.term.write(
    "> OPTION write " + el.id + " " + el.value.toString() + "\n\r"
  );
}

function readLsSettings() {
  Object.entries(
    document.getElementById("LsSettings").getElementsByTagName("input")
  ).forEach((el) => {
    socket.emit("LASERSHOWdata", "OPTION read " + el[1].id);
  });
}

function readWMSettings() {
  socket.emit("WIFIMANdata", "read");
}

$("#fileupload").submit(function (e) {
  console.log("upload");
  e.preventDefault(); // prevent actual form submit
  let postData = new FormData(this);
  $.ajax({
    type: "POST",
    url: "/fileupload",
    data: postData,
    contentType: false, //this is requireded please see answers above
    processData: false, //this is requireded please see answers above
    success: function (data) {
      alert(JSON.stringify(data));
    },
    error: function () {
      alert("HTTP request ERROR\n");
    },
  });
});
$("#projectionform").submit(function (e) {
  e.preventDefault(); // prevent actual form submit
  let postData = new FormData(this);
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
      alert("ERROR\n" + JSON.stringify(data));
    },
  });
});
$("#projectsvg").on("click", () => {
  let postData = new FormData(document.getElementById("projectionform"));
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
      alert("ERROR\n" + JSON.stringify(data));
    },
  });
});

$("#uploadsvg").on("click", () => {
  console.log("upload");
  let postData = new FormData(document.getElementById("fileupload"));
  $.ajax({
    type: "POST",
    url: "/fileupload",
    data: postData,
    contentType: false, //this is requireded please see answers above
    processData: false, //this is requireded please see answers above
    success: function (data) {
      terminals.ssh.write("\n" + data);
    },
    error: function (data) {
      alert("ERROR\n" + JSON.stringify(data));
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

async function uploadDrawFile() {
  let points = [];

  let HEAD_LEN = 32;
  let POINT_LEN = 8; //format 5 - 2D Coordinates with True Color
  let array = new Uint8Array(HEAD_LEN + (points.length * POINT_LEN));

  //write head

  //write "ILDA" to the first 4 bytes
  array[0] = "I".charCodeAt(0);
  array[1] = "L".charCodeAt(0);
  array[2] = "D".charCodeAt(0);
  array[3] = "A".charCodeAt(0);

  //write reserved 0 to the next 3 bytes
  array[4] = 0;
  array[5] = 0;
  array[6] = 0;

  //write format code to the next byte
  array[7] = 5;

  //write "lasP-tmp" to the next 8 bytes
  array[8] = "l".charCodeAt(0);
  array[9] = "a".charCodeAt(0);
  array[10] = "s".charCodeAt(0);
  array[11] = "P".charCodeAt(0);
  array[12] = "-".charCodeAt(0);
  array[13] = "t".charCodeAt(0);
  array[14] = "m".charCodeAt(0);
  array[15] = "p".charCodeAt(0);

  //write "gh/phuid" to the next 8 bytes
  array[16] = "g".charCodeAt(0);
  array[17] = "h".charCodeAt(0);
  array[18] = "/".charCodeAt(0);
  array[19] = "p".charCodeAt(0);
  array[20] = "h".charCodeAt(0);
  array[21] = "u".charCodeAt(0);
  array[22] = "i".charCodeAt(0);
  array[23] = "d".charCodeAt(0);

  //write number of points to the next 2 bytes
  array[24] = points.length & (0xf0 >> 8); //fixme? (not sure if this is correct)
  array[25] = points.length & 0x0f;

  //write frame number to the next 2 bytes
  array[26] = 0;
  array[27] = 0;

  //write total number of frames to the next 2 bytes
  array[28] = 1;
  array[29] = 1;

  //write projector number to the next byte
  array[30] = 0;

  //write reserved 0 to the next byte
  array[31] = 0;

  //write points
  for (let i = 0; i < points.length; i++) {
    array[i * POINT_LEN + HEAD_LEN + 1] = points[i].x & (0xf0 >> 8); //fixme? (not sure if this is correct)
    array[i * POINT_LEN + HEAD_LEN + 1 + 1] = points[i].x & 0x0f;

    array[i * POINT_LEN + HEAD_LEN + 1 + 2] = points[i].y & (0xf0 >> 8); //fixme again
    array[i * POINT_LEN + HEAD_LEN + 1 + 3] = points[i].y & 0x0f;

    array[i * POINT_LEN + HEAD_LEN + 1 + 4] = points[i].status;

    array[i * POINT_LEN + HEAD_LEN + 1 + 5] = points[i].r;
    array[i * POINT_LEN + HEAD_LEN + 1 + 6] = points[i].g;
    array[i * POINT_LEN + HEAD_LEN + 1 + 7] = points[i].b;
  }
  let blob = new Blob([array]);

  let formData = new FormData();
  fd.append("fname", "laserProjector-draw-tmp.ild");
  formData.append("file", blob);
  await fetch("/upload-draw-file", {
    method: "POST",
    body: formData,
  });
  alert("The file has been uploaded successfully.");
}


drawcanvas = document.getElementById("drawCanvas");
