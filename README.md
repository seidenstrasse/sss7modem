SSS7 Modems
===========
SSS7 is the **S**eidenstrasse **S**ignaling **S**ystem 7.

**This is still work in progress, contact me before building hardware !**

This repository contains the schematics its the hardware,
along with a description of the physical layer and the transport layer.
The actual application layer is developed independently of this project.


Physical Layer
--------------
The physical layer is based on a RS485-like two wire bus
(normal RS485 has different levels when idle),
using the *MCP2551* is used as transceiver chip.
Since this chip is actually a CAN-transceiver,
it has a well defined behaviour in case of collisions
and it allows to read back the bits which were actually on the bus while sending.
For regular RS485 transceivers a collision on the bus results
in one transceiver pulling the bus while the other one pulls it high,
effectively creating a temporary short circuit.
Therefore using a CAN-transceiver is preferable for our bus configuration.

The MCP2551 requires that the levels of the differential data lines are within +-7V
of its ground potential.
This a bit of a problem, since it is not really practical to run an additional wire
along the bus to provide a common ground.
Additionally routers may use power supplies which do not have a floating ground potential
(e.g. a computer power supply).
Connecting the ground rails of two non-floating power supplies will lead to a problems.
Therefore an isolated DC/DC converter is used to provide a floating supply voltage
for the transceiver chip and the data lines are isolated using optocouplers.
Still a common ground potential for all nodes is required for the transceiver chips to work.
This is solved by adding diodes in blocking direction between the floating 5v rail and both data lines
and between the chips ground and both data lines.
If the potential of the high data line is more than 0.7v above the floating 5v rail,
on of diodes will become conducting effectively shifting the floating grounds potential up.
Likewise if the potential of the low data line is more than 0.7v below the floating ground rail,
the ground rail will be shifted down.
Hence the floating grounds rails of all modems connected to bus will be within +-0.7v
relative to each other(not considering the voltage drop over the long wires).

Additionally these diodes suppress voltages spikes on the bus,
protecting the transceiver.

Using a sss7modem any device with a decent UART (microcontroller, raspi ...)
can communicate over long distances (1000m+).
The data is send at 9600 Baud (aka. 9,6kbit/s) with a maximum baudrate error of +-5%,
to ensure that even the slowest microcontroller can participate in the communication.
A slow bitrate also necessary to keep communication and collision detection reliable over longer
distances and allows to use slower (and cheaper) optocouplers.

Transport Layer
---------------


### Frame format:
``` 0xAA 0xFE <payload 16 bytes> <payload crc 1byte> ```

19 bytes total

**Header: 0xAA 0xFE**
- Choosen emperically to help the Uarts to synchronize
- Improves detection of two headers colliding

**CRC-Type: Maxim iButton 8-bit**
Polynomial: x^8 + x^5 + x^4 + 1 (0x8C)
Initial value: 0x0

### Communication Protocol

- **Not final, use with caution**
- Frames are sent using standard RS232 Uarts connected to sss7 modems
- Uarts should be configured to: 9600baud, 8bits, 1 stop bit, no flow-control, no parity
- Every node can send a frame at an arbitrary point in time,
    unless it is currently receiving a frame.
- Collision detection on the sender side is done by reading back each sent byte.
- Collision checking on receiver side, is done by checking the CRC and frame header.
- If a frame has been started and there are no new bytes received for 50ms,
    the frame is considered timed out and all received data can be dropped.
- Incoming messages are stored in a fifo until the application retrieves them.
- The receive fifo has a size of at least 2 messages.
- If the fifo is full new messages will override the older ones.
    It is up to the application retrieve message in time.
- Even if sending was successful, there is still a chance that the receiver could not
    receive the frame due to missing buffer space or not enough processing time to react.
    **Important messages should utilize a ack-mechanism.**
- It is up to the application to resend messages in case of a collision/missing ack.

### Planned API
- **Not final, use with caution**
- All calls are non-blocking,
    any real work should be done by ISRs or an eventloop, which is called peridically.

- **sss7_can_send()**

    Checks wether we see the bus as idle.
    Return true iff. we neither are currently receiving a frame
    nor are sending a frame of our own.

- **sss7_send_failed()**

    Checks the send error flag.
    Returns true if the last attempt at sending a frame failed.
    An attempt is considered failed if the Uart reads back a different byte than the byte send out.
    This indicates either a collision or a hardware failure.

- **sss7_send(payload)**

    Constructs a frame around the payload and prepares for sending.
    The actual sending is either done in the ISR or by the eventloop, depending on the platform.
    The frame is completely sent iff. ```sss7_can_send() && !sss7_send_failed()``` holds.

- **sss7_has_received()**

    Returns True iff. the receive fifo contains at least one message.

- **sss7_get_received(payload)**

    Retrieves the oldest message from the receive fifo.
    It is up the application to provide sufficient space in payload.
    Does nothing sss7_has_received() is false.


Licenses
--------

Anything in the folder hardware/libs/smisitoto_eu/ is taken from http://smisioto.no-ip.org/elettronica/kicad/kicad-en.htm
The files are made avaible under the Creative Commons license rev2.5 Attribution-ShareAlike. (see https://creativecommons.org/licenses/by-sa/2.5/)

The Chaos InKL logo is licensed CC BY-NC-SA 3.0.

The seidenstrasse logo and my personal logo are not avaiable under an open licence.
Therefore you should ask for permission before using the seidenstrasse logo in derived works.
Also please remove my personal logo if I am not involved in your project.

The schematics and pcb layout files in hardware are released under the *CERN Open Hardware Licence v1.2*.
