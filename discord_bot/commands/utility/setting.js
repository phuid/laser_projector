const { SlashCommandBuilder } = require("discord.js");
const { setting_names } = require("../../setting_names.json");

module.exports = {
  data: new SlashCommandBuilder()
    .setName("setting")
    .setDescription("used to modify or read settings")
    .addStringOption((option) => {
      option
        .setName("action")
        .setDescription("read/write")
        .setRequired(true)
        .addChoices(
          { name: "read", value: "read" },
          { name: "write", value: "write" },
          { name: "reset", value: "reset" }
        );
      return option;
    })
    .addStringOption((option) => {
      mychoices = [];
      option
        .setName("setting_name")
        .setDescription("name of the modified or read setting")
        .setRequired(true);
      for (var setting of setting_names) {
        option.addChoices({ name: setting.name, value: setting.value });
      }
      return option;
    })
    .addNumberOption((option) => {
      option
        .setName("value")
        .setDescription("the value to be asigned to the setting")
        .setRequired(false);
      return option;
    }),

  async execute(interaction) {
    var zmq = require("zeromq"),
      lasershow_sender = zmq.socket("pub"),
      lasershow_receiver = zmq.socket("sub");

    lasershow_sender.connect("tcp://localhost:5557");
    lasershow_receiver.connect("tcp://localhost:5556");
    lasershow_receiver.subscribe("");

    const setting_name = interaction.options.getString("setting_name");
    const action = interaction.options.getString("action");
    const value = interaction.options.getNumber("value");

    if (action == "write" && value == null) {
      await interaction.reply(
        "You must provide a value when writing an option"
      );
      return;
    }

    lasershow_sender.send(
      "OPTION " +
        action +
        " " +
        setting_name +
        (value != null ? " " + value : "")
    );

    await interaction.reply(
      "*lasershow* **<<** `OPTION " +
        action +
        " " +
        setting_name +
        (value != null ? " " + value : "") +
        "`"
    );
  },
};
