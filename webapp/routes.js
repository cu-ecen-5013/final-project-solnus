'use strict';

var express = require("express");
var router = express.Router();

var Client = require('azure-iothub').Client;
var Message = require('azure-iot-common').Message;

var deviceNames = ['ecen5713-iot-edge', 'LEDControlSvc1'];

var serviceConnStr = 'HostName=ecen5713-iot-hub.azure-devices.net;SharedAccessKeyName=service;SharedAccessKey=Gply81vOUaYpgBujYD0xhIk13xzExrKSf0EDvTW0LDA=';
var serviceClient = Client.fromConnectionString(serviceConnStr);

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
    res.render("index");
});

router.post('/command',function(req, res) {
    console.log("Device: " + req.body.device)  
    console.log("Command: " + req.body.cmd)
    console.log("Color: " + req.body.color);
    console.log("Preset: " + req.body.preset);

    var targetDevice;
    switch(req.body.device)
    {
      case 'device0':
        targetDevice = deviceNames[0];
        break;

      case 'device1':
        targetDevice = deviceNames[1];
        break;

      default:
        return;
    }

    var message;
    switch(req.body.cmd)
    {
      case 'color':
        message = "color " + req.body.color;
        break;
      case 'preset':
        message = "preset " + req.body.preset;
        break;
      case 'command':
        message = req.body.commandstr;
        break;
      default:
        console.error("Unhandled value: + req.body.cmd")
        return;
    }

    serviceClient.open(
        function (err) {
        if (err) {
          console.error('Could not connect: ' + err.message);
        } else {
          var msg = new Message(message);
          console.log('Sending message: ' + msg.getData());
          serviceClient.send(targetDevice, msg, printResultFor('send'));
        }
      });
    res.render("index", {req:req});
});


module.exports = router;