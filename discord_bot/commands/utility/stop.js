const { SlashCommandBuilder } = require("discord.js");

module.exports = {
  data: new SlashCommandBuilder()
    .setName("stop")
    .setDescription("used to stop current projection"),

  async execute(interaction) {
    var zmq = require("zeromq"),
      lasershow_sender = zmq.socket("pub"),
      lasershow_receiver = zmq.socket("sub");

    lasershow_sender.connect("tcp://localhost:5557");
    lasershow_receiver.connect("tcp://localhost:5556");
    lasershow_receiver.subscribe("");

    lasershow_sender.send(
      "STOP"
    );

    await interaction.reply(
      "*lasershow* **<<** `STOP`"
    );
  },
};
