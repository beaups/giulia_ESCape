# giulia_ESCape
A fork of beaups/giulia_ESCape, but being tailored to the 2.0, non-Quadrifoglio, Giulia.

Traction Control enable/disable for the Alfa Romeo Giulia via Arduino + canbus module

Future enhancements may include the following:
- Disable AWD (unlock center diff/transfer case) to essentially make it RWD.
- Engine RPM output for aftermarket tach and/or shift light.

This is an Arduino project that enables/disables traction control in any (R)DNA mode via ~~pressing the left stalk button (LDW) for ~2 seconds.~~ center console MENU button and steering wheel cruise control buttons “RES”/toggle. This is an evolving configuration, so may change as development progresses.

This has been tested on Beaups 2018 Giulia Quadrifoglio (late 2017 built).  It is (yet) untested on 2.0 Giulias.  It is untested on Stelvio.  It is (yet) untested on vehicles with the Secure Gateway module.

You assume all risk for using this.  You are interfacing with the stability control systems on a $50,000 car with a $50 computer.  It's very possible something could go very wrong...like loss of critical safety functions.  Seriously, this code is provided for informational purposes only.  You should not use it on an actual car.

You will need a few items:

1. An Arduino Uno
2. A CAN-BUS Shield V2 https://www.amazon.com/seeed-studio-CAN-BUS-Shield-V2/dp/B076DSQFXH
3. A way to access the CAN-CH bus.  This is not the primary can bus.  There are various places to access the CAN-CH bus (I'm using a connector in the trunk), but the most convenient place to access is is via pins 12 and 13 at the OBD port.  If you've bought an OBD cable set for your Alfa, you will want to use the gray adapter cable.  Additionally you will need an OBD->DB9 cable.  I purchased this one: https://www.amazon.com/gp/product/B01ETRINYO/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1
4. Voltage regulator for a permanent installation - https://smile.amazon.com/gp/product/B07WQJ2GD6/ The canbus shield has an option for you to power the entire setup via the power provided by the OBD port.  This seems to work fine for testing, but automotive voltage is a bit higher than optimum for the Arduino.  Also, it's possible to drain your car's battery if you choose to power the arduino via OBD port.

You'll need the CAN-BUS Shield Library.  ***IMPORTANT***  There seems to be a bug in this library, or some implementation issue.  As the library ships, setting masks and filters is very hit or miss.  I've solved it by adding a small delay (100ms) after the controller switches into config mode via mcp2515_setCANCTRL_Mode(MODE_CONFIG) in both the init_Mask() and init_Filt() functions.  If you do not make these two changes, the application is going to be working on a *lot* more packets than it needs to.  It could cause the packets we want to be filtered out erroneously.  
