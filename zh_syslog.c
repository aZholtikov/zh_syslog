#include "zh_syslog.h"

static zh_syslog_init_config_t _init_config = {0};
static struct sockaddr_in _syslog_server = {0};
static int _socket = 0;
static bool _is_initialized = false;

static const char *TAG = "zh_syslog";

esp_err_t zh_syslog_init(const zh_syslog_init_config_t *config)
{
    ESP_LOGI(TAG, "Syslog client initialization begin.");
    if (config == NULL)
    {
        ESP_LOGE(TAG, "Syslog client initialization fail. Invalid argument.");
        return ESP_ERR_INVALID_ARG;
    }
    _init_config = *config;
    _syslog_server.sin_addr.s_addr = inet_addr(_init_config.syslog_ip);
    _syslog_server.sin_family = AF_INET;
    _syslog_server.sin_port = htons(_init_config.syslog_port);
    _socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (_socket < 0)
    {
        ESP_LOGE(TAG, "Syslog client initialization fail. Create socket error.");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "Syslog client initialization success.");
    _is_initialized = true;
    return ESP_OK;
}

esp_err_t zh_syslog_deinit(void)
{
    ESP_LOGI(TAG, "Syslog client deinitialization begin.");
    if (_is_initialized == false)
    {
        ESP_LOGE(TAG, "Syslog client deinitialization fail. Syslog client not initialized.");
        return ESP_ERR_NOT_FOUND;
    }
    shutdown(_socket, 0);
    close(_socket);
    ESP_LOGI(TAG, "Syslog client deinitialization success.");
    return ESP_OK;
}

esp_err_t zh_syslog_send(const zh_syslog_facility_code_t facility, const zh_syslog_severity_code_t severity, const char *hostname, const char *app_name, const char *message)
{
    ESP_LOGI(TAG, "Syslog client send begin.");
    if (hostname == NULL || app_name == NULL || message == NULL)
    {
        ESP_LOGE(TAG, "Syslog client send fail. Invalid argument.");
        return ESP_ERR_INVALID_ARG;
    }
    if (_is_initialized == false)
    {
        ESP_LOGE(TAG, "Syslog client send fail. Syslog client not initialized.");
        return ESP_ERR_NOT_FOUND;
    }
    char *payload = (char *)heap_caps_malloc(1024, MALLOC_CAP_8BIT);
    memset(payload, 0, 1024);
    sprintf(payload, "<%d>1 - %s %s - - - %s", (facility * 8) + severity, hostname, app_name, message);
    if (sendto(_socket, payload, strlen(payload), 0, (struct sockaddr *)&_syslog_server, sizeof(_syslog_server)) < 0)
    {
        ESP_LOGE(TAG, "Syslog client send fail. Socker error.");
        heap_caps_free(payload);
        return ESP_FAIL;
    }
    heap_caps_free(payload);
    ESP_LOGI(TAG, "Syslog client send success.");
    return ESP_OK;
}