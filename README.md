# ESP32 ESP-IDF component for syslog (UDP client)

## Features

1. Support for all standard syslog facility codes (0-23) per RFC 5424.
2. Support for all standard syslog severity levels (0-7) per RFC 5424.
3. RFC 5424 format message generation with priority calculation.
4. Thread-safe initialization and deinitialization.
5. Memory allocation with MALLOC_CAP_8BIT for DMA compatibility.
6. Short messages use stack buffer, longer ones fall back to heap allocation.

## Attention

1. Network connection must be established before calling zh_syslog_init.
2. UDP is connectionless and message delivery is not guaranteed.
3. zh_syslog_send is not thread-safe, caller must ensure mutual exclusion.

## Using

In an existing project, run the following command to install the component:

```bash
cd ../your_project/components
git clone https://github.com/aZholtikov/zh_syslog
```

In the application, add the component:

```c
#include "zh_syslog.h"
```

## Examples

### Basic Example

```c
#include "zh_syslog.h"

void app_main(void)
{
    esp_log_level_set("zh_syslog", ESP_LOG_ERROR);
    zh_syslog_init_config_t config = ZH_SYSLOG_INIT_CONFIG_DEFAULT();
    strcpy(config.syslog_ip, "192.168.1.100");
    esp_err_t ret = zh_syslog_init(&config);
    if (ret != ESP_OK)
    {
        printf("Syslog initialization error\n");
        return;
    }
    ret = zh_syslog_send(ZH_USER, ZH_INFO, "my_device", "my_app", "Hello Syslog");
    if (ret == ESP_OK)
    {
        printf("Message sent\n");
    }
    else
    {
        printf("Send error: %s\n", esp_err_to_name(ret));
    }
    zh_syslog_deinit();
}
```

---

### Facility and Severity Example

```c
#include "zh_syslog.h"

void app_main(void)
{
    esp_log_level_set("zh_syslog", ESP_LOG_ERROR);
    zh_syslog_init_config_t config = ZH_SYSLOG_INIT_CONFIG_DEFAULT();
    strcpy(config.syslog_ip, "192.168.1.100");
    esp_err_t ret = zh_syslog_init(&config);
    if (ret != ESP_OK)
    {
        printf("Syslog initialization error\n");
        return;
    }
    ret = zh_syslog_send(ZH_KERN, ZH_ERR, "my_device", "my_app", "Kernel error");
    if (ret == ESP_OK)
    {
        printf("Kernel error sent\n");
    }
    else
    {
        printf("Send error: %s\n", esp_err_to_name(ret));
    }
    ret = zh_syslog_send(ZH_USER, ZH_WARNING, "my_device", "my_app", "User warning");
    if (ret == ESP_OK)
    {
        printf("User warning sent\n");
    }
    else
    {
        printf("Send error: %s\n", esp_err_to_name(ret));
    }
    zh_syslog_deinit();
}
```

---

### Long Message Example

```c
#include "zh_syslog.h"

void app_main(void)
{
    esp_log_level_set("zh_syslog", ESP_LOG_ERROR);
    zh_syslog_init_config_t config = ZH_SYSLOG_INIT_CONFIG_DEFAULT();
    strcpy(config.syslog_ip, "192.168.1.100");
    esp_err_t ret = zh_syslog_init(&config);
    if (ret != ESP_OK)
    {
        printf("Syslog initialization error\n");
        return;
    }
    char long_message[512];
    memset(long_message, 'A', sizeof(long_message) - 1);
    long_message[sizeof(long_message) - 1] = '\0';
    ret = zh_syslog_send(ZH_USER, ZH_INFO, "my_device", "my_app", long_message);
    if (ret == ESP_OK)
    {
        printf("Long message sent\n");
    }
    else
    {
        printf("Send error: %s\n", esp_err_to_name(ret));
    }
    zh_syslog_deinit();
}
```

---

### Deinitialization Example

```c
#include "zh_syslog.h"

void app_main(void)
{
    esp_log_level_set("zh_syslog", ESP_LOG_ERROR);
    zh_syslog_init_config_t config = ZH_SYSLOG_INIT_CONFIG_DEFAULT();
    strcpy(config.syslog_ip, "192.168.1.100");
    esp_err_t ret = zh_syslog_init(&config);
    if (ret != ESP_OK)
    {
        printf("Syslog initialization error\n");
        return;
    }
    ret = zh_syslog_send(ZH_USER, ZH_DEBUG, "my_device", "my_app", "Debug message");
    if (ret == ESP_OK)
    {
        printf("Message sent\n");
    }
    else
    {
        printf("Send error: %s\n", esp_err_to_name(ret));
    }
    ret = zh_syslog_deinit();
    if (ret == ESP_OK)
    {
        printf("Syslog deinitialized\n");
    }
    else
    {
        printf("Deinit error: %s\n", esp_err_to_name(ret));
    }
}
```
