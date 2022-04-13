// Require the necessary discord.js classes
const { exec } = require("child_process");
const { Client, Intents, MessageActionRow, MessageButton, MessageSelectMenu } = require('discord.js');
const { token } = require('../config.json').discord;
var path = require('path');
let request = require(`request`);
let fs = require(`fs`);

// Create a new client instance
const client = new Client({ intents: [Intents.FLAGS.GUILDS, "GUILDS", "GUILD_MESSAGES", "DIRECT_MESSAGES"] });

// When the client is ready, run this code (only once)
client.once('ready', () => {
  console.log('Ready!');
});

function getLastChangedFile() {
  var lasttime = 0;
  var lastpath = '';

  var fs = require('fs');
  const ildFiles = fs.readdirSync('../ild/').filter(file => file.endsWith('.ild'));

  ildFiles.forEach(function (file) {
    // console.log(path.join(__dirname + '/ild/', file));
    stats = fs.statSync(path.join(__dirname, '../ild/' + file), true);
    if (stats.mtimeMs > lasttime) {
      lasttime = stats.mtimeMs;
      lastpath = path.join(__dirname + '/ild/', file);
    }
  })
  return lastpath;
}

function getnewrow() {
  const ildFiles = fs.readdirSync('../ild/').filter(file => file.endsWith('.ild'));

  var options = [];

  ildFiles.forEach(function (file) {
    stats = fs.statSync(path.join(__dirname, '../ild/' + file), true);
    options.push({
      label: file,
      description: stats.mtime.toString(),
      value: path.join(__dirname, '../ild/' + file)
    })
  })

  const newrow = new MessageActionRow()
    .addComponents(
      new MessageSelectMenu()
        .setCustomId('select')
        .setPlaceholder('project ILDA file:')
        .addOptions(options)
    );
  return newrow;
}

function download(url, filename, msg) {
  var response = '__**user:**__ <@' + msg.author.id + '>';
  if (filename.endsWith('.svg')) {
    request.get(url)
      .on('error', console.error)
      .pipe(fs.createWriteStream(path.join(__dirname, '../svg/' + filename)));
    response += 'downloaded to `' + path.join(__dirname, '../svg/' + filename + '`');

    exec('python3 ' + path.join(__dirname, '../svg2ild.py') + ' ' + path.join(__dirname, '../svg/' + filename) + ' ' + path.join(__dirname, '../ild/' + filename.substring(0, filename.indexOf('.'))) + '.ild', (error, stdout, stderr) => {
      var response = '__**user:**__ <@' + msg.author.id + '>\n__**file:**__ `' + path.join(__dirname, '../ild/' + filename.substring(0, filename.indexOf('.'))) + '.ild`\n';
      if (error) {
        response += 'error:\n`' + error.message + '`\n';
        console.log(`error:\n${error.message}\n`);
      }
      if (stderr) {
        response += 'stderr:\n`' + stderr + '`\n';
        console.log(`stderr:\n${stderr}\n`);
      }
      response += 'stdout:\n`' + stdout + '`\n';
      console.log(`${stdout}`);

      const ildFiles = fs.readdirSync('../ild/').filter(file => file.endsWith('.ild'));
      var latestfilepaths = [];
      latestfilepaths.length = ildFiles.length;
      var latestfiles = [];
      latestfiles.length = ildFiles.length;

      ildFiles.forEach(function (file) {
        // console.log(path.join(__dirname + '/ild/', file));
        stats = fs.statSync(path.join(__dirname, '../ild/' + file), true);
        var biggerThan = 0;

        ildFiles.forEach(function (comparefile) {
          comparestats = fs.statSync(path.join(__dirname, '../ild/' + comparefile), true);
          if (stats.mtimeMs > comparestats.mtimeMs) {
            biggerThan += 1; //never used ++ in js, so i feel safer this way
          }
        })

        latestfilepaths[ildFiles.length - biggerThan - 1] = path.join(__dirname, '../ild/' + file);
        latestfiles[ildFiles.length - biggerThan - 1] = file;
      })

      const row = new MessageActionRow()
        .addComponents([
          new MessageButton()
            .setCustomId('last')
            .setLabel('Project last uploaded file')
            .setStyle('PRIMARY'),
          new MessageButton()
            .setCustomId('ls')
            .setLabel('Show all available ILDA files')
            .setStyle('SECONDARY'),
        ]);
      try {
        const newrow = getnewrow();

        if (response.length < 2000) msg.reply({ content: response, components: [row, newrow] });
        else {
          fs.writeFileSync('./response.txt', response);
          msg.reply({
            content: 'response longer than 2000 characters - included in response.txt', components: [row, newrow], files: ["./response.txt"]
          })
        }
      } catch (error) {
        console.log('catch: ' + error)
        if (response.length < 2000) msg.reply({ content: response, components: [row, newrow] });
        else {
          fs.writeFileSync('./response.txt', response);
          msg.reply({
            content: 'response longer than 2000 characters - included in response.txt', components: [row], files: ["./response.txt"]
          })
        }
      }
    })

    response += '\nnew path is `' + path.join(__dirname, '../ild/' + filename.substring(0, filename.indexOf('.'))) + '.ild`';
  }
  else if (filename.endsWith('.ild')) {
    request.get(url)
      .on('error', console.error)
      .pipe(fs.createWriteStream(path.join(__dirname, '../ild/' + filename)));
    response += 'downloaded to `' + path.join(__dirname, '../ild/' + filename + '`');
  }

  return response;
}

