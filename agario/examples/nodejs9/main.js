var readline = require('readline');

var iface = readline.createInterface({
    input: process.stdin,
    output: process.stdout,
    terminal: false
});

var config = null;
iface.on('line', function (line) {
	if (config === null) {
		config = JSON.parse(line);
		return;
	}
	var parsed = JSON.parse(line);
	var command = JSON.stringify(onTick(parsed));
	console.log(command);
});

function onTick(parsed) {
	if (parsed.Mine.length > 0) {
		var mine = parsed.Mine[0];
		var goal = findFood(parsed.Objects);
		if (!!goal) {
			return {X: goal.X, Y: goal.Y};
		}
		return {X: 0, Y: 0, Debug: 'No food'};
	}
	return {X: 0, Y: 0, Debug: 'Died'};
}

function findFood(objects) {
	var food = objects.filter(function (obj) {
		return obj.T == 'F';
	});
	if (food.length > 0) {
		return food[0];
	}
	return null;
}