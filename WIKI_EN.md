# zh_syslog — Syslog UDP Client Component for ESP-IDF

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Installation](#installation)
- [API Reference](#api-reference)
- [Usage Examples](#usage-examples)
- [Technical Specifications](#technical-specifications)
- [Error Codes](#error-codes)
- [Contributing](#contributing)
- [License](#license)

---

## Overview

`zh_syslog` is a lightweight syslog UDP client library for ESP-IDF (Espressif IoT Development Framework). It provides an implementation of RFC 5424-compatible syslog message formatting and transmission over UDP datagrams. The library is designed specifically for ESP32/ESP-IDF platforms and integrates seamlessly with FreeRTOS-based applications.

The library enables ESP32 devices to send structured log messages to a remote syslog server, facilitating centralized log management and monitoring in IoT deployments. Messages are formatted according to the RFC 5424 standard, supporting facility codes (0-23) and severity levels (0-7) for precise log categorization.

---

## Features

- **RFC 5424 Compliance**: Messages are formatted according to the RFC 5424 syslog standard
- **Standard Facility Codes**: Support for all 24 standard syslog facility codes (KERN, USER, DAEMON, LOCAL0-LOCAL7, etc.)
- **Severity Levels**: Support for all 8 standard syslog severity levels (EMERG, ALERT, CRIT, ERR, WARNING, NOTICE, INFO, DEBUG)
- **UDP Transport**: Uses datagram sockets for efficient log transmission
- **DMA-Compatible Memory**: Utilizes MALLOC_CAP_8BIT allocations for potential DMA usage
- **Structured Logging**: Includes hostname, app name, and message fields per RFC 5424

---

## Installation

Navigate to your project's components directory:

```bash
cd ../your_project/components
```

Clone the repository:

```bash
git clone https://github.com/aZholtikov/zh_syslog
```

In your application, include the header:

```c
#include "zh_syslog.h"
```

The component will be automatically built with your project.

---

## API Reference

### zh_syslog_init_config_t Structure

Configuration structure for syslog client initialization.

**Fields (internal):**

| Field | Type | Description |
| ------ | ------ | ---------- |
| `syslog_ip` | `char[16]` | Syslog server IPv4 address as a null-terminated string (e.g., "192.168.1.1"). Maximum 15 characters plus null terminator. |
| `syslog_port` | `uint32_t` | Syslog server UDP port number (typically 514). |

**Note:** IP address must be a valid IPv4 string. Port should be within valid range (standard syslog port is 514).

---

### zh_syslog_facility_code_t Enum

Standard syslog facility codes per RFC 5424 (0-23).

**Values:**

| Value | Enum | Description |
| ------- | ------ | ------------- |
| 0 | `ZH_KERN` | Kernel messages |
| 1 | `ZH_USER` | User-level messages |
| 2 | `ZH_MAIL` | Mail system |
| 3 | `ZH_DAEMON` | System daemons |
| 4 | `ZH_AUTH` | Security/authorization messages |
| 5 | `ZH_SYSLOG` | Messages from syslogd internally |
| 6 | `ZH_LPR` | Line printer subsystem |
| 7 | `ZH_NEWS` | Network news subsystem |
| 8 | `ZH_UUCP` | UUCP subsystem |
| 9 | `ZH_CRON` | Cron subsystem |
| 10 | `ZH_AUTHPRIV` | Security/authorization messages (private) |
| 11 | `ZH_FTP` | FTP daemon |
| 12 | `ZH_NTP` | NTP subsystem |
| 13 | `ZH_SECURITY` | Log audit |
| 14 | `ZH_CONSOLE` | Log alert |
| 15 | `ZH_SOLARIS_CRON` | Scheduling daemon |
| 16-23 | `ZH_LOCAL0`-`ZH_LOCAL7` | Local/custom use facilities |

---

### zh_syslog_severity_code_t Enum

Standard syslog severity codes per RFC 5424 (0-7).

**Values:**

| Value | Enum | Description |
| ------- | ------ | ------------- |
| 0 | `ZH_EMERG` | System is unusable |
| 1 | `ZH_ALERT` | Action must be taken immediately |
| 2 | `ZH_CRIT` | Critical conditions |
| 3 | `ZH_ERR` | Error conditions |
| 4 | `ZH_WARNING` | Warning conditions |
| 5 | `ZH_NOTICE` | Normal but significant condition |
| 6 | `ZH_INFO` | Informational messages |
| 7 | `ZH_DEBUG` | Debug-level messages |

---

### zh_syslog_init()

Initializes the syslog client by creating a UDP socket and configuring the destination server address.

**Parameters:**

- `config` - Pointer to `zh_syslog_init_config_t` with syslog server address and port. Can point to a temporary variable.

**Returns:**

- `ESP_OK` - Success
- `ESP_ERR_INVALID_ARG` - Config pointer is NULL
- `ESP_FAIL` - Socket creation failed

**Example:**

```c
zh_syslog_init_config_t config = ZH_SYSLOG_INIT_CONFIG_DEFAULT();
strcpy(config.syslog_ip, "192.168.1.100");
esp_err_t ret = zh_syslog_init(&config);
if (ret != ESP_OK) {
    // Handle initialization error
}
```

**Note:** Network connection must be established before calling this function. The config pointer can be temporary. Must not be NULL

---

### zh_syslog_deinit()

Deinitializes the syslog client and releases resources.

**Returns:**

- `ESP_OK` - Success
- `ESP_ERR_INVALID_STATE` - Client was not initialized

**Note:** After calling this function, `zh_syslog_init()` must be called again before any subsequent `zh_syslog_send()` calls.

---

### zh_syslog_send()

Sends a formatted syslog message to the configured server.

**Parameters:**

- `facility` - Syslog facility code (0-23) from `zh_syslog_facility_code_t`
- `severity` - Syslog severity code (0-7) from `zh_syslog_severity_code_t`
- `hostname` - Pointer to device hostname (alphanumeric characters only, no spaces). Must not be NULL
- `app_name` - Pointer to application name (alphanumeric characters only, no spaces). Must not be NULL
- `message` - Pointer to the message payload string. Must not be NULL

**Returns:**

- `ESP_OK` - Success
- `ESP_ERR_INVALID_ARG` - hostname, app_name, or message is NULL
- `ESP_ERR_INVALID_STATE` - Client was not initialized
- `ESP_ERR_NO_MEM` - Memory allocation failed (1024 bytes payload buffer)
- `ESP_FAIL` - UDP send operation failed

**Example:**

```c
zh_syslog_send(ZH_USER, ZH_INFO, "esp32_device", "my_app", "System started");
```

**Note:** This function is not thread-safe. Caller must ensure mutual exclusion if multiple tasks may call this function concurrently. Message format follows RFC 5424: `<priority>1 timestamp hostname app-name procid msgid - message`.

---

## Usage Examples

### Basic Example: Initialize and Send Messages

```c
#include "zh_syslog.h"

void app_main(void)
{
    esp_log_level_set("zh_syslog", ESP_LOG_ERROR);
    // Initialize configuration with defaults
    zh_syslog_init_config_t config = ZH_SYSLOG_INIT_CONFIG_DEFAULT();
    strcpy(config.syslog_ip, "192.168.1.100");  // Set syslog server IP

    // Initialize syslog client
    esp_err_t ret = zh_syslog_init(&config);
    if (ret != ESP_OK)
    {
        printf("Syslog initialization failed: %s\n", esp_err_to_name(ret));
        return;
    }

    // Send informational message
    zh_syslog_send(ZH_USER, ZH_INFO, "my_device", "my_app", "Application started");

    // Send warning message
    zh_syslog_send(ZH_USER, ZH_WARNING, "my_device", "my_app", "Low battery");

    // Send error message
    zh_syslog_send(ZH_KERN, ZH_ERR, "my_device", "kernel", "Disk read error");

    // Deinitialize when done
    zh_syslog_deinit();
}
```

---

### Example: Wi-Fi Integration with Periodic Logging

```c
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "zh_syslog.h"

#define WIFI_SSID "your_ssid"
#define WIFI_PASS "your_password"
#define WIFI_CONNECT_SUCCESS BIT0

static EventGroupHandle_t s_event_group;
static const int WIFI_CONNECTED_BIT = BIT0;

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

void app_main(void)
{
    esp_log_level_set("zh_syslog", ESP_LOG_ERROR);
    nvs_flash_init();
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL);
    esp_wifi_start();

    s_event_group = xEventGroupCreate();
    xEventGroupWaitBits(s_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdTRUE, portMAX_DELAY);

    // Initialize syslog after Wi-Fi connection
    zh_syslog_init_config_t config = ZH_SYSLOG_INIT_CONFIG_DEFAULT();
    strcpy(config.syslog_ip, "192.168.1.100");
    zh_syslog_init(&config);

    // Send periodic log messages
    for (int i = 0; i < 10; i++) {
        char msg[64];
        snprintf(msg, sizeof(msg), "Heartbeat #%d", i + 1);
        zh_syslog_send(ZH_USER, ZH_INFO, "esp32_sensor", "logger", msg);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    zh_syslog_deinit();
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_id == IP_EVENT_STA_GOT_IP)
    {
        xEventGroupSetBits(s_event_group, WIFI_CONNECTED_BIT);
    }
}
```

---

### Example: Using Facility and Severity Codes

```c
#include "zh_syslog.h"

void app_main(void)
{
    esp_log_level_set("zh_syslog", ESP_LOG_ERROR);
    zh_syslog_init_config_t config = ZH_SYSLOG_INIT_CONFIG_DEFAULT();
    strcpy(config.syslog_ip, "192.168.1.100");
    zh_syslog_init(&config);

    // Kernel critical error
    zh_syslog_send(ZH_KERN, ZH_CRIT, "esp32", "kernel", "Out of memory");

    // User application warning
    zh_syslog_send(ZH_USER, ZH_WARNING, "esp32", "app", "Sensor reading timeout");

    // Auth subsystem notice
    zh_syslog_send(ZH_AUTH, ZH_NOTICE, "esp32", "auth", "User login successful");

    // Local facility for custom logging
    zh_syslog_send(ZH_LOCAL0, ZH_DEBUG, "esp32", "debugger", "Variable state: 0x1234");

    zh_syslog_deinit();
}
```

---

## Technical Specifications

| Parameter | Value |
| ----------- | ------- |
| **Message Size** | 1024 bytes (fixed buffer) |
| **Transport Protocol** | UDP (SOCK_DGRAM) |
| **Default Port** | 514 |
| **Format Standard** | RFC 5424 |
| **Memory Allocation** | heap_caps_calloc (MALLOC_CAP_8BIT) |
| **Thread Safety** | Not thread-safe (caller responsibility) |
| **ESP-IDF Version** | >= 5.0 |
| **Platform** | ESP32 series |
| **Language** | C (C99) |

---

## Error Codes

| Error Code | Description |
| ------------ | ------------- |
| `ESP_OK` | Operation successful |
| `ESP_ERR_INVALID_ARG` | Invalid argument (NULL config pointer, NULL hostname/app_name/message) |
| `ESP_ERR_INVALID_STATE` | Client not initialized (zh_syslog_init not called) |
| `ESP_ERR_NO_MEM` | Memory allocation failed (out of memory for 1024-byte buffer) |
| `ESP_FAIL` | Socket creation failed or UDP send operation failed |

---

## Contributing

Contributions are welcome! To contribute:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

Please ensure your code follows the existing style and includes appropriate documentation.

---

## License

This project is licensed under the Apache License, Version 2.0 - see the [LICENSE](LICENSE) file for details.

### Apache License, Version 2.0

Copyright (c) 2026 Alexey Zholtikov

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

---

## Additional Notes

- **UDP Reliability**: This library uses UDP which does not guarantee message delivery. For reliable delivery, implement application-level acknowledgment.
- **Performance**: O(n) for message formatting (n = message length). Send operation is blocking.
- **Best Practices**:
  - Always check return values from zh_syslog_init() and zh_syslog_send()
  - Call zh_syslog_deinit() before application exit or module unload
  - Use facility codes to categorize log sources (KERN for OS, USER for app, LOCAL0-7 for custom)
  - Keep hostname and app_name alphanumeric (no spaces) per RFC 5424
  - Set appropriate log level filtering on the syslog server to avoid excessive traffic

---

*Generated for zh_syslog v1.0.0*
