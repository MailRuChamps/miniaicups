const readline = require('readline');

const rl = readline.createInterface({
  input: process.stdin,
  output: process.stdout
});

let handler = () => {
    let commands = ['left', 'right', 'up', 'down'];
    let command = commands[Math.floor(Math.random() * commands.length)];
    console.log(JSON.stringify({command, debug: command}));
    rl.question('', handler);
};

rl.question('', handler);