client.on(`messageCreate`, function (msg) {
  if (msg.author.id != '926432210683310141') {
    if (msg.attachments.first()) {//checks if an attachment is sent
      var response = '';
      if (msg.attachments.first().name.endsWith('.svg') || msg.attachments.first().name.endsWith('.ild')) {
        response += download(msg.attachments.first().url, msg.attachments.first().name, msg);//Function I will show later
      }
      else {
        response += 'make sure uploaded file is either *.svg* or *.ild*';
      }

      const ildFiles = fs.readdirSync('../ild/').filter(file => file.endsWith('.ild'));
      var latestfilepaths = [];
      latestfilepaths.length = ildFiles.length;
      var latestfiles = [];
      latestfiles.length = ildFiles.length;

      ildFiles.forEach(function (file) {
        // console.log(path.join(__dirname + '/ild/', file));
        stats = fs.statSync(path.join(__dirname, '../ild/' + file), true);
        var biggerThan = 0;

        ildFiles.forEach(function (comparefile) {
          comparestats = fs.statSync(path.join(__dirname, '../ild/' + comparefile), true);
          if (stats.mtimeMs > comparestats.mtimeMs) {
            biggerThan += 1; //never used ++ in js, so i feel safer this way
          }
        })

        latestfilepaths[ildFiles.length - biggerThan - 1] = path.join(__dirname, '../ild/' + file);
        latestfiles[ildFiles.length - biggerThan - 1] = file;
      })

      const row = new MessageActionRow()
        .addComponents([
          new MessageButton()
            .setCustomId('last')
            .setLabel('Project last uploaded file')
            .setStyle('PRIMARY'),
          new MessageButton()
            .setCustomId('ls')
            .setLabel('Show all available ILDA files')
            .setStyle('SECONDARY'),
        ]);
      try {
        const newrow = getnewrow();

        if (response.length < 2000) msg.reply({ content: response, components: [row, newrow] });
        else {
          fs.writeFileSync('./response.txt', response);
          msg.reply({
            content: 'response longer than 2000 characters - included in response.txt', components: [row, newrow], files: ["./response.txt"]
          })
        }
      } catch (error) {
        console.log('catch: ' + error);
        if (response.length < 2000) msg.reply({ content: response, components: [row] });
        else {
          fs.writeFileSync('./response.txt', response);
          msg.reply({
            content: 'response longer than 2000 characters - included in response.txt', components: [row], files: ["./response.txt"]
          })
        }
      }
    }
  }
});

