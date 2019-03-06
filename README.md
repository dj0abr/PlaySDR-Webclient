# PlaySDR-Webclient
a web based SDR program made for the SDRplay RSP1a

a PC runs the SDR software:
* capturing samples from the SDR receiver (SDRplay)
* creating one line of a waterfall over a range of 200kHz
* downmixing a selected QRG into baseband
* creating a line of baseband waterfall over 20 kHz
* SSB demodulation and playing to a soundcard

the waterfall data are written into the Apache HTML directory.

User Interface:
the GUI runs in a browser
* receiving the single line of the waterfalls via WebSocket
* drawing the full waterfall
* creating the GUI
* sending user command via WebSocket to above SDR program

currently I am working with the SDRplay Hardware only.
But I use 2.4Msamples, so later on this can be compatible for
i.e. RTL sticks and others.

# this is WORK in PROGRESS
the software is not useable until the paperwork is done :-)
