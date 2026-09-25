# BitBank i2c query tool (ls_i2c)<br>
Copyright (c) 2026 BitBank Software, Inc.<br>
Written by Larry Bank<br>
email: bitbank@pobox.com<br>
## What does it do?
The ls_i2c tool is meant to replace the ancient Linux i2cdetect with more and friendlier features. The tool can scan multiple I2C buses at once and has the ability to auto-detect a growing list of popular I2C devices. The output is neat list of detected devices with optional device model and type info (if recognized). It uses the device address and info such as the WHO_AM_I register to determine what devices are responding.<br>

## Why did you write it?
Lately I've been doing a lot of work on my sensor libraries and have run into some frustrating problems with getting the I2C overlays to work properly on random Linux SBCs. I have had to run i2cdetect many times and every time I did, I kept thinking that there must be a better way to do it. Making use of some code I wrote for my Arduino BitBang_I2C library turned out to be the right way to approach the problem. The result is a tool that I'm much happier to use and hopefully you'll feel the same way.<br>


If you find this code useful, please consider becoming a sponsor or sending a donation.

[![paypal](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=SR4F44J2UR8S4)


