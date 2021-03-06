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
//for MonogoDB
var MongoClient;
var url;
var mongo = require('mongodb');
MongoClient = require('mongodb').MongoClient, assert = require('assert');
url = 'mongodb://localhost:27017/ch6Node';

var i2c = require('i2c');
var device1 = new i2c(0x18, {device: '/dev/i2c-1', debug: false});
device1.setAddress(0x4);




io.on('connection', function(socket){
  console.log('a user connected');
  socket.on('disconnect', function(){
  }); 
  
  socket.on('forward', function(msg){

    console.log('forward');

    device1.writeByte(0x46, function(err) {console.log("werror"); console.log(err);});
        
  });
  socket.on('backward', function(msg){

    console.log('backward');

    device1.writeByte(0x42, function(err) {console.log("werror"); console.log(err);});
    
    
  });
  socket.on('right', function(msg){

    console.log('right');

    device1.writeByte(0x52, function(err) {console.log("werror"); console.log(err);});
        
  });
  socket.on('left', function(msg){

    console.log('left');

    device1.writeByte(0x4C, function(err) {console.log("werror"); console.log(err);});
    
    
  });
  socket.on('middle', function(msg){

    console.log('middle');

    device1.writeByte(0x4D, function(err) {console.log("werror"); console.log(err);});

  });
  socket.on('auto', function(msg){

    console.log('auto');

    device1.writeByte(0x41, function(err) {console.log("werror"); console.log(err);});
    
  });
  socket.on('manual', function(msg){

    console.log('manual');

    device1.writeByte(0x4E, function(err) {console.log("werror"); console.log(err);});
    
  });
   socket.on('stop', function(msg){

    console.log('stop');

    device1.writeByte(0x53, function(err) {console.log("werror"); console.log(err);});
    
  });
});


http.listen(3000, function(){
  console.log('listening on *:3000');
});


app.set('views', path.join(__dirname, 'views'));
app.set('view engine', 'ejs');


var routes = require('./routes/index');
app.use('/', routes);app.use(express.static(path.join(__dirname, 'public')));
app.use('/', express.static(__dirname + '/public'));



var XBeeAPI = new xbee_api.XBeeAPI({
  api_mode: 2
});

//Note that with the XBeeAPI parser, the serialport's "data" event will not fire when messages are received!
var portConfig = {
	baudRate: 9600,
  parser: XBeeAPI.rawParser()
};


sp = new SerialPort.SerialPort(portName, portConfig);


//Create a packet to be sent to all other XBEE units on the PAN.
// The value of 'data' is meaningless, for now.
var RSSIRequestPacket = {
  type: C.FRAME_TYPE.ZIGBEE_TRANSMIT_REQUEST,
  destination64: "000000000000ffff",
  broadcastRadius: 0x01,
  options: 0x00,
  data: "test"
}

var requestRSSI = function(){
  sp.write(XBeeAPI.buildFrame(RSSIRequestPacket));
}

sp.on("open", function () {
  console.log('open');
  requestRSSI();
  setInterval(requestRSSI, sampleDelay);
  
});

//Variables for find query.
var update = [0, 0, 0, 0];
var rssis = new Array(0, 0, 0, 0);

var res_X = -999.9;
var res_Y = -999.9;

XBeeAPI.on("frame_object", function(frame) {
  if (frame.type == 144){
    console.log("Beacon ID: " + frame.data[1] + ", RSSI: " + (frame.data[0]));
	io.emit("rssi message", frame.data[1] + " " + frame.data[0]);
	//distance = Math.pow(10.0,((A + rssi)/(10.0*n)));
	var range = 200;

	var tempID = frame.data[1];
	var tempRSSI = frame.data[0];
	
	var minDist = 65535;


	update[tempID] = 1;
	//console.log(update[0]);
	rssis[tempID] = tempRSSI;
	
	if( ( update[0] == 1 ) && ( update[1] == 1) && ( update[2] == 1) && ( update[3] == 1) ){
		
		var i;
		for( i = 0; i < 4; i++){
			update[i] = 0;
		}
		MongoClient.connect(url, function(err, db) {
			assert.equal(null, err);
			
			var collection = db.collection('ch6Collection');
			var cursor =collection.find({ $and: [{"RSSI0": { $lt: (rssis[0] + range), 
				$gt: (rssis[0] - range)}}, {"RSSI1" : {$lt: (rssis[1] + range), 
				$gt: (rssis[1] - range)}},{"RSSI2": { $lt: (rssis[2] + range), 
				$gt: (rssis[2] - range)}},{"RSSI3": { $lt: (rssis[3] + range), 
				$gt: (rssis[3] - range)}}]});				
			cursor.sort({X: -1});			
			cursor.each(function(err, doc) 
			{					
				if (err) 
				{
					console.log(err);
				} 
				if (doc != null) 
				{
					var tempRSSI = new Array();
									
					tempRSSI[0] = doc.RSSI0;
					tempRSSI[1] = doc.RSSI1;
					tempRSSI[2] = doc.RSSI2;
					tempRSSI[3] = doc.RSSI3;
					
					
					var tempDist = (rssis[0] - tempRSSI[0])*(rssis[0] - tempRSSI[0]) + 
						(rssis[1] - tempRSSI[1])*(rssis[1] - tempRSSI[1]) + 
						(rssis[2] - tempRSSI[2])*(rssis[2] - tempRSSI[2]) + 
						(rssis[3] - tempRSSI[3])*(rssis[3] - tempRSSI[3]);
					
	
					if(tempDist < minDist)
					{
						minDist = tempDist;

						res_X = doc.X;
						res_Y = doc.Y;
					}
				}
				else
				{	
					if( parseInt(res_X) == 9 && parseInt(res_Y) < 20 && parseInt(res_Y) > 13)
					{
						device1.writeByte(0x58, function(err) { console.log("werror"); console.log(err); });
					}
					if(parseInt(res_X) < 10){
						res_X = '0' + res_X;
					}
					if(parseInt(res_Y) < 10){
						res_Y = '0' + res_Y;
					}
					var msg = '[' + res_X + ']' + '[' + res_Y + ']';
					console.log("coordinate msg" + msg);						
					io.emit("Coordinate Message", msg);
				}
			
				db.close();
			});			
		});	
	}
  }
});
