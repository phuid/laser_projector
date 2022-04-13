var terminalContainer = document.getElementById('terminal-container');
const term = new Terminal({
  cursorBlink: true,
  allowTransparency: true,
  drawBoldTextInBrightColors: true,
  fontSize: (1 * parseFloat(getComputedStyle(document.documentElement).fontSize)),
  theme: {
    background: 'rgba(0, 0, 0, 0)'
  }
});
const fitAddon = new FitAddon.FitAddon();
term.loadAddon(fitAddon);
term.open(terminalContainer);
fitAddon.fit();

var socket = io() //.connect();
socket.on('connect', function () {
  term.write('\r\n*** WebSocket connection estabilished ***\r\n');
});

// Browser -> Backend
term.onData(function (ev) {
  socket.emit('sshdata', ev.toString());
});

// Backend -> Browser
socket.on('sshdata', function (data) {
  term.write(data);
});

socket.on('disconnect', function () {
  term.write('\r\n*** WebSocket connection lost ***\r\n');
});

socket.on('alert', (alert) => {
  alert(alert);
});

$("#fileupload").submit(function (e) {
  e.preventDefault(); // prevent actual form submit
  var postData = new FormData(this);
  $.ajax({
    type: "POST",
    url: '/fileupload',
    data: postData,
    contentType: false, //this is requireded please see answers above
    processData: false, //this is requireded please see answers above
    success: function (data) {
      alert(data);
    },
    error: function () {
      alert('HTTP request ERROR\n');
    }
  });
});
$("#projectionform").submit(function (e) {
  e.preventDefault(); // prevent actual form submit
  var postData = new FormData(this);
  $.ajax({
    type: "POST",
    url: '/project',
    data: postData,
    contentType: false, //this is requireded please see answers above
    processData: false, //this is requireded please see answers above
    success: function (data) {
      term.write('\n' + data);
    },
    error: function (data) {
      alert('ERROR\n' + data);
    }
  });
});

function focusOnTextArea() {
  document.getElementsByClassName('xterm-helper-textarea')[0].focus();
}

document.getElementById('terminal-container').style.zIndex = '10';