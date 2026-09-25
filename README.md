# i2c query tool (ls_i2c)<br>
Copyright (c) 2026 BitBank Software, Inc.<br>
Written by Larry Bank<br>
email: bitbank@pobox.com<br>
## What does it do?
The ls_i2c tool is meant as a friendlier and more functional replacement for the ancient Linux i2cdetect. The tool can scan multiple I2C buses at once and has the ability to auto-detect a growing list of popular I2C devices. The output is a neat list of detected addresses with device info (model and type) if it's one of the 50+ recognized devices. It uses the device address as a starting point and then matches unique info such as the WHO_AM_I register to determine what devices are responding.<br>

## Why did you write it?
Lately I've been doing a lot of work on my sensor libraries and have run into some frustrating problems with getting the I2C overlays to work on my collection of Linux SBCs. I have had to run i2cdetect many times and every time I did, it made me think there must be a better way to display that info. Making use of some code I wrote for my Arduino BitBang_I2C library turned out to be the right way to approach the problem. The result is a tool I find much more useful.<br>

## How does it look?
See the images below to compare the output of i2cdetect (top) versus ls_i2c (bottom) for the same machine and sensors.<br>

![i2cdetect](/i2ctool.png?raw=true "i2cdetect")

![ls_i2c](/ls_i2c.png?raw=true "ls_i2c")

## Getting started
The project is composed of a single .C file and makefile. Simply clone the repo and type 'make' to build it. When you run the tool without any parameters, instructions will be printed on the terminal.<br>

If you find this code useful, please consider becoming a sponsor or sending a donation.

[![paypal](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=SR4F44J2UR8S4)


