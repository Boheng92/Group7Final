var SerialPort = require("serialport");
var app = require('express')();
var http = require('http').Server(app);
var io = require('socket.io')(http);

var BROADCAST =  "000000000000ffff";

var portName = process.argv[2],
portConfig = {
	baudRate: 9600,
	parser: SerialPort.parsers.readline("\n")
};

var sp;

sp = new SerialPort.SerialPort(portName, portConfig);

app.get('/', function(req, res){
  res.sendfile('index.html');
});

io.on('connection', function(socket){
  console.log('a user connected');
  socket.on('disconnect', function(){
  });
  
  socket.on('chat message', function(msg)
  {
    var RSSIRequestPacket = 
    {
      type: C.FRAME_TYPE.ZIGBEE_TRANSMIT_REQUEST,
      destination64: BROADCAST,
      broadcastRadius: 0x01,
      options: 0x00,
      data: msg
    }
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

