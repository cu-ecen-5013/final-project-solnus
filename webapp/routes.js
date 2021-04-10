'use strict';

var express = require("express");
var router = express.Router();

var Client = require('azure-iothub').Client;
var Message = require('azure-iot-common').Message;
var connectionString = 'HostName=ecen5713-iot-hub.azure-devices.net;SharedAccessKeyName=service;SharedAccessKey=Gply81vOUaYpgBujYD0xhIk13xzExrKSf0EDvTW0LDA=';
var targetDevice = 'ecen5713-iot-edge';

var serviceClient = Client.fromConnectionString(connectionString);
function printResultFor(op) {
    return function printResult(err, res) {
      if (err) console.log(op + ' error: ' + err.toString());
      if (res) console.log(op + ' status: ' + res.constructor.name);
    };
}

function receiveFeedback(err, receiver){
  receiver.on('message', function (msg) {
    console.log('Feedback message:')
    console.log(msg.getData().toString('utf-8'));
  });
}

router.get("/",function(req,res){
    console.log("Hello World");
    res.render("index");
});

router.post('/status',function(req, res) {
    var cmd = (req.body.cmd) + "";
    console.log("receive " + cmd);
    serviceClient.open(
        function (err) {
        if (err) {
          console.error('Could not connect: ' + err.message);
        } else {
          var message = new Message(cmd);
          console.log('Sending message: ' + message.getData());
          serviceClient.send(targetDevice, message, printResultFor('send'));
        }
      });
    res.render("index");
});

router.post('/colorPick',function(req, res) {
    var clr = (req.body.clr) + "";
    console.log("receive color" + clr);
    serviceClient.open(
        function (err) {
        if (err) {
          console.error('Could not connect: ' + err.message);
        } else {
          var message = new Message(clr);
          console.log('Sending message: ' + message.getData());
          serviceClient.send(targetDevice, message, printResultFor('send'));
        }
      });
    res.render("index");
});


module.exports = router;