client.on('interactionCreate', async interaction => {
  if (interaction.isButton()) {
    if (interaction.customId == 'ls') {
      exec('cd ' + path.join('../ild') + ' && ls', (error, stdout, stderr) => {
        console.log('exec');
        var response = '__**user:**__ <@' + interaction.user.id + '>\n```';
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

        const ildFiles = fs.readdirSync('../ild/').filter(file => file.endsWith('.ild'));
        var latestfilepaths = [];
        latestfilepaths.length = ildFiles.length;
        var latestfiles = [];
        latestfiles.length = ildFiles.length;

        ildFiles.forEach(function (file) {
          // console.log(path.join(__dirname + '/ild/', file));
          stats = fs.statSync(path.join(__dirname, '../ild/' + file), true);
          var biggerThan = 0;

          ildFiles.forEach(function (comparefile) {
            comparestats = fs.statSync(path.join(__dirname, '../ild/' + comparefile), true);
            if (stats.mtimeMs > comparestats.mtimeMs) {
              biggerThan += 1; //never used ++ in js, so i feel safer this way
            }
          })

          latestfilepaths[ildFiles.length - biggerThan - 1] = path.join(__dirname, '../ild/' + file);
          latestfiles[ildFiles.length - biggerThan - 1] = file;
        })

        const row = new MessageActionRow()
          .addComponents([
            new MessageButton()
              .setCustomId('last')
              .setLabel('Project last uploaded file')
              .setStyle('PRIMARY'),
            new MessageButton()
              .setCustomId('ls')
              .setLabel('Show all available ILDA files')
              .setStyle('SECONDARY'),
          ]
          );
        try {
          const newrow = getnewrow();

          interaction.reply({ content: response, components: [row, newrow] });
        } catch (error) {
          console.log('catch: ' + error)
          interaction.reply({ content: response, components: [row] });
        }
      })
    }

    else if (interaction.customId == 'last') {
      // const row = new MessageActionRow()
      //   .addComponents(
      //     new MessageButton()
      //       .setCustomId('last')
      //       .setLabel('Project last uploaded file')
      //       .setStyle('PRIMARY'),
      //   );

      // interaction.reply({ content: path.join('../lasershow') + ' 0 ' + getLastChangedFile(), components: [row] });
      exec(path.join('../lasershow') + ' 0 ' + getLastChangedFile(), (error, stdout, stderr) => {
        console.log('exec');
        var response = '__**user:**__ <@' + interaction.user.id + '>\n__**filename:**__ `' + getLastChangedFile() + '`\n```';
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

        const ildFiles = fs.readdirSync('../ild/').filter(file => file.endsWith('.ild'));
        var latestfilepaths = [];
        latestfilepaths.length = ildFiles.length;
        var latestfiles = [];
        latestfiles.length = ildFiles.length;

        ildFiles.forEach(function (file) {
          // console.log(path.join(__dirname + '/ild/', file));
          stats = fs.statSync(path.join(__dirname, '../ild/' + file), true);
          var biggerThan = 0;

          ildFiles.forEach(function (comparefile) {
            comparestats = fs.statSync(path.join(__dirname, '../ild/' + comparefile), true);
            if (stats.mtimeMs > comparestats.mtimeMs) {
              biggerThan += 1; //never used ++ in js, so i feel safer this way
            }
          })

          latestfilepaths[ildFiles.length - biggerThan - 1] = path.join(__dirname, '../ild/' + file);
          latestfiles[ildFiles.length - biggerThan - 1] = file;
        })

        const row = new MessageActionRow()
          .addComponents([
            new MessageButton()
              .setCustomId('last')
              .setLabel('Project last uploaded file')
              .setStyle('PRIMARY'),
            new MessageButton()
              .setCustomId('ls')
              .setLabel('Show all available ILDA files')
              .setStyle('SECONDARY'),
          ]
          );
        try {
          const newrow = getnewrow();

          interaction.reply({ content: response, components: [row, newrow] });
        } catch (error) {
          console.log('catch: ' + error)
          interaction.reply({ content: response, components: [row] });
        }
      })
    }
  }
  else if (interaction.isSelectMenu()) {
    console.log(path.join(__dirname, '../lasershow') + ' 0 ' + interaction.values[0]);
    exec(path.join(__dirname, '../lasershow') + ' 0 ' + interaction.values[0], async (error, stdout, stderr) => {
      var response = '__**user:**__ <@' + interaction.user.id + '>\n__**filename:**__ `' + interaction.values[0] + '`\n```';
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
      const ildFiles = fs.readdirSync('../ild/').filter(file => file.endsWith('.ild'));
      var latestfilepaths = [];
      latestfilepaths.length = ildFiles.length;
      var latestfiles = [];
      latestfiles.length = ildFiles.length;

      ildFiles.forEach(function (file) {
        // console.log(path.join(__dirname + '/ild/', file));
        stats = fs.statSync(path.join(__dirname, '../ild/' + file), true);
        var biggerThan = 0;

        ildFiles.forEach(function (comparefile) {
          comparestats = fs.statSync(path.join(__dirname, '../ild/' + comparefile), true);
          if (stats.mtimeMs > comparestats.mtimeMs) {
            biggerThan += 1; //never used ++ in js, so i feel safer this way
          }
        })

        latestfilepaths[ildFiles.length - biggerThan - 1] = path.join(__dirname, '../ild/' + file);
        latestfiles[ildFiles.length - biggerThan - 1] = file;
      })

      const row = new MessageActionRow()
        .addComponents([
          new MessageButton()
            .setCustomId('last')
            .setLabel('Project last uploaded file')
            .setStyle('PRIMARY'),
          new MessageButton()
            .setCustomId('ls')
            .setLabel('Show all available ILDA files')
            .setStyle('SECONDARY'),
        ]);
      try {
        const newrow = getnewrow();

        interaction.reply({ content: response, components: [row, newrow] });
      } catch (error) {
        console.log('catch: ' + error)
        interaction.reply({ content: response, components: [row] });
      }
    })
  }
  else if (interaction.isCommand()) {

    const { commandName } = interaction;

    if (commandName === 'cmd') {
      exec(interaction.options.get('input').value, async (error, stdout, stderr) => {
        var response = '__**user:**__ <@' + interaction.user.id + '>\n__**input:**__ `' + interaction.options.get('input').value + '`\n```';
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
        const ildFiles = fs.readdirSync('../ild/').filter(file => file.endsWith('.ild'));
        var latestfilepaths = [];
        latestfilepaths.length = ildFiles.length;
        var latestfiles = [];
        latestfiles.length = ildFiles.length;

        ildFiles.forEach(function (file) {
          // console.log(path.join(__dirname + '/ild/', file));
          stats = fs.statSync(path.join(__dirname, '../ild/' + file), true);
          var biggerThan = 0;

          ildFiles.forEach(function (comparefile) {
            comparestats = fs.statSync(path.join(__dirname, '../ild/' + comparefile), true);
            if (stats.mtimeMs > comparestats.mtimeMs) {
              biggerThan += 1; //never used ++ in js, so i feel safer this way
            }
          })

          latestfilepaths[ildFiles.length - biggerThan - 1] = path.join(__dirname, '../ild/' + file);
          latestfiles[ildFiles.length - biggerThan - 1] = file;
        })

        const row = new MessageActionRow()
          .addComponents(
            new MessageButton()
              .setCustomId('last')
              .setLabel('Project last uploaded file')
              .setStyle('PRIMARY'),
          );
        try {
          const newrow = getnewrow();

          interaction.reply({ content: response, components: [row, newrow] });
        } catch (error) {
          console.log('catch: ' + error)
          interaction.reply({ content: response, components: [row] });
        }
      })
    } else if (commandName === 'project') {
      console.log('project');
      //add stoping of previous projection
      console.log(path.join(__dirname, '../lasershow') + ' 0 ' + path.join(__dirname, '../ild/' + interaction.options.get('filename').value));
      exec(path.join(__dirname, '../lasershow') + ' 0 ' + path.join(__dirname, '../ild/' + interaction.options.get('filename').value), async (error, stdout, stderr) => {
        var response = '__**user:**__ <@' + interaction.user.id + '>\n__**filename:**__ `' + interaction.options.get('filename').value + '`\n```';
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
        const ildFiles = fs.readdirSync('../ild/').filter(file => file.endsWith('.ild'));
        var latestfilepaths = [];
        latestfilepaths.length = ildFiles.length;
        var latestfiles = [];
        latestfiles.length = ildFiles.length;

        ildFiles.forEach(function (file) {
          // console.log(path.join(__dirname + '/ild/', file));
          stats = fs.statSync(path.join(__dirname, '../ild/' + file), true);
          var biggerThan = 0;

          ildFiles.forEach(function (comparefile) {
            comparestats = fs.statSync(path.join(__dirname, '../ild/' + comparefile), true);
            if (stats.mtimeMs > comparestats.mtimeMs) {
              biggerThan += 1; //never used ++ in js, so i feel safer this way
            }
          })

          latestfilepaths[ildFiles.length - biggerThan - 1] = path.join(__dirname, '../ild/' + file);
          latestfiles[ildFiles.length - biggerThan - 1] = file;
        })

        const row = new MessageActionRow()
          .addComponents(
            new MessageButton()
              .setCustomId('last')
              .setLabel('Project last uploaded file')
              .setStyle('PRIMARY'),
          );
        try {
          const newrow = getnewrow();

          interaction.reply({ content: response, components: [row, newrow] });
        } catch (error) {
          console.log('catch: ' + error)
          interaction.reply({ content: response, components: [row] });
        }
      })
    }
  }
});

client.login(token);