const {
  SlashCommandBuilder,
  StringSelectMenuBuilder,
  StringSelectMenuOptionBuilder,
  ActionRowBuilder,
} = require("discord.js");
const fs = require("node:fs");
const { setting_names } = require("../../setting_names.json");

const getSortedFiles = async (dir) => {
  ILDdirectoryContent = fs.readdirSync(dir);
  let files = Array();
  ILDdirectoryContent.forEach((filename) => {
    files.push({filename: filename, stats: fs.statSync(`${dir}/${filename}`)});
  });
  files = files.filter((file) => {
    return file.stats.isFile();
  });
  let sorted = files.sort((a, b) => {
    // let aStat = fs.statSync(`${dir}/${a}`),
    //   bStat = fs.statSync(`${dir}/${b}`);
    return (
      new Date(b.stats.birthtime).getTime() - new Date(a.stats.birthtime).getTime()
    );
  });
  return sorted;
};

module.exports = {
  data: new SlashCommandBuilder()
    .setName("project")
    .setDescription("used to start projecting a file")
    .addStringOption((option) => {
      option
        .setName("filename")
        .setDescription(
          "absolute path / path relative to the lasershow executable /laser_projector/rpi-lasershow"
        )
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

    const filename = interaction.options.getString("filename");

    if (filename == null) {
      files = await getSortedFiles("../ild");
      console.log(files.length);
      console.log("files:", files);

      const select = new StringSelectMenuBuilder()
        .setCustomId("project_selection")
        .setPlaceholder("What file do you want to project?")
        .setMinValues(1)
        .setMaxValues(files.length);

      files.forEach((file) => {
        select.addOptions(
          new StringSelectMenuOptionBuilder()
            .setLabel(file.filename)
            .setDescription(`${file.stats.birthtime}`)
            .setValue(file.filename)
            .setEmoji("✨")
        );
      });

      const row = new ActionRowBuilder().addComponents(select);

      await interaction.reply({
        content:
          "Choose the file you want to project from the following list or upload a new file by sending a message with it as an attachment in this channel!",
        components: [row],
      });
      return;
    }

    lasershow_sender.send("PROJECT " + filename);

    await interaction.reply("*lasershow* **<<** `PROJECT " + filename + "`");
  },
};
