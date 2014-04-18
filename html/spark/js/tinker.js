var rgbX = 175.6;
var rgbY = 83.3;
var analogX = 100;
var analogXT = 70;
var digitalX = 250;
var digitalXT = 265;
var startY = 132;
var pinDistance = 22;
var varticalPinPos = [132, 154, 176, 198, 220, 241, 261, 283];
var onColor = "#11A11F";
var disconnectColor = "#F0981D";
var connectedColor = "#14BFCF";
var connectingColor = "#3914CF";
var rgbColor = connectingColor;
var errorColor = "#F000000";
/*
 * counter to keep track of number of times we have waited for HB
 */
var howMany;
var connectedColor = "#2FD1D7";
var connected = false;

var rgbOnOff = true;

var digitalPinStates = [false, false, false, false, false, false, false, false];
/**
 * input / out 
 * true means input
 */
var analogPinIO = [true, true, true, true, true, true, true, true];
/**
 * input / output
 * true means input
 */
var digitalPinIO = [true, true, true, true, true, true, true, true];
var analogPinValues = [-1, -1, -1, -1, -1, -1, -1, -1];
var inputText = "I";
var outputText = "O";
if ( typeof String.prototype.startsWith != 'function') {
 // see below for better implementation!
 String.prototype.startsWith = function(str) {
  return this.indexOf(str) == 0;
 };
}
function connect() {
 if ("WebSocket" in window) {
  console.log("Connecting!");
  // Let us open a web socket
  ws = new WebSocket("ws://192.168.1.8:2525");
  //var ws = new WebSocket("ws://192.168.1.8:2525");
  ws.onopen = function() {
   // Web Socket is connected, send data using send()
   console.log("Connected");
   rgbColor = connectedColor;
   howMany=0;
   ws.send("/digitalreadall");

  };
  ws.onmessage = function(evt) {
   var received_msg = evt.data;
   console.log("got msg: " + received_msg);
   if (received_msg == "HB") {
    howMany=0;
    rgbColor=connectedColor;
   } else if (received_msg.startsWith("analoglWrote") || received_msg.startsWith("Analog,")) {
    var what = received_msg.split(",")[1].trim();
    var pin = what.charAt(0);
    updateAnalogPin(pin, what.substring(2,what.lenght));
   } else if (received_msg.startsWith("digitalWrote") || received_msg.startsWith("Digital,")) {
    var what = received_msg.split(",")[1].trim();
    var pin = what.charAt(0);
    updateDigialPin(pin, what.charAt(2) == "1");
   } else if (received_msg.startsWith("DigitalAll")) {
    var pins = received_msg.split(",");
    for (var i = 1; i < pins.length; i++) {
     var test = pins[i].split(":");
     var pin = test[0].trim().charAt(1);
     var value = test[1].trim();
     console.log(pin + ":" + value);
     updateDigialPin(pin, value == "1");
    }
    //Do something
   }

  };
  ws.onclose = function() {
   // websocket is closed.
   rgbColor = disconnectColor;
   console.log("Disconnected.");
  };
 } else {
  // The browser doesn't support WebSocket
  alert("WebSocket NOT supported by your Browser!");
 }
 function WebSocketTest() {
  ws.send("/pos");
 }

}

function doAnalogPin(pin, value, input) {
 if (input) {
  ws.send("/analogread?A" + pin);
 } else {
  ws.send("/analogread?A" + pin + "=" + value);
 }
}
function updateAnalogPin(pin, value) {  
 $('#core').getLayer('AT' + pin).text =  value +":"; 
 $('#core').drawLayers();
}

function doDigitalPin(pin, on, input) {
 if (input) {
  ws.send("/digitalread?D" + pin);
 } else {
  ws.send("/digitalwrite?D" + pin + "=" + ( on ? "HIGH" : "LOW"));
 }
}

function updateDigialPin(pin, value) {
 digitalPinStates[pin] = value;
 for ( i = 0; i < digitalPinStates.length; i++) {
  // $('#core').getLayer('DL' + pin).visible = digitalPinStates[i];
  $('#core').getLayer('DT' + pin).text = ":" + ( value ? 1 : 0);
 }
 $('#core').drawLayers();
}

