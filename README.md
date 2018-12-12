# esp8266-air-controller
This is an automated air control system that we use to keep our Christmas light snowmen properly inflated.

There are two implementations documented here, both using the same hardware (ESP8266-01 controlling 12V solenoid air relays). The software allows a web interface to read and update the pulse settings, as well as a manual pulse option to trigger a solenoid on command. The basic software simply triggers two solenoids on set intervals and is included here mostly just for reference.

For more info on how/why we built this and details on the hardware, see https://broomfieldlights.com/led-snowmen/

## Air Control
To use, update the top section of the file with your wifi network ssid and password. Also, make up a password to use as your api key. This is a very basic (and not very secure but better than nothing) means of making sure only authorized people can talk to your device. Since I exposed mine through my router with port forwarding, this gives a small bit of assurance that random people aren't interacting with my device and possibly messing with things. Every RESTful call you make to your device must include "apikey" as a URL argument with your made up password as the value.

There are three RESTful endpoints available, all using the GET verb.

### /read?apikey=my_password
This returns text showing the current settings for each solenoid. It looks something like below:
	side[left], pulse[1000], delay[600]
	side[right], pulse[1000], delay[1200]

### /settings?apikey=my_password&side=left&pulse=1000&delay=300
This stores new settings and updates the timers for when to trigger one of the solenoids and for how long.
- side: either "left" or "right" indicating which solenoid to update
- pulse: length of the pulse of air to release in milliseconds 
- delay: length of time in seconds between the start of one pulse and the start of the next pulse

### /pulse?apikey=my_password&side=left&pulse=1000
This runs an immediate manual pulse of air to the specified side for the specified duration.
See /settings for parameter details

## How to invoke
To interact with the device, send RESTful GET requests to the endpoints described above.
For example, if your device is located on your network at 192.168.1.1, then you would send a GET request to 192.168.1.1/read?apikey=my_password
With port forwarding and Dynamic DNS, you can expose this to the outside world so you can remotely manage your air system.

A RESTful GET like this can be sent simply through the address bar of a web browser, but it is a bit clunky to manage the URL parameters. Programs such as POSTMAN are great when on a computer to formulate and save RESTful requests. On a mobile device, there are a bunch of apps that offer similar functionality. I use Httper on Android and it is adequate for my needs. I may write a simple Android app instead, but it isn't really necessary.


## Basic Air Control
To use, just update the loop function with the correct sequence of pulses and delays for your system. It works, but anytime you want to make a change, you will need to upload new code to the device. The fancier version is far better.
