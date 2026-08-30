# ESP32 ESP-IDF component for Syslog UDP client

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

```bash
cd ../your_project/components
git clone https://github.com/aZholtikov/zh_syslog
```

In the application, add the component:

```c
#include "zh_syslog.h"
```

## Note

Messages are built using the format defined in [RFC 5424](https://datatracker.ietf.org/doc/html/rfc5424).

## Example

```c
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_timer.h"
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "zh_syslog.h"

#define WIFI_SSID "SSID"
#define WIFI_PASS "password"
#define WIFI_MAXIMUM_RETRY 5
#define WIFI_RECONNECT_TIME 5
#define WIFI_CONNECT_SUCCESS BIT0

#define SYSLOG_IP "8.8.8.8"

esp_timer_handle_t wifi_reconnect_timer = {0};
uint8_t wifi_reconnect_retry_num = 0;
EventGroupHandle_t event_group_handle = {0};

bool flag = false;

void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

void app_main(void)
{
    esp_log_level_set("zh_syslog", ESP_LOG_ERROR);
    nvs_flash_init();
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t wifi_init_sta_config = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_init_sta_config);
    wifi_config_t wifi_config = {
        .sta.ssid = WIFI_SSID,
        .sta.password = WIFI_PASS};
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL);
    esp_wifi_start();
    event_group_handle = xEventGroupCreate();
    xEventGroupWaitBits(event_group_handle, WIFI_CONNECT_SUCCESS, pdTRUE, pdTRUE, portMAX_DELAY);
    zh_syslog_init_config_t config = ZH_SYSLOG_INIT_CONFIG_DEFAULT();
    strcpy(config.syslog_ip, SYSLOG_IP);
    zh_syslog_init(&config);
    for (;;)
    {
        if (flag == true)
        {
            zh_syslog_send(ZH_USER, ZH_INFO, "my_device", "my_application", "Message");
            vTaskDelay(500 / portTICK_PERIOD_MS);
            zh_syslog_send(ZH_USER, ZH_CRIT, "my_device", "my_application", "Message");
            vTaskDelay(500 / portTICK_PERIOD_MS);
            zh_syslog_send(ZH_KERN, ZH_ERR, "my_device", "my_application", "Message");
            vTaskDelay(500 / portTICK_PERIOD_MS);
            zh_syslog_send(ZH_KERN, ZH_WARNING, "my_device", "my_application", "Message");
        }
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id)
    {
    case WIFI_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case WIFI_EVENT_STA_DISCONNECTED:
        if (flag == true)
        {
            flag = false;
        }
        if (wifi_reconnect_retry_num < WIFI_MAXIMUM_RETRY)
        {
            esp_wifi_connect();
            ++wifi_reconnect_retry_num;
        }
        else
        {
            wifi_reconnect_retry_num = 0;
            if (wifi_reconnect_timer != NULL)
            {
                esp_timer_stop(wifi_reconnect_timer);
                esp_timer_delete(wifi_reconnect_timer);
            }
            esp_timer_create_args_t wifi_reconnect_timer_args = {
                .callback = (void *)esp_wifi_connect,
                .name = "wifi_reconnect"};
            esp_timer_create(&wifi_reconnect_timer_args, &wifi_reconnect_timer);
            esp_timer_start_once(wifi_reconnect_timer, WIFI_RECONNECT_TIME * 1000);
        }
        break;
    case IP_EVENT_STA_GOT_IP:
        flag = true;
        wifi_reconnect_retry_num = 0;
        if (wifi_reconnect_timer != NULL)
        {
            esp_timer_stop(wifi_reconnect_timer);
            esp_timer_delete(wifi_reconnect_timer);
            wifi_reconnect_timer = NULL;
        }
        xEventGroupSetBits(event_group_handle, WIFI_CONNECT_SUCCESS);
        break;
    default:
        break;
    }
}
```
