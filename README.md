![image-20211214100756741](C:\Users\JMC\AppData\Roaming\Typora\typora-user-images\image-20211214100756741.png)

# General

Like many other 3D printer users, I often find myself in a situation where I’m not sure if the spool I want to use has enough filament left to complete a print.  It is a guessing game in which I too often lose.  As a result, I created a scale that gives an accurate estimate of the amount of filament left on a given spool.  A quick internet search found several projects which were designed to do just that.  However, none of them were exactly what I was looking for.  Most of the projects were very well done.  Here are some of the scales that I especially liked:

- ​	**Filament Health Monitor** at https://replicantfx.com/filament-health-monitor/ .  This scale gives humidity, temperature, and automatic detection of the color of the filament that is loaded..
- ​	**Build a 3D Printer Filament Scale** in Nuts and Volts Magazine 2019/Issue-4.  For Nuts and Volts subscribers it can be found at https://nutsvolts.texterity.com/nutsvolts/201904/MobilePagedReplica.action?pm=2&folio=44#pg44 This scale is very well done and very well documented.  Some notable features are a database for many common filament types, latest settings are preserved through power cycles, and a nice display that uses a rotary encoder for selection.
- ​	**Filament Holder and Scales** at https://www.youmagine.com/designs/filament-holder-and-scales .  This scale has fairly simple functionality, but I liked the 3D printed spool holder it presented, and I used it in my project.
- ​	**3D Printer Filament Measuring with Arduino** at https://www.electroschematics.com/3d-printer-filament-measuring-arduino/ .  This scale is nice, but simple.

# Features

The scale presented here has the following features that I deemed useful:

- ​	Accurate measurement of weight up to 5 kg with units selection of grams (g), kilograms (kg), ounces (oz), or pounds (lb).
- ​	Accurate measure of filament length based on spool weight and filament density.  Length units are selectable as millimeters (mm), centimeters (cm), meters (m), inches (in), feet (ft), or yards (yd).
- ​	Extensive user interface using a color 1.8” TFT display and rotary encoder.
- ​	Menu driven option settings.
- ​	Displays environmental data of temperature and humidity with selectable Fahrenheit/Celsius selection.
- ​	Database of up to 15 spools with associated ID string, empty spool weight, filament color, filament type, filament density, and filament diameter.
- ​	Database of default filament densities with 12 built-in default densities, and 3 user definable densities.
- ​	WiFi compatible, with built-in web page.
- ​	Non-volatile storage of all user settings which are preserved through power cycles.

#    