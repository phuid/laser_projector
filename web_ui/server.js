const fs = require("fs");
const path = require("path");
const formidable = require("formidable");
const { exec } = require("child_process");
var zmq = require("zeromq"),
  command_sender = zmq.socket("sub"),
  ls_receiver = zmq.socket("sub"),
  wifi_man_receiver = zmq.socket("sub");
const config = require("../config.json").web_ui;
var server = require("http").createServer(onRequest);

command_sender.connect("tcp://localhost:5557");

ls_receiver.connect("tcp://localhost:5556");
ls_receiver.subscribe("LASERSHOW");
wifi_man_receiver.connect("tcp://localhost:5556");
wifi_man_receiver.subscribe("WIFIMAN");

var io = require("socket.io")(server);
var SSHClient = require("ssh2").Client;

// Load static files into memory
var staticFiles = {};

var xtermBasePath = path.join(require.resolve("xterm"), "..");
staticFiles["/xterm.css"] = fs.readFileSync(
  path.join(xtermBasePath, "../css/xterm.css")
);
staticFiles["/xterm.js"] = fs.readFileSync(path.join(xtermBasePath, "xterm.js"));
staticFiles["/xterm.js.map"] = fs.readFileSync(path.join(xtermBasePath, "xterm.js.map"));
xtermBasePath = path.join(require.resolve("xterm-addon-fit"), "..");
staticFiles["/xterm-addon-fit.js"] = fs.readFileSync(
  path.join(xtermBasePath, "xterm-addon-fit.js")
);
staticFiles["/xterm-addon-fit.js.map"] = fs.readFileSync(
  path.join(xtermBasePath, "xterm-addon-fit.js.map")
);

staticFiles["/lib/jquery-3.5.1.min.js"] = fs.readFileSync(
  "lib/jquery-3.5.1.min.js"
);

var swiperBasePath = path.join(require.resolve("swiper"), "..");
staticFiles["/swiper-bundle.min.js"] = fs.readFileSync(path.join(swiperBasePath, "swiper-bundle.min.js"));
staticFiles["/swiper-bundle.min.js.map"] = fs.readFileSync(path.join(swiperBasePath, "swiper-bundle.min.js.map"));
staticFiles["/swiper-bundle.min.css"] = fs.readFileSync(path.join(swiperBasePath, "swiper-bundle.min.css"));

staticFiles["/"] = fs.readFileSync("index.html");
staticFiles["/style.css"] = fs.readFileSync("style.css");
staticFiles["/script.js"] = fs.readFileSync("script.js");

