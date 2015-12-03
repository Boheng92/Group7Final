var SerialPort = require("serialport");
var app = require('express')();
var http = require('http').Server(app);
var io = require('socket.io')(http);
var express = require('express');
var xbee_api = require('xbee-api');
var C = xbee_api.constants;
var sampleDelay = 3000;
var portName = process.argv[2];
var sp;
var path = require('path');

var BROADCAST =  "000000000000ffff";

app.set('views', path.join(__dirname, 'views'));
app.set('view engine', 'ejs');

var routes = require('./routes/index');
app.use('/', routes);
app.use(express.static(path.join(__dirname, 'public')));

var XBeeAPI = new xbee_api.XBeeAPI({
  api_mode: 2
});

//Note that with the XBeeAPI parser, the serialport's "data" event will not fire when messages are received!
var portConfig = {
	baudRate: 9600,
  parser: XBeeAPI.rawParser()
};

sp = new SerialPort.SerialPort(portName, portConfig);

io.on('connection', function(socket){
  console.log('a user connected');
  socket.on('disconnect', function(){
  });
  socket.on('chat message', function(msg){
	
	console.log('MSG');
	  
    var RSSIRequestPacket = 
    {
      type: C.FRAME_TYPE.ZIGBEE_TRANSMIT_REQUEST,
      destination64: BROADCAST,
      broadcastRadius: 0x01,
      options: 0x00,
      data: msg
    }
	
	sp.write(XBeeAPI.buildFrame(RSSIRequestPacket));
  });
});

http.listen(3000, function(){
  console.log('listening on *:3000');
});

sp.on("open", function () {
  console.log('open');
  sp.on('data', function(data) {
    console.log('data received: ' + data);
    io.emit("chat message", "An XBee says: " + data);
  });
});

