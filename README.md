# ESP32 ESP-IDF and ESP8266 RTOS SDK component for Syslog server UDP client

## Tested on

1. ESP8266 RTOS_SDK v3.4
2. ESP32 ESP-IDF v5.2

## Using

In an existing project, run the following command to install the component:

```text
cd ../your_project/components
git clone https://github.com/aZholtikov/zh_syslog.git
```

In the application, add the component:

```c
#include "zh_syslog.h"
```

## Example

Sending messages:

```c
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_timer.h"
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "zh_syslog.h"

#define WIFI_SSID "ssid"
#define WIFI_PASS "password"
#define WIFI_MAXIMUM_RETRY 5
#define WIFI_RECONNECT_TIME 5
#define WIFI_CONNECT_SUCCESS BIT0

#define SYSLOG_IP "192.168.1.2"

esp_timer_handle_t wifi_reconnect_timer = {0};
uint8_t wifi_reconnect_retry_num = 0;
EventGroupHandle_t event_group_handle = {0};

void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

void app_main(void)
{
    esp_log_level_set("zh_syslog", ESP_LOG_NONE);
    nvs_flash_init();
    esp_netif_init();
    esp_event_loop_create_default();
#ifdef CONFIG_IDF_TARGET_ESP8266
    tcpip_adapter_init();
#else
    esp_netif_create_default_wifi_sta();
#endif
    wifi_init_config_t wifi_init_sta_config = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_init_sta_config);
    wifi_config_t wifi_config = {
        .sta.ssid = WIFI_SSID,
        .sta.password = WIFI_PASS};
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
#ifdef CONFIG_IDF_TARGET_ESP8266
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL);
#else
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL);
#endif
    esp_wifi_start();
    event_group_handle = xEventGroupCreate();
    xEventGroupWaitBits(event_group_handle, WIFI_CONNECT_SUCCESS, pdTRUE, pdTRUE, portMAX_DELAY);
    zh_syslog_init_config_t syslog_init_config = ZH_SYSLOG_INIT_CONFIG_DEFAULT();
    memcpy(syslog_init_config.syslog_ip, SYSLOG_IP, strlen(SYSLOG_IP));
    zh_syslog_init(&syslog_init_config);
    for (;;)
    {
        zh_syslog_send(ZH_USER, ZH_INFO, "my_device", "my_application", "Message");
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
        if (wifi_reconnect_retry_num < WIFI_MAXIMUM_RETRY)
        {
            esp_wifi_connect();
            ++wifi_reconnect_retry_num;
        }
        else
        {
            wifi_reconnect_retry_num = 0;
            esp_timer_create_args_t wifi_reconnect_timer_args = {
                .callback = (void *)esp_wifi_connect};
            esp_timer_create(&wifi_reconnect_timer_args, &wifi_reconnect_timer);
            esp_timer_start_once(wifi_reconnect_timer, WIFI_RECONNECT_TIME * 1000);
        }
        break;
    case IP_EVENT_STA_GOT_IP:
        wifi_reconnect_retry_num = 0;
        xEventGroupSetBits(event_group_handle, WIFI_CONNECT_SUCCESS);
        break;
    default:
        break;
    }
}
```

Any [feedback](mailto:github@azholtikov.ru) will be gladly accepted.
