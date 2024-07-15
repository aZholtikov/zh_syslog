#pragma once

#include "stdbool.h"
#include "esp_err.h"
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"

#define ZH_SYSLOG_INIT_CONFIG_DEFAULT() \
    {                                   \
        .syslog_ip = "192.168.0.1",     \
        .syslog_port = 514              \
    }

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum // Enumeration of possible syslog facility codes.
    {
        ZH_KERN,         // Kernel messages.
        ZH_USER,         // User-level messages.
        ZH_MAIL,         // Mail system.
        ZH_DAEMON,       // System daemons.
        ZH_AUTH,         // Security/authorization messages.
        ZH_SYSLOG,       // Messages generated internally by syslogd.
        ZH_LPR,          // Line printer subsystem.
        ZH_NEWS,         // Network news subsystem.
        ZH_UUCP,         // UUCP subsystem.
        ZH_CRON,         // Cron subsystem.
        ZH_AUTHPRIV,     // Security/authorization messages.
        ZH_FTP,          // FTP daemon.
        ZH_NTP,          // NTP subsystem.
        ZH_SECURITY,     // Log audit.
        ZH_CONSOLE,      // Log alert.
        ZH_SOLARIS_CRON, // Scheduling daemon.
        ZH_LOCAL0,       // Local use 0.
        ZH_LOCAL1,       // Local use 1.
        ZH_LOCAL2,       // Local use 2.
        ZH_LOCAL3,       // Local use 3.
        ZH_LOCAL4,       // Local use 4.
        ZH_LOCAL5,       // Local use 5.
        ZH_LOCAL6,       // Local use 6.
        ZH_LOCAL7,       // Local use 7.
    } zh_syslog_facility_code_t;

    typedef enum // Enumeration of possible syslog severity codes.
    {
        ZH_EMERG,   // System is unusable.
        ZH_ALERT,   // Action must be taken immediately.
        ZH_CRIT,    // Critical conditions.
        ZH_ERR,     // Error conditions.
        ZH_WARNING, // Warning conditions.
        ZH_NOTICE,  // Normal but significant condition.
        ZH_INFO,    // Informational messages.
        ZH_DEBUG    // Debug-level messages.
    } zh_syslog_severity_code_t;

    typedef struct // Structure for initial initialization of syslog client.
    {
        char syslog_ip[16];   // Syslog server IP address.
        uint32_t syslog_port; // Syslog server port.
    } zh_syslog_init_config_t;

    /**
     * @brief Initialize syslog client.
     *
     * @param[in] config Pointer to syslog client initialized configuration structure. Can point to a temporary variable.
     *
     * @attention Connection to the network must be established first.
     *
     * @note Before initialize the syslog client recommend initialize zh_syslog_init_config_t structure with default values.
     *
     * @code zh_syslog_init_config_t config = ZH_SYSLOG_INIT_CONFIG_DEFAULT() @endcode
     *
     * @return
     *              - ESP_OK if initialization was success
     *              - ESP_ERR_INVALID_ARG if parameter error
     *              - ESP_FAIL if socker error
     */
    esp_err_t zh_syslog_init(const zh_syslog_init_config_t *config);

    /**
     * @brief Deinitialize syslog client.
     *
     * @return
     *              - ESP_OK if deinitialization was success
     *              - ESP_ERR_NOT_FOUND if syslog client is not initialized
     */
    esp_err_t zh_syslog_deinit(void);

    /**
     * @brief Send message to syslog server.
     *
     * @param[in] facility Syslog facility code.
     * @param[in] severity Syslog severity code.
     * @param[in] hostname Pointer to device that generated the message. Only alphanumeric characters without spaces.
     * @param[in] app_name Pointer to application that generated the message. Only alphanumeric characters without spaces.
     * @param[in] message Pointer to message for send.
     *
     * @return
     *              - ESP_OK if send was success
     *              - ESP_ERR_INVALID_ARG if parameter error
     *              - ESP_ERR_NOT_FOUND if syslog client is not initialized
     *              - ESP_FAIL if socker error
     */
    esp_err_t zh_syslog_send(const zh_syslog_facility_code_t facility, const zh_syslog_severity_code_t severity, const char *hostname, const char *app_name, const char *message);

#ifdef __cplusplus
}
#endif