# openwheel
Set of tools for handling ASUS Dial and simmilar designware hardware under Linux

Developed and tested on a ASUS ProArt Studiobook H7604JV

Splitted into two parts:
- `openwheel-daemon` - a daemon for handling the hardware
- `openwheel-gadget` - a GUI for the tools

## openwheel-daemon
The daemon is a C daemon which interacts between the Dial device and dbus for signaling the desktop environment
