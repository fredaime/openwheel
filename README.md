# openwheel
Set of tools for handling ASUS Dial and simmilar designware hardware under Linux

Developed and tested on a ASUS ProArt Studiobook H7604JV

Splitted into two parts:
- `openwheel-daemon` - a daemon for handling the hardware
- `openwheel-gadget` - a GUI for the tools

## openwheel-daemon
The daemon is a simple python script that listens for the hardware events and sends them to the gadget.

