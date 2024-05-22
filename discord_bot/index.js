// Require the necessary discord.js classes
const fs = require("node:fs");
const path = require("node:path");
const { Client, Collection, Events, GatewayIntentBits } = require("discord.js");

var zmq = require("zeromq"),
  lasershow_sender = zmq.socket("pub"),
  lasershow_receiver = zmq.socket("sub");

lasershow_sender.connect("tcp://localhost:5557");
lasershow_receiver.connect("tcp://localhost:5556");
lasershow_receiver.subscribe("");

const config = require("../config.json").discord;

// Create a new client instance
const client = new Client({ intents: [GatewayIntentBits.Guilds] });

client.commands = new Collection();

const foldersPath = path.join(__dirname, "commands");
const commandFolders = fs.readdirSync(foldersPath);

for (const folder of commandFolders) {
  const commandsPath = path.join(foldersPath, folder);
  const commandFiles = fs
    .readdirSync(commandsPath)
    .filter((file) => file.endsWith(".js"));
  for (const file of commandFiles) {
    const filePath = path.join(commandsPath, file);
    const command = require(filePath);
    // Set a new item in the Collection with the key as the command name and the value as the exported module
    if ("data" in command && "execute" in command) {
      client.commands.set(command.data.name, command);
    } else {
      console.log(
        `[WARNING] The command at ${filePath} is missing a required "data" or "execute" property.`
      );
    }
  }
}

client.on(Events.InteractionCreate, async (interaction) => {
  if (!interaction.isChatInputCommand()) {
    console.log(interaction);
    return;
  }


  const command = interaction.client.commands.get(interaction.commandName);

  if (!command) {
    console.error(`No command matching ${interaction.commandName} was found.`);
    console.log(interaction);
    return;
  }

  if (interaction.channelId != config.channelId) {
    interaction.reply(`all commands must be sent in <#${config.channelId}>`);
    console.log(`all commands must be sent in <#${config.channelId}>`);
    console.log(interaction);
    return;
  }

  try {
    await command.execute(interaction);
  } catch (error) {
    console.error(error);
    if (interaction.replied || interaction.deferred) {
      await interaction.followUp({
        content: "There was an error while executing this command!",
        ephemeral: true,
      });
    } else {
      await interaction.reply({
        content: "There was an error while executing this command!",
        ephemeral: true,
      });
    }
  }
});

// client.on(`message`,function(msg){
//   console.log(msg);
//   if(msg.attachments.first()){//checks if an attachment is sent
//       if(msg.attachments.first().filename === `png`){//Download only png (customize this)
//           console.log(msg.attachments.first().filename);
//       }
//   }
// });

// When the client is ready, run this code (only once).
// The distinction between `client: Client<boolean>` and `readyClient: Client<true>` is important for TypeScript developers.
// It makes some properties non-nullable.
client.once(Events.ClientReady, async (readyClient) => {
  console.log(`Ready! Logged in as ${readyClient.user.tag}`);

  const channel = await client.channels.fetch(config.channelId);

  lasershow_receiver.on("message", (msg) => {
    channel.send("*lasershow* **>>** `" + msg.toString() + "`");
  });
});

// Log in to Discord with your client's token
client.login(config.token);
