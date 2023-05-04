# Project build

1. cd core
2. make leds-dices
3. ./flash

# Directories

* core - main application file and its header.
* common - core files for device elements or used protocols (with their headers)

# Application Testing

## device usage as USB-midi:

To list midi devices, which should include this demo device
```
    $ amidi -l
    Dir Device    Name
    IO  hw:2,0,0  MIDI demo MIDI 1
    $
```
To record events, while pushing the user button
```
    $ amidi -d -p hw:2,0,0

    90 3C 40   -- key down
    80 3C 40   -- key up
    90 3C 40
    80 3C 40^C
    12 bytes read
    $s
```
