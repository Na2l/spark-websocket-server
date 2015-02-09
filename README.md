Simple "local" (cloud disabled) websocket server implementation for the sparkcoe (https://www.spark.io/) ported from https://github.com/brandenhall/Arduino-Websocket.git.
The server, by default handles up-to 4 simultaneous clients.

In the demo below, On the right, html5 +WebSocket that is connected to a sparkcore, on the left you see the serial output from the core. You can change the state of the digital pins by clicking on the pins, clicking on the analog pins will read the analog values. 



[![ScreenShot](https://i1.ytimg.com/vi/B886_m16s6s/1.jpg?time=1399011012908)](https://www.youtube.com/watch?v=B886_m16s6s&feature=youtu.be)


NOTE, I have only tested this with local builds, so try it with sparkulator on your own risk :).

To build, simply run make in core-firmware/build. 

The applicaiton.cpp is a simplified version of the tinker firmware that I used for my testing and currently can handle: digitalread, digitalwrite, analogread and analogwrite. When I get a chance I will add something like Spark.publish... 
applicaiton.cpp requires Serial connection, the core will halt the execution in the setup (the RGB led will flash blue, green and pink) until the serial communication is established. 

example syntax to how communicate with tinker firmware

/digitalwrite?D3=HIGH
/digitalread?D3

If you develop on Linux and if you have nodejs installed read on.
After flashing the core for the first time with, you can use "doit" in the core-firmware/build folder, "doit" will put the core in the dfu mode, compile, and flash the core again.

tinker.js in the example folder can be used for demo and or testing, currently it has loop that will turn on/off the first four digital outputs on the core.
Note you have to change the <IP>:<PORT> in the tinker.js


