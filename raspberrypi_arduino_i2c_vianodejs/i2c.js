var i2c = require('i2c');
var device1 = new i2c(0x18, {device: '/dev/i2c-1', debug: false});
device1.setAddress(0x4);
device1.writeByte(0x6, function(err) { console.log("werror"); console.log(err); });
