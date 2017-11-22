# PolyESP
Door opening project using an ESP8266 with an Android app.

---------------
How it'll work:
---------------

An ESP8266 microcontroller will use its WiFi capabilities to communicate with an Android app. The ESP8266's (non-volatile) flash memory is used to host a simple website, as well as user data (which will be protected). The ESP8266 will be able to activate a door unlocking system consisting of a motor (type TBD) connected to a mechanism that will rotate the door's handle.

The Android app will signal the ESP8266 to open the door through CoAP, but only if the phone's owner is registered in the ESP8266's user database and has permission to unlock it. If the phone's owner has a higher permission level, they will also be able to add or modify user data and permissions through the app.

The app should also be able to display the door's current status - whether it's locked, unlocking, or unlocked - and if so, how long until it locks itself again.

---------------
Current status:
---------------

At the moment, the program is limited to a branch, as it's still in the preliminary phase of development. I'm working through a guide to ESP8266 programming, and once I have a good grasp of everything I'll pull it into the master and start to add things. The current program has the ESP8266 hosting a simple web server, using the ESP8266's extra 4MB of flash to store some HTML files. The ESP8266 hosts a webpage that allows file uploads and deletion, and it also allows the user to turn an LED on and off. In order to modify the ESP866's file system, however, you need to sign in with a username and password.

I spoke with my university's network administrator and resolved the internet connection issues. Next step will not be to figure out how to verify who is accessing the ESP8266 as I had previously planned. The next steps will be to learn how CoAP works, to see if I can implement it on the ESP8266, and to decide on the motor control circuitry. At the same time, I'll want to password-protect the LED toggle switch: before moving to an Android app I'm going to test the system just using the website, so I'll replace the LED with the motor system. Once I see it works and once I understand how IoT protocols work, I'll be able to move the controls from the website to an app.

From what I've found on the net, the main IoT protocols are MQTT and CoAP, and CoAP seems to suit my purposes better since I'm looking for more of a call-and-response type communication that has minimal power consumption and a fast response time.

My two options for motors are 1) a servo 2) a DC motor. If I use a servo, I have high torque but a limited range of motion, and I want to limit the amount of forces in the mechanism. If I use a DC motor, I can use the principle of a lever in order to minimize the torque, but then I'll need some king of feedback in order to know when to stop turning the motor.

I haven't read much more about Android apps lately, as school has taken up the majority of my time. Now that I know there are internet protocols specifically for IoT, I have a solid starting point for when I start working on the app. Since this is an IoT application, ideally I'll be able to program it to work in some sort of low-power or sleep mode. I've yet to learn how that works. I also have to learn more about internet security to make sure it's well-protected.