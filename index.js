// You can use 'exec' this way

const { exec } = require("child_process");

var http = require('http');
var formidable = require('formidable');
var fs = require('fs');
var path = require('path');

var lastuploadedpath;

http.createServer(function (req, res) {
  if (req.url == '/style.css') {
    res.writeHead(200, { 'Content-Type': 'text/css' });
    var filePath = path.join(__dirname, 'style.css');
    var readStream = fs.createReadStream(filePath);
    // We replaced all the event handlers with a simple call to readStream.pipe()
    readStream.pipe(res);
  }
  else {
    res.writeHead(200, { 'Content-Type': 'text/html' });
    res.write('<html lang="cs"><head><meta cahrset="utf-8"><title>laser-projector</title><link rel="stylesheet" href="style.css"></head><body>')
    res.write('<form id="fileupload" action="fileupload" method="post" enctype="multipart/form-data">');
    res.write('<input type="file" name="filetoupload"><br>');
    res.write('<input class="button" type="submit" value="upload">');
    res.write('</form>');
    res.write('<form id="input" action="cmd" method="post" enctype="multipart/form-data">');
    res.write('<input type="textarea" rows="4" cols="50" name="text" autofocus><br>');
    res.write('<input class="button" type="submit" value="run cmd">');
    res.write('</form>');
    res.write('<div id="output">');


    if (req.url == '/fileupload') {
      var form = new formidable.IncomingForm();
      form.parse(req, function (err, fields, files) {
        var oldpath = files.filetoupload.filepath;

        if (files.filetoupload.originalFilename.substring(files.filetoupload.originalFilename.indexOf('.')) == ".svg")
          var newpath = path.join(__dirname, 'svg/' + files.filetoupload.originalFilename);
        else if (files.filetoupload.originalFilename.substring(files.filetoupload.originalFilename.indexOf('.')) == ".ild")
          var newpath = path.join(__dirname, 'ild/' + files.filetoupload.originalFilename);

        fs.rename(oldpath, newpath, function (err) {
          res.write('File uploaded and moved! to ' + newpath + '<br>');
          if (err) {
            console.error(err);
            res.write('<h3 style="color: red; font-family: monospace;">error while renaming file</h3>');
            res.write('<h2 style="color: red; font-family: monospace;"><u>MAKE SURE TO SELECT A FILE</u> BEFORE UPLOADING AN EMPTY FORM</h2>');
            res.end('</body></html>')
          }
          else {
            if (files.filetoupload.originalFilename.substring(files.filetoupload.originalFilename.indexOf('.')) == ".svg") {
              exec('python3 ' + path.join(__dirname, '/svg2ild.py') + ' ' + newpath + ' ' + path.join(__dirname, '/ild/' + files.filetoupload.originalFilename.substring(0, files.filetoupload.originalFilename.indexOf('.'))) + '.ild', (error, stdout, stderr) => {
                if (error) {
                  res.write(`error:\n${error.message}`);
                  console.log(`error:\n${error.message}`);
                  return;
                }
                if (stderr) {
                  res.write(`stderr:\n${stderr}`);
                  console.log(`stderr:\n${stderr}`);
                  return;
                }
                if (stdout.length > 0) {
                  res.write(stdout[0]);
                  for (var i = 1; i < stdout.length; i++) {
                    if (stdout[i] == '\n') {
                      res.write('<br>');
                    }
                    else if (stdout[i] == '<' || stdout[i] == '>') continue;
                    else {
                      res.write(stdout[i]);
                    }
                  }
                  console.log(`${stdout}`);
                }
              }).on('close', () => {
                res.write('new path: "' + path.join(__dirname, '/ild/' + files.filetoupload.originalFilename.substring(0, files.filetoupload.originalFilename.indexOf('.'))) + '.ild' + '"');
                return res.end('</div></body></html>');
              })
            }
            else {
              res.end('</div></body></html>');
            }
          }
        });

      });
    }
    else if (req.url == '/cmd') {
      var form = new formidable.IncomingForm();
      form.parse(req, function (err, fields, files) {
        console.log(fields.text);
        if (fields.text.length > 0) {
          exec(fields.text, (error, stdout, stderr) => {
            if (error) {
              res.write(`error:\n${error.message}`);
              console.log(`error:\n${error.message}`);
              return;
            }
            if (stderr) {
              res.write(`stderr:\n${stderr}`);
              console.log(`stderr:\n${stderr}`);
              return;
            }
            if (stdout.length > 0) {
              res.write(stdout[0]);
              for (var i = 1; i < stdout.length; i++) {
                if (stdout[i] == '\n') {
                  res.write('<br>');
                }
                else if (stdout[i] == '<' || stdout[i] == '>') continue;
                else {
                  res.write(stdout[i]);
                }
              }
              console.log(`${stdout}`);
            }
          }).on('close', () => {
            return res.end('</div></body></html>');
          })
        }
        else {
          res.write('<h2 style="color: red; font-family: monospace;"><u>INPUT IS EMPTY</u></h2>');
          res.end('</div></body></html>');
        }
      });
    }
    else {
      return res.end('</div></body></html>');
    }
  }
}).listen(5000);
console.log("localhost:5000");