// function updateAnalogPin(pin, value) {
 // digitalPinStates[pin] = value;
 // for ( i = 0; i < analogPinValues.length; i++) {
  // $('#core').getLayer('A' + pin).visible = (analogPinValues[i] != -1);
  // if (analogPinValues[i] != -1) {
   // //NASSER draw text
  // }
  // $('#core').drawLayers();
 // }
// }

function initPins() {
 for ( i = 0; i < 8; i++) {
  $('#core').drawRect({
   layer : true,
   strokeStyle : '#c33',
   strokeWidth : 1,
   x : analogX,
   name : "A" + (7 - i),
   y : varticalPinPos[i],
   width : 16,
   height : 16,
   cornerRadius : 5,
   click : function(layer) {
    var id = layer.name.charAt(1);
    if (analogPinIO[id]) {     
     doAnalogPin(id, -1, analogPinIO[id]);
    }
   }
  });

  // $("#core").drawEllipse({
  // index : 2,
  // layer : true,
  // name : "DL" + (7 - i),
  // visible : false,
  // groups : "[digitalPins]",
  // fillStyle : onColor,
  // x : digitalX,
  // y : varticalPinPos[i],
  // width : 15,
  // height : 15
  //
  // });

  $('#core').drawRect({
   layer : true,
   strokeStyle : '#c33',
   strokeWidth : 1,
   x : digitalX,
   y : varticalPinPos[i],
   width : 16,
   height : 16,
   name : "D" + (7 - i),
   cornerRadius : 5,
   click : function(layer) {
    var id = layer.name.charAt(1);
    var state = !digitalPinStates[id];
    digitalPinStates[id] = state;
    doDigitalPin(id, state, false);
   }
  });

 }
}

function initPinTexts() {
 for ( i = 0; i < 8; i++) {
  $("#core").drawText({
   index : 1,
   layer : true,
   visible : true,
   fontSize : '10',
   fontFamily : 'Verdana, sans-serif',
   text : "",
   align : 'right',
   respectAlign : true,
   name : "AT" + (7 - i),
   groups : "[analogPins]",
   fillStyle : "black",
   x : analogXT,
   y : varticalPinPos[i] - 5,
   fromCenter : false,
   // rotate : '45',
   click : function(layer) {
    // Click a star to spin it
    $(this).animateLayer(layer, {
    // rotate : '-=270',
    });
   }
  });

  $("#core").drawText({
   index : 1,
   layer : true,
   visible : true,
   fontSize : '10',
   fontFamily : 'Verdana, sans-serif',
   text : "",
   name : "DT" + (7 - i),
   groups : "[analogPins]",
   fillStyle : "black",
   x : digitalXT,
   y : varticalPinPos[i]-5,
   fromCenter : false,
   // rotate : '-45',
   click : function(layer) {
    // Click a star to spin it
    $(this).animateLayer(layer, {
     // rotate: '-=45',
    });
   }
  });

 }
}

function initRGB() {
 $("#core").drawEllipse({
  index : 1,
  layer : true,
  name : "RGB",
  visible : false,
  fillStyle : disconnectColor,
  x : rgbX,
  y : rgbY,
  width : 19,
  height : 19
 });
}

function init() {
 initPins();
 initPinTexts();
 initRGB();
 $("#core").jCanvas.eventHooks.add = function() {
  console.log("cool");
 };
 setInterval(function() {
  beat();
 }, 1000);
 connect();
}

function beat() {
 if(howMany>=40) {
  howMany=0;
  rgbColor=disconnectColor;
 }
 $('#core').getLayer('RGB').fillStyle = rgbColor;
 $('#core').getLayer('RGB').visible = rgbOnOff;
 $('#core').drawLayer('RGB');
 rgbOnOff = !rgbOnOff;
 $('#core').drawLayers();
 howMany++;
}

function digitalOnOff(pin, on) {
 $('#core').getLayer('DL' + pin).visible = on;
 //$('#core').drawLayer('D' + pin);
 $('#core').drawLayers();
}
