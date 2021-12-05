# macfanpp
Fan control for MacBook/MacMini, spiritual successor to macfanctld.

This is a software to control the Macbook, MacbookPro, and MacMini machine's SMC interface to obtain
temperature sensor reaings and use it to control the system's fan on the same interface.

## What problem does this program solve?

When running Linux on the Mac devices, the SMC is used to read the temperature sensors and control
the fan. The driver does not have the bindings between the two that allows lm_sensors to control
the fan. Therefore a separate program will be needed to manage the fans.

Additional requirement too, may arise from needing to define custom group of sensor -> fan control.
This can be because one might want to turn on the fan when the GPU is in use more, to prevent throttling.
Or to give battery or harddrive more cooling based on detected temperature in those sensor readings.

## How is a successor to `macfanctld`?

- Uses a layered, tiered sensor groups to control the fans that is essentially
  the same as `macfanctld`.
- Switched to using Modern C++ (C++20)
  - There was a lot of bugs: integer sanity, address sanity, memory management issues that is simply solved by
    using STL, instead of writing from scratch or finding more dependency libraries.
  - The entirety of the program logic: config parser, control loop, SMC interface now fits in <500 lines
    of code comfortably.
- Switched to using json for configuration file
  - Removed having to write a config file parser and allows any number of groups instead of the traditional
    fixed groups.

## How is this compared with `other_fan_control` program?

- No Unit Tests yet, but `-Werror -Wall -Weverything` as default compile flag. Yes this means compile with clang...
- Allows user to define their own custom groups of sensors and use that to drive the
  sensors in a porportional controller for the fan. This is one of the original
  reason I decided to use `macfanctld` over the other programs (despite not having
  UTs, and possible bugs, etc.)
- Configuration file that is not in obscure, hard-to-read format that is clunky.


## What's working?

- Been using it as daily driver for MacbookPro14,2 for a while doing development work 
  (many Firefox tabs, light compiles, streaming, OBS, gstreamer encode/decode, youtube, Google Meet/Chat, etc.)
- Recented started using it on my MacMini6,2 also (mainly a media/gaming machine)


## What's missing?

- Handling signals, daemonize?
- Unit tests, code clean up, break up into files instead of one huge file.
- Switch to single thread async model: epoll(), libuv, libev type of system (instead of round-robin file polling)
- Manual overrides: TBD interaction
- Not a full PID controller
  - Only has a porportional factor (see doc for more algorithm detail)
  - This means the controls can swing more, might be noisier. (I and D parts)
- No shut offs either, so the fan might be running more than on it is on MacOS (I'm trading for lower temp here)
  But it stands to reason that a hysteresis shutoff might be good to have to prolong
  the life of the fans (esp. on the older Macs).
- GUI, display, status report, notification
- DBus
- Proper logging
- A generic, global group actions; more actions and sensor process methods.


## What's a possible odd use for this program?

- Possibly running this with my Raspberry Pi 4 case (it uses a percentage system also).
  What a strange idea. Maybe. We'll see.


## DEPENDS

- `clang`, recent version that supports `std::filesystem`
- `nlohmann::json`, it makes json so easy.
