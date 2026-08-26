# iS300-Gauge-Cluster-Controller
ESP32-bassed custom controller to run a 2001-2005 iS300 gauge cluster

### To-Dos
- Fix the power input mixup
- Move headlight level pin away from GPIO46 (input-only)
- Add Forza support
- Print iP address using tach and speed
- Add Toyota MPX support
- Add USB support
- Add a web interface

## Pin-Out Documents
- [Wiring Diagrams](https://www.2jzgarage.com/2013/04/is300-wiring-diagrams/)
- [Reverse-Engineered Pinouts & Behavior]()
- [2001 Pinouts]()
- [2002-2005 Pinouts]()


## PCB Schematic & Layout
![Schematic]()
![Layout]()

**Given the current state of the PCB, do NOT connect both the 12v barrel jack and the USB-C port at once. Only use one or the other at any given time or disconnect D2 near the USB-C connector to isolate the power from the USB-C port**

## Program
Written in the Arduino IDE, this is a (very) rough version 1 with full functionality of the indicator lights as well as the tachometer and the speedometer. It connects over WiFi and support BeamNG Drive using the telemetry settings. Feel free to modify the code however you want and *please* upload it on GitHub to contribute to this repository.

## Notes
Feel free to contact me if you need any help getting anything working or if you want to help out with this project! I would greatly appreciate it!
[haniel@spotech.dev](mailto:haniel@spotech.dev)

### The Story
For my Rx8 project I went to the junkyard to pick up some parts when a 2001 Lexus iS300 caught my attention. Taking a look at the interior revealed a beautiful gauge cluster that I knew I had to take home and reverse engineer. I've always wanted to add a gauge cluster to my sim rig, anyway. After some search it became obvious that no one has reverse engineered the iS300 gauge cluster online which led me down a rather fascinating rabbit hole of service manuals and GitHub repositories to see what my options were. Ultimately, I opted to reverse engineer the iS300 gauge cluster with the service manual, a multi meter, and a power supply. I successfully figured out what every pin does on the cluster and documented the whole process for future reference. Afterwards, I created a custom ESP32-bassed PCB to cleanly control the cluster and got it manufactured with JLC's PCB services. I also sourced all the parts I would need as well as solder paste and a hot plate. After everything arrived, I got to re flowing the PCB which was new territory for me as I have only purchased pre-assembled PCBs with AER (what a luxury it is!). It took a few attempts but ultimately I was able to solder everything on except the USB-C connector which kept cutting out. As a workaround, I soldered a female plug directly to the pins on the board which simplified the soldering and strengthened the connection. This wasn't a huge problem for me since I would only use the port to reprogram the board. After a few short hours I got the gauge cluster lights to flash on command using the Arduino IDE and after a few more I was able to also control the tachometer and speedometer by applying a frequency to those pins corresponding to the speed. A few days later, I got wifi working on the board as well as BeamNG integration. For now, the cluster is very playable but I would love to add a web interface for configuring everything, Forza integration, as well as a smoothing algorithm for the gauges. The one feature I did not implement yet is Toyota MPX, a proprietary interface used to control the built in screen as well as the fuel, coolant, and MPG gauges. For now, I've documented everything on my YouTube channel and GitHub repository.
