// Require the necessary discord.js classes
const { exec } = require("child_process");
const { Client, Intents } = require('discord.js');
const { token } = require('./config.json');
var path = require('path');
let request = require(`request`);
let fs = require(`fs`);

// Create a new client instance
const client = new Client({ intents: [Intents.FLAGS.GUILDS] });

// When the client is ready, run this code (only once)
client.once('ready', () => {
  console.log('Ready!');
});

function download(url, filename) {
  var response = '';
  if (msg.attachments.first().filename.indexOf(`.svg`) != -1) {
    request.get(url)
      .on('error', console.error)
      .pipe(fs.createWriteStream(path.join(__dirname, '../svg/' + filename)));
      response += 'downloaded to ' + path.join(__dirname, '../svg/' + filename);

  }
  else if (msg.attachments.first().filename.indexOf(`.ild`) != -1) {
    request.get(url)
      .on('error', console.error)
      .pipe(fs.createWriteStream(path.join(__dirname, '../ild/' + filename)));
      response += 'downloaded to ' + path.join(__dirname, '../ild/' + filename);
      response += '`/project filename:' + filename + '` to start projecting the file';
  }
}

client.on(`message`, function (msg) {
  console.log('message');
  if (msg.attachments.first()) {//checks if an attachment is sent
    var response = '';
    if (msg.attachments.first().filename.indexOf(`.svg`) != -1 || msg.attachments.first().filename.indexOf(`.ild`) != -1) {//Download only png (customize this)
      response += download(msg.attachments.first().url, msg.attachments.first().filename);//Function I will show later
    }
    else {
      response += 'make sure uploaded file is either *.svg* or *.ild*';
    }
    message.lineReplyNoMention(response);
  }
});

client.on('interactionCreate', async interaction => {
  if (!interaction.isCommand()) return;

  const { commandName } = interaction;

  if (commandName === 'cmd') {
    exec(interaction.options.get('input').value, async (error, stdout, stderr) => {
      var response = '__**input:**__ `' + interaction.options.get('input').value + '`\n```';
      if (error) {
        response += `error:\n${error.message}\n`;
        console.log(`error:\n${error.message}\n`);
      }
      if (stderr) {
        response += `stderr:\n${stderr}\n`;
        console.log(`stderr:\n${stderr}\n`);
      }
      response += `stdout:\n${stdout}\n`;
      console.log(`${stdout}`);
      response += '```';
      await interaction.reply(response);
    })
  } else if (commandName === 'project') {
    console.log('project');
    //add stoping of previous projection
    console.log('../lasershow 0 ' + path.join(__dirname, '../ild/' + interaction.options.get('filename').value));
    exec('../lasershow 0 ' + path.join(__dirname, '../ild/' + interaction.options.get('filename').value), async (error, stdout, stderr) => {
      var response = '__**filename:**__ `' + interaction.options.get('filename').value + '`\n```';
      if (error) {
        response += `error:\n${error.message}\n`;
        console.log(`error:\n${error.message}\n`);
      }
      if (stderr) {
        response += `stderr:\n${stderr}\n`;
        console.log(`stderr:\n${stderr}\n`);
      }
      response += `stdout:\n${stdout}\n`;
      console.log(`${stdout}`);
      response += '```';
      await interaction.reply(response);
    })
  }
});

client.login(token);