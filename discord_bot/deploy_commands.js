const { SlashCommandBuilder } = require('@discordjs/builders');
const { REST } = require('@discordjs/rest');
const { Routes } = require('discord-api-types/v9');
const { clientId, token } = require('./config.json');

//https://discordjs.guide/interactions/registering-slash-commands.html#options

const commands = [
  new SlashCommandBuilder()
    .setName('cmd')
    .setDescription('Executes your input!')
    .addStringOption(option =>
      option.setName('input')
        .setDescription('The input to Execute')
        .setRequired(true)),

  new SlashCommandBuilder()
    .setName('user')
    .setDescription('Replies with user info!')
]
  .map(command => command.toJSON());

const rest = new REST({ version: '9' }).setToken(token);

rest.put(Routes.applicationCommands(clientId), { body: commands })
  .then(() => console.log('Successfully registered application commands.'))
  .catch(console.error);
