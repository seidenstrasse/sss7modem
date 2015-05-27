SSS7 Modems
===========
SSS7 is the **S**eidenstrasse **S**ignaling **S**ystem 7.

**This is still work in progress, contact me before building hardware !**

This repository contains the schematics its the hardware,
along with a description of the physical layer and the transport layer.
The actual application layer will be developed independent of this specification.


Physical Layer
--------------
The physical layer is based on a RS485-like two wire bus 
(normal RS485 has different levels when idle),
using the *MCP2551* is used as transceiver chip.
Since this chip is actually a CAN-transceiver,
it has a well defined behaviour in case of collisions (no short circuits possible)
and it allows to read back the bits which were actually on the bus while sending.
Additionally for most regular RS485 transceivers a collision on the bus results
in one transceiver pulling the bus while the other one pulls it high,
effectively creating a temporary short circuit.
Therefore using a CAN-transceiver is preferable for our bus configuration.

The MCP2551 requires that the levels of the differential data lines are within +-7V
of its ground potential.
This a bit of a problem, since it is not really practical to run an additional wire
along the bus to provide a common.
Additionally routers may use power supplies which do not have a floating potential
(e.g. a computer power supply).
Connecting the ground rails of two non-floating power supplies can lead to a nasty
short circuit in the worst case.
Therefore an isolated DC/DC converter is used to provide a floating supply voltage 
for the transceiver chip and the data lines are isolated using optocouplers.
Still a common ground potential for all modes is required for the transceiver chips to work.
This is solved by adding diodes in inverse direction between the floating 5v rail and both data lines
and between the chips ground and bot datalines.
If the potential of the high data line is more than 0.7v above the floating 5v rail,
on of diodes will become conducting effectively shifting the floating grounds potential up.
Likewise if the potential of the low data line is more than 0.7v below the floating ground rail,
the ground rail will be shifted down.
Hence the floating grounds rails of all modems connected to bus will be within +-0.7v relative to each other
(not considering the voltage drop over the long wires).

Additionally these diodes suppress voltages spikes on the bus,
protecting the transceiver.

Using a sss7modem any device with a decent UART (microcontroller, raspi ...) 
can communicate over long distances (1000m+).
The data is send at 9600 Baud (aka. 9,6kbit/s) with a maximum baudrate error of +-5%,
to ensure that even the slowest microcontroller can participate in the communication.
A slow bitrate also necessary to keep communication and collision detection reliable over longer
distances.


Transport Layer
---------------
A device is allowed to send data after the bus has been idle for at least *48 bit times* (aka. 5ms).
After this period the sender is allowed to send a single frame.
The frame format is simply the first byte is the length of the entire frame in bytes (`= N + 3`), 
followed by N bytes of payload.
The remaining 2 bytes of the are a 16bit CRC checksum over the frames payload.
If a received frame has timed out or if its CRC sum does not match its payload,
any receiving devices should assume a collision and drop the frame.
A frame can be assumed as timed out if it was not fully received yet and no
additional data has been received for at least *24 bit times* (aka. 2.5ms).
Furthermore each frame is associated to a priority between 0 (highest) and 5 (lowest),
which is not part of the frame send over the bus.
Instead the priority is used to calculate the necessary back off in case of a collision.

The sender has to detect collisions by reading back each byte written to the bus immediately after writing it.
If the read byte does not match the written byte, a collision occurred.
In this case the sender has to wait for random back off interval until attempting a retransmit.
The back off interval is calculated as `(8 * bit_time) * (priority + rand(1,5))`,
where `bit_time = 1 / 9600 s` and priority is the frames priority.
Device receiving on the bus are not necessarily able to detect collisions,
unless they show up as timing error at their UART.
Therefore receiving devices should rely only on the length and the CRC sum of frame to detect collisions.


Licenses
--------

Anything in the folder hardware/libs/smisitoto_eu/ is taken from http://smisioto.no-ip.org/elettronica/kicad/kicad-en.htm
The files are made avaible under the Creative Commons license rev2.5 Attribution-ShareAlike. (see https://creativecommons.org/licenses/by-sa/2.5/)

The Chaos InKL logo is licensed CC BY-NC-SA 3.0.

The seidenstrasse logo and my personal logo are not avaiable under an open licence.
Therefore you should ask for permission before using the seidenstrasse logo in derived works.
Also please remove my personal logo if I am not involved in your project.

The schematics and pcb layout files in hardware are released under the *CERN Open Hardware Licence v1.2*.

