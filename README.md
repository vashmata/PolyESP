# PolyESP
Door opening project using an ESP8266 with an Android app.

---------------
How it'll work:
---------------

An ESP microcontroller (ESP8266 or possibly ESP32) will use its WiFi capabilities to communicate with an Android app. The ESP's flash memory is used to host a simple website, as well as user data. The ESP will be able to activate a door unlocking system consisting of a motor (type TBD) connected to a mechanism that will turn the door's handle.

The Android app will be able to signal the ESP to open the door, but only if the phone's owner is registered in the ESP's user database and has permission to unlock it. If the phone's owner has a higher permission level, they will also be able to add or modify user data and permissions through the app.

The app should also be able to display the door's current status - whether it's locked, unlocking, or unlocked - and if so, how long until it locks itself again.

---------------
Current status:
---------------

At the moment, the program is limited to a branch, as it's still in the preliminary phase of development. I'm working through a guide to ESP8266 programming, and once I have a good grasp of everything I'll pull it into the master and start to add things. The current program has the ESP hosting a simple web server, using the ESP's extra 4MB of flash to store the html files. The webpage allows file uploads and deletion, and it also allows the user to turn an LED on and off.

Current main issue is that computers aren't always able to find the ESP in the network. Will probably be resolved once I know what I'm doing.
Next stage after resolving network issues will be to figure out how to write code that verifies whether the person attempting to access a certain file actually has permission, and if not, to refuse access.

I've been learning how Android apps work, but it'll be some time before I have a good idea how I'll get the ESP to communicate with an app. I haven't uploaded any code for the app yet because I don't have much at the moment.