// Handle static file serving
function onRequest(req, res) {
  var file;
  if (req.method === "GET" && (file = staticFiles[req.url])) {
    res.writeHead(200, {
      "Content-Type":
        "text/" +
        (/css$/.test(req.url)
          ? "css"
          : /js$/.test(req.url)
          ? "javascript"
          : "html"),
    });
    console.log("sending" + req.url);
    return res.end(file);
  } else if (req.method == "POST") {
    if (req.url == "/fileupload") {
      var form = new formidable.IncomingForm();
      form.parse(req, function (err, fields, files) {
        if (
          files.filetoupload.originalFilename != "" &&
          !(
            files.filetoupload.originalFilename.substring(
              files.filetoupload.originalFilename.indexOf(".")
            ) != ".svg" &&
            files.filetoupload.originalFilename.substring(
              files.filetoupload.originalFilename.indexOf(".")
            ) != ".ild"
          )
        ) {
          var oldpath = files.filetoupload.filepath;

          if (
            files.filetoupload.originalFilename.substring(
              files.filetoupload.originalFilename.indexOf(".")
            ) == ".svg"
          )
            var newpath = path.join(
              __dirname,
              "../svg/" + files.filetoupload.originalFilename
            );
          else if (
            files.filetoupload.originalFilename.substring(
              files.filetoupload.originalFilename.indexOf(".")
            ) == ".ild"
          )
            var newpath = path.join(
              __dirname,
              "../ild/" + files.filetoupload.originalFilename
            );

          fs.rename(oldpath, newpath, function (err) {
            if (err) {
              console.error(err);
              res.writeHead(200, { "Content-Type": "text" });
              res.write("ERROR:\nerror while renaming file\n");
              res.end(
                "MAKE SURE TO SELECT A FILE BEFORE UPLOADING AN EMPTY FORM"
              );
            } else {
              res.writeHead(200, { "Content-Type": "text" });
              res.write(
                "SUCCESS:\nFile uploaded and moved to " +
                  newpath +
                  "\npython3 " +
                  path.join(__dirname, "/svg2ild.py") +
                  " " +
                  newpath +
                  " " +
                  path.join(
                    __dirname,
                    "/ild/" +
                      files.filetoupload.originalFilename.substring(
                        0,
                        files.filetoupload.originalFilename.indexOf(".")
                      )
                  ) +
                  ".ild\n"
              );
              if (
                files.filetoupload.originalFilename.substring(
                  files.filetoupload.originalFilename.indexOf(".")
                ) == ".svg"
              ) {
                exec(
                  "python3 " +
                    path.join(__dirname, "../svg2ild.py") +
                    " " +
                    newpath +
                    " " +
                    path.join(
                      __dirname,
                      "../ild/" +
                        files.filetoupload.originalFilename.substring(
                          0,
                          files.filetoupload.originalFilename.indexOf(".")
                        )
                    ) +
                    ".ild",
                  (error, stdout, stderr) => {
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
                  }
                ).on("close", () => {
                  res.end(
                    'new path: "' +
                      path.join(
                        __dirname,
                        "/ild/" +
                          files.filetoupload.originalFilename.substring(
                            0,
                            files.filetoupload.originalFilename.indexOf(".")
                          )
                      ) +
                      ".ild" +
                      '"'
                  );
                });
              } else {
                res.end(
                  "you uploaded .ild file, so the conversion is not needed"
                );
              }
            }
          });
        } else {
          res.writeHead(200, { "Content-Type": "text" });
          res.write("ERROR:\n");
          res.end("MAKE SURE TO SELECT FILE ENDING WITH .ild / .svg");
        }
      });
    } else if (req.url == "/project") {
      var form = new formidable.IncomingForm();
      form.parse(req, function (err, fields, files) {
        console.log("asdfasdf" + fields.filename);
        console.log(fields.filename);
        if (fields.filename.length > 0) {
          //add stoping of previous projection
          console.log(
            "PROJECT " + path.join(__dirname, "../ild/" + fields.filename)
          );
          send_command(
            "PROJECT " + path.join(__dirname, "../ild/" + fields.filename)
          );
        } else {
          res.end(
            '<h2 style="color: red; font-family: monospace;"><u>INPUT IS EMPTY</u></h2>'
          );
        }
      });
    } else if (req.url == "/listIld") {
      res.writeHead(200, { "Content-Type": "text/json" });

      xtermBasePath = path.join(__dirname, "../ild");
      ILDdirectoryContent = fs.readdirSync(xtermBasePath);
      let files = ILDdirectoryContent.filter((filename) => {
        return fs.statSync(`${xtermBasePath}/${filename}`).isFile();
      });
      let sorted = files.sort((a, b) => {
        let aStat = fs.statSync(`${xtermBasePath}/${a}`),
          bStat = fs.statSync(`${xtermBasePath}/${b}`);
        return (
          new Date(bStat.birthtime).getTime() -
          new Date(aStat.birthtime).getTime()
        );
      });
      res.write(JSON.stringify(sorted));

      res.end();
    }
  } else if (req.method == "GET" && req.url == "/loadoptions") {
    option_names.forEach((option_name) => {
      send_command("OPTION read " + option_name);
    });
  } else {
    res.writeHead(404);
    res.end();
  }
}

io.on("connection", function (socket) {
  var conn = new SSHClient();
  conn
    .on("ready", function () {
      conn.shell(function (err, stream) {
        if (err)
          return socket.emit(
            "sshdata",
            "\r\n*** SSH SHELL ERROR: " + err.message + " ***\r\n"
          );
        socket.on("sshdata", function (data) {
          stream.write(data);
        });
        socket.on("projection", function (data) {
          console.log("projection");
        });
        stream
          .on("data", function (d) {
            socket.emit("sshdata", d.toString("binary"));
          })
          .on("close", function () {
            conn.end();
          });
      });
    })
    .on("close", function () {
      socket.emit("sshdata", "\r\n*** SSH CONNECTION CLOSED ***\r\n");
    })
    .on("error", function (err) {
      socket.emit(
        "sshdata",
        "\r\n*** SSH CONNECTION ERROR: " + err.message + " ***\r\n"
      ); //solution Error:0308010C:digital envelope routines::unsupported https://github.com/facebook/create-react-app/issues/11708#issuecomment-1267989131
    });
    conn.connect({
      host: config.sshHost,
      username: config.sshUsername,
      privateKey: fs.readFileSync(config.sshKeyPath),
    });
});

ls_receiver.on("message", (msg) => {
  io.sockets.emit("LASERSHOWmsg", msg);
});
wifi_man_receiver.on("message", (msg) => {
  io.sockets.emit("WIFIMANmsg", msg);
});

let port = 5000;
console.log("Listening on http://localhost:" + port);
server.listen(port);
