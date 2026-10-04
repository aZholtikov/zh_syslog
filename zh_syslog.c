#include "zh_syslog.h"

static const char *TAG = "zh_syslog";

#define ZH_LOGI(msg, ...) ESP_LOGI(TAG, msg, ##__VA_ARGS__)
#define ZH_LOGE(msg, err, ...) ESP_LOGE(TAG, "[%s:%d:%s] " msg, __FILE__, __LINE__, esp_err_to_name(err), ##__VA_ARGS__)

#define ZH_ERROR_CHECK(cond, err, cleanup, msg, ...) \
    if (!(cond))                                     \
    {                                                \
        ZH_LOGE(msg, err, ##__VA_ARGS__);            \
        cleanup;                                     \
        return err;                                  \
    }

static struct sockaddr_in _syslog_server = {0};
volatile static int _socket = -1;
volatile static bool _is_initialized = false;

esp_err_t zh_syslog_init(const zh_syslog_init_config_t *config)
{
    ESP_LOGI(TAG, "Syslog client initialization begin.");
    ZH_ERROR_CHECK(config != NULL, ESP_ERR_INVALID_ARG, NULL, "Syslog client initialization fail. Invalid argument.");
    ZH_ERROR_CHECK(_is_initialized == false, ESP_ERR_INVALID_STATE, NULL, "Syslog client initialization fail. Syslog client already initialized.");
    ZH_ERROR_CHECK(config->syslog_port != 0 && config->syslog_port <= 65535, ESP_ERR_INVALID_ARG, NULL, "Syslog client initialization fail. Invalid argument.");
    ZH_ERROR_CHECK(memchr(config->syslog_ip, '\0', sizeof(config->syslog_ip)) != NULL, ESP_ERR_INVALID_ARG, NULL, "Syslog client initialization fail. Invalid IP.");
    struct in_addr addr;
    ZH_ERROR_CHECK(inet_pton(AF_INET, config->syslog_ip, &addr) == 1, ESP_ERR_INVALID_ARG, NULL, "Syslog client initialization fail. Invalid argument.");
    _syslog_server.sin_addr = addr;
    _syslog_server.sin_family = AF_INET;
    _syslog_server.sin_port = htons((uint16_t)config->syslog_port);
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
    close(_socket);
    _socket = -1;
    _is_initialized = false;
    ESP_LOGI(TAG, "Syslog client deinitialization success.");
    return ESP_OK;
}

esp_err_t zh_syslog_send(const zh_syslog_facility_code_t facility, const zh_syslog_severity_code_t severity, const char *hostname, const char *app_name, const char *message)
{
    ESP_LOGI(TAG, "Syslog client send begin.");
    ZH_ERROR_CHECK(hostname != NULL && app_name != NULL && message != NULL, ESP_ERR_INVALID_ARG, NULL, "Syslog client send fail. Invalid argument.");
    ZH_ERROR_CHECK(_is_initialized == true, ESP_ERR_INVALID_STATE, NULL, "Syslog client send fail. Syslog client not initialized.");
    ZH_ERROR_CHECK(facility >= ZH_KERN && facility <= ZH_LOCAL7 && severity >= ZH_EMERG && severity <= ZH_DEBUG, ESP_ERR_INVALID_ARG, NULL, "Syslog client send fail. Invalid argument.");
    const int pri = ((uint8_t)facility * 8) + (uint8_t)severity;
    char stack_buf[256];
    int len = snprintf(stack_buf, sizeof(stack_buf), "<%d>1 - %s %s - - - %s", pri, hostname, app_name, message);
    ZH_ERROR_CHECK(len >= 0, ESP_FAIL, NULL, "Syslog client send fail. Format error.");
    char *payload = stack_buf;
    bool heap_allocated = false;
    if (len >= sizeof(stack_buf))
    {
        payload = heap_caps_calloc(1, len + 1, MALLOC_CAP_8BIT);
        ZH_ERROR_CHECK(payload != NULL, ESP_ERR_NO_MEM, NULL, "Syslog client send fail. Memory allocation fail or no free memory in the heap.");
        int written = snprintf(payload, len + 1, "<%d>1 - %s %s - - - %s", pri, hostname, app_name, message);
        ZH_ERROR_CHECK(written == len, ESP_FAIL, heap_caps_free(payload), "Syslog client send fail. Format error.");
        heap_allocated = true;
    }
    int sent = sendto(_socket, payload, len, 0, (struct sockaddr *)&_syslog_server, sizeof(_syslog_server));
    if (heap_allocated == true)
    {
        heap_caps_free(payload);
    }
    ZH_ERROR_CHECK(sent == len, ESP_FAIL, NULL, "Syslog client send fail. Socket error.");
    ESP_LOGI(TAG, "Syslog client send success.");
    return ESP_OK;
}