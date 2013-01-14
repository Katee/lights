var http = require('http');
var connect = require('connect');
var serialport = require("serialport");
var SerialPort = serialport.SerialPort;
var tinycolor = require("tinycolor2");
require('sugar');

var options = require('./options');

var app = connect().use(connect.static('public'));

var server = http.createServer(app).listen(options.port, function(){
  console.log("Lights started on port %s", options.port);
});

io = require('socket.io').listen(server);

var lastUpdate = 0;
var arduino = new SerialPort(options.serialDevice, {baudrate: options.baudrate, parser: serialport.parsers.readline("\n")});
arduino.on("data", function (color) {
  // send no more then 20 updates a second
  if (Date.now() - lastUpdate > 50) {
    io.sockets.in('color-updates').volatile.emit('color', color);
    lastUpdate = Date.now();
  }
});

io.sockets.on('connection', function (socket) {
  socket.join('color-updates');

  // change the mode
  socket.on('mode', function (mode) {
    var message = '*' + (mode + '======').first(6);
    arduino.write(message);
  });

  // change the color
  socket.on('color', function (color) {
    var color2 = tinycolor(color);
    arduino.write(color2.toHexString());
  });
});
