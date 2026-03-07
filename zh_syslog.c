#include "zh_syslog.h"

#define ZH_LOGI(msg, ...) ESP_LOGI(TAG, msg, ##__VA_ARGS__)
#define ZH_LOGE(msg, err, ...) ESP_LOGE(TAG, "[%s:%d:%s] " msg, __FILE__, __LINE__, esp_err_to_name(err), ##__VA_ARGS__)

#define ZH_ERROR_CHECK(cond, err, cleanup, msg, ...) \
    if (!(cond))                                     \
    {                                                \
        ZH_LOGE(msg, err, ##__VA_ARGS__);            \
        cleanup;                                     \
        return err;                                  \
    }

static const char *TAG = "zh_syslog";

static zh_syslog_init_config_t _init_config = {0};
static struct sockaddr_in _syslog_server = {0};
static int _socket = 0;
static bool _is_initialized = false;

esp_err_t zh_syslog_init(const zh_syslog_init_config_t *config)
{
    ESP_LOGI(TAG, "Syslog client initialization begin.");
    ZH_ERROR_CHECK(config != NULL, ESP_ERR_INVALID_ARG, NULL, "Syslog client initialization fail. Invalid argument.");
    _init_config = *config;
    _syslog_server.sin_addr.s_addr = inet_addr(_init_config.syslog_ip);
    _syslog_server.sin_family = AF_INET;
    _syslog_server.sin_port = htons(_init_config.syslog_port);
    _socket = socket(AF_INET, SOCK_DGRAM, 0);
    ZH_ERROR_CHECK(_socket >= 0, ESP_FAIL, NULL, "Syslog client initialization fail. Create socket error.");
    _is_initialized = true;
    ESP_LOGI(TAG, "Syslog client initialization success.");
    return ESP_OK;
}

esp_err_t zh_syslog_deinit(void)
{
    ESP_LOGI(TAG, "Syslog client deinitialization begin.");
    ZH_ERROR_CHECK(_is_initialized == true, ESP_ERR_INVALID_STATE, NULL, "Syslog client deinitialization fail. Syslog client not initialized.");
    shutdown(_socket, 0);
    close(_socket);
    _is_initialized = false;
    ESP_LOGI(TAG, "Syslog client deinitialization success.");
    return ESP_OK;
}

esp_err_t zh_syslog_send(const zh_syslog_facility_code_t facility, const zh_syslog_severity_code_t severity, const char *hostname, const char *app_name, const char *message) // -V2008
{
    ESP_LOGI(TAG, "Syslog client send begin.");
    ZH_ERROR_CHECK(hostname != NULL && app_name != NULL && message != NULL, ESP_ERR_INVALID_ARG, NULL, "Syslog client send fail. Invalid argument.");
    ZH_ERROR_CHECK(_is_initialized == true, ESP_ERR_INVALID_STATE, NULL, "Syslog client send fail. Syslog client not initialized.");
    char *payload = heap_caps_calloc(1, 1024, MALLOC_CAP_8BIT);
    ZH_ERROR_CHECK(payload != NULL, ESP_ERR_NO_MEM, NULL, "Syslog client send fail. Memory allocation fail or no free memory in the heap.");
    sprintf(payload, "<%d>1 - %s %s - - - %s", ((uint8_t)facility * 8) + (uint8_t)severity, hostname, app_name, message);
    int temp = sendto(_socket, payload, strlen(payload), 0, (struct sockaddr *)&_syslog_server, sizeof(_syslog_server)); // -V641
    ZH_ERROR_CHECK(temp >= 0, ESP_FAIL, heap_caps_free(payload), "Syslog client send fail. Socker error.");
    heap_caps_free(payload);
    ESP_LOGI(TAG, "Syslog client send success.");
    return ESP_OK;
}