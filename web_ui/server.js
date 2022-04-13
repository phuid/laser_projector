const fs = require('fs');
const path = require('path');
const formidable = require('formidable');
const { exec } = require('child_process');
const config = require('../config.json').web_ui;
var server = require('http').createServer(onRequest);

var io = require('socket.io')(server);
var SSHClient = require('ssh2').Client;

// Load static files into memory
var staticFiles = {};
var basePath = path.join(require.resolve('xterm'), '..');
staticFiles['/xterm.css'] = fs.readFileSync(path.join(basePath, '../css/xterm.css'));
staticFiles['/xterm.js'] = fs.readFileSync(path.join(basePath, 'xterm.js'));
basePath = path.join(require.resolve('xterm-addon-fit'), '..');
staticFiles['/xterm-addon-fit.js'] = fs.readFileSync(path.join(basePath, 'xterm-addon-fit.js'));
staticFiles['/lib/jquery-3.5.1.min.js'] = fs.readFileSync('lib/jquery-3.5.1.min.js');
staticFiles['/'] = fs.readFileSync('index.html');
staticFiles['/style.css'] = fs.readFileSync('style.css');
staticFiles['/script.js'] = fs.readFileSync('script.js');

// Handle static file serving
function onRequest(req, res) {
  var file;
  if (req.method === 'GET' && (file = staticFiles[req.url])) {
    res.writeHead(200, {
      'Content-Type': 'text/'
        + (/css$/.test(req.url)
          ? 'css'
          : (/js$/.test(req.url) ? 'javascript' : 'html'))
    });
    return res.end(file);
  }
  else if (req.method == 'POST') {
    if (req.url == '/fileupload') {
      var form = new formidable.IncomingForm();
      form.parse(req, function (err, fields, files) {
        if (files.filetoupload.originalFilename != "" && !(files.filetoupload.originalFilename.substring(files.filetoupload.originalFilename.indexOf('.')) != ".svg" && files.filetoupload.originalFilename.substring(files.filetoupload.originalFilename.indexOf('.')) != ".ild")) {
          var oldpath = files.filetoupload.filepath;

          if (files.filetoupload.originalFilename.substring(files.filetoupload.originalFilename.indexOf('.')) == ".svg")
            var newpath = path.join(__dirname, '../svg/' + files.filetoupload.originalFilename);
          else if (files.filetoupload.originalFilename.substring(files.filetoupload.originalFilename.indexOf('.')) == ".ild")
            var newpath = path.join(__dirname, '../ild/' + files.filetoupload.originalFilename);

          fs.rename(oldpath, newpath, function (err) {
            if (err) {
              console.error(err);
              res.writeHead(200, { 'Content-Type': 'text' });
              res.write('ERROR:\nerror while renaming file\n');
              res.end('MAKE SURE TO SELECT A FILE BEFORE UPLOADING AN EMPTY FORM');
            }
            else {
              res.writeHead(200, { 'Content-Type': 'text' });
              res.write('SUCCESS:\nFile uploaded and moved to ' + newpath + '\npython3 ' + path.join(__dirname, '/svg2ild.py') + ' ' + newpath + ' ' + path.join(__dirname, '/ild/' + files.filetoupload.originalFilename.substring(0, files.filetoupload.originalFilename.indexOf('.'))) + '.ild\n');
              if (files.filetoupload.originalFilename.substring(files.filetoupload.originalFilename.indexOf('.')) == ".svg") {
                exec('python3 ' + path.join(__dirname, '../svg2ild.py') + ' ' + newpath + ' ' + path.join(__dirname, '../ild/' + files.filetoupload.originalFilename.substring(0, files.filetoupload.originalFilename.indexOf('.'))) + '.ild', (error, stdout, stderr) => {
                  if (error) {
                    res.write(`error:\n${error.message}\n`);
                    console.log(`error:\n${error.message}\n`);
                  }
                  if (stderr) {
                    res.write(`stderr:\n${stderr}\n`);
                    console.log(`stderr:\n${stderr}\n`);
                  }
                  res.write(`stdout:\n${stdout}\n`);
                  console.log(`${stdout}`);

                }).on('close', () => {
                  res.end('new path: "' + path.join(__dirname, '/ild/' + files.filetoupload.originalFilename.substring(0, files.filetoupload.originalFilename.indexOf('.'))) + '.ild' + '"');
                })
              }
              else {
                res.end('you uploaded .ild file, so the conversion is not needed');
              }
            }
          });
        }
        else {
          res.writeHead(200, { 'Content-Type': 'text' });
          res.write('ERROR:\n');
          res.end('MAKE SURE TO SELECT FILE ENDING WITH .ild / .svg');
        }
      });
    }
    else if (req.url == '/project') {
      var form = new formidable.IncomingForm();
      form.parse(req, function (err, fields, files) {
        console.log('asdfasdf' + fields.filename);
        console.log(fields.filename);
        if (fields.filename.length > 0) {
          //add stoping of previous projection
          console.log('../lasershow 0 ' + path.join(__dirname, '../ild/' + fields.filename));
          exec('../lasershow 0 ' + path.join(__dirname, '../ild/' + fields.filename), (error, stdout, stderr) => {
            if (error) {
              res.write(`error:\n${error.message}\n`);
              console.log(`error:\n${error.message}\n`);
            }
            if (stderr) {
              res.write(`stderr:\n${stderr}\n`);
              console.log(`stderr:\n${stderr}\n`);
            }
            res.write('stdout:\n');

            for (var i = 0; i < stdout.length; i++) {
              if (stdout[i] == '\n') {
                res.write('\n');
              }
              //protection from creating unwanted html elements
              else if (stdout[i] == '<' || stdout[i] == '>') continue;
              else {
                res.write(stdout[i]);
              }
            }
            console.log(`${stdout}`);

          }).on('close', () => {
            res.end('\n    **process finished**')
          })
        }
        else {
          res.end('<h2 style="color: red; font-family: monospace;"><u>INPUT IS EMPTY</u></h2>');
        }
      });
    }
  }
  else {
    res.writeHead(404);
    res.end();
  }
}

io.on('connection', function (socket) {
  var conn = new SSHClient();
  conn.on('ready', function () {
    socket.emit('sshdata', '\r\n*** BACKEND CONNECTION ESTABLISHED ***\r\n');
    conn.shell(function (err, stream) {
      if (err)
        return socket.emit('sshdata', '\r\n*** SSH SHELL ERROR: ' + err.message + ' ***\r\n');
      socket.on('sshdata', function (data) {
        stream.write(data);
      });
      stream.on('data', function (d) {
        socket.emit('sshdata', d.toString('binary'));
      }).on('close', function () {
        conn.end();
      });
    });
  }).on('close', function () {
    socket.emit('sshdata', '\r\n*** SSH CONNECTION CLOSED ***\r\n');
  }).on('error', function (err) {
    socket.emit('sshdata', '\r\n*** SSH CONNECTION ERROR: ' + err.message + ' ***\r\n');
  }).connect({
    host: config.sshHost,
    port: config.sshPort,
    username: config.sshUsername,
    password: config.sshPassword
  });
});

let port = 3000;
console.log('Listening on port', port)
server.listen(port);