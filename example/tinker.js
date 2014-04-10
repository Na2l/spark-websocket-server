/*******************************************************************************
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, #EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF #MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO #EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR #OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, #ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER #DEALINGS IN
* THE SOFTWARE.
* Copyright (c) 2014 NaAl (h20@alocreative.com)
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*/

var digital = [ 'D0', 'D1', 'D2', 'D3'];
var analog = [ 'A0', 'A1', 'A2', 'A3'];
var cmdArray =[ 'digitalwrite', 'digitalread', /*'analogwrite', 'analogread' */];
var pinIndex=0;
var cmdIndex=0;
var state=true;
var WebSocket = require('ws')
  , ws = new WebSocket('ws://<IP>:<PORT>');

ws.on('open', function() {
   console.log('connected');
    
});
ws.on('close', function(message) {
console.log('closed');

});
ws.on('message', function(message) {
console.log('received: %s', message);

});
function test() {

   setInterval(function() {
	if(pinIndex>=digital.length) {
	pinIndex=0;
	state=!state;
cmdIndex++;
	}
if(cmdIndex>=cmdArray.length) {
	cmdIndex=0
pinIndex=0;
	}
	var sendThis='/'+cmdArray[cmdIndex]+'?';
	if(cmdIndex<2) {
sendThis+=digital[pinIndex]+'='+(state?'HIGH':'LOW');
	} else {
sendThis+=analog[pinIndex];
	}

	

   console.log('sending: '+sendThis);
         ws.send(sendThis);
pinIndex++;
}, 2000);

}
test();
