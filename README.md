# ESP32 ESP-IDF component for Syslog UDP client

## Wiki

[EN](WIKI_EN.md) | [RU](WIKI_RU.md)

## Tested on

1. [ESP-IDF v6.0.0](https://docs.espressif.com/projects/esp-idf/en/v6.0/esp32/index.html)

## SAST Tools

[PVS-Studio](https://pvs-studio.com/pvs-studio/?utm_source=website&utm_medium=github&utm_campaign=open_source) — static analyzer for C, C++, C#, and Java code.

## Features

1. Support for all standard syslog facility codes (0-23) per RFC 5424.
2. Support for all standard syslog severity levels (0-7) per RFC 5424.
3. RFC 5424 format message generation with priority calculation.
4. Thread-safe initialization and deinitialization.
5. MALLOC_CAP_8BIT memory allocation for DMA compatibility.

## Using

In an existing project, run the following command to install the components:

```text
cd ../your_project/components
git clone https://github.com/aZholtikov/zh_syslog.git
```

In the application, add the component:

```c
#include "zh_syslog.h"
```

## Examples

See Wiki [EN](WIKI_EN.md#usage-examples) | [RU](WIKI_RU.md#примеры-использования)

## Note

Messages are built using the format defined in [RFC 5424](https://datatracker.ietf.org/doc/html/rfc5424).
