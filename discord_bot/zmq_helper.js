var zmq = require("zeromq"),
  lasershow_sender = zmq.socket("pub"),
  lasershow_receiver = zmq.socket("sub");

lasershow_sender.connect("tcp://localhost:5557");
lasershow_receiver.connect("tcp://localhost:5556");
lasershow_receiver.subscribe("");

const lasershow_send = async (msg) => {
  lasershow_sender.send(msg);
};

exports.lasershow_send = lasershow_send;
exports.lasershow_receiver = lasershow_receiver;
