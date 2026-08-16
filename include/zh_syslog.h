/**
 * @file zh_syslog.h
 *
 * @brief ESP-IDF syslog UDP client library for sending log messages to a remote syslog server.
 *
 * Lightweight implementation of RFC 5424-compatible syslog client using UDP datagrams.
 * Designed for ESP32/ESP-IDF platforms with FreeRTOS support.
 *
 * Key features:
 * - Support for all standard syslog facility codes (0-23)
 * - Support for all standard syslog severity levels (0-7)
 * - RFC 5424 format message generation
 * - Thread-safe initialization and deinitialization
 * - MALLOC_CAP_8BIT memory allocation for DMA compatibility
 *
 * @note Connection to the network must be established before calling zh_syslog_init().
 * @warning This library does not guarantee message delivery (UDP is connectionless).
 */

#pragma once

#include "stdbool.h"
#include "esp_err.h"
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"

/**
 * @brief Default syslog client initialization configuration.
 */
#define ZH_SYSLOG_INIT_CONFIG_DEFAULT() \
    {                                   \
        .syslog_ip = "192.168.0.1",     \
        .syslog_port = 514}

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Syslog facility codes (RFC 5424).
     *
     * Defines standard facility values ranging from 0 to 23.
     * Facilities 0-22 are standardized by RFC 5424.
     * Facilities 16-23 (LOCAL0-LOCAL7) are reserved for local/custom use.
     */
    typedef enum
    {
        ZH_KERN,         /*!< Kernel messages */
        ZH_USER,         /*!< User-level messages */
        ZH_MAIL,         /*!< Mail system */
        ZH_DAEMON,       /*!< System daemons */
        ZH_AUTH,         /*!< Security/authorization messages */
        ZH_SYSLOG,       /*!< Messages generated internally by syslogd */
        ZH_LPR,          /*!< Line printer subsystem */
        ZH_NEWS,         /*!< Network news subsystem */
        ZH_UUCP,         /*!< UUCP subsystem */
        ZH_CRON,         /*!< Cron subsystem */
        ZH_AUTHPRIV,     /*!< Security/authorization messages */
        ZH_FTP,          /*!< FTP daemon */
        ZH_NTP,          /*!< NTP subsystem */
        ZH_SECURITY,     /*!< Log audit */
        ZH_CONSOLE,      /*!< Log alert */
        ZH_SOLARIS_CRON, /*!< Scheduling daemon */
        ZH_LOCAL0,       /*!< Local use 0 */
        ZH_LOCAL1,       /*!< Local use 1 */
        ZH_LOCAL2,       /*!< Local use 2 */
        ZH_LOCAL3,       /*!< Local use 3 */
        ZH_LOCAL4,       /*!< Local use 4 */
        ZH_LOCAL5,       /*!< Local use 5 */
        ZH_LOCAL6,       /*!< Local use 6 */
        ZH_LOCAL7,       /*!< Local use 7 */
    } zh_syslog_facility_code_t;

    /**
     * @brief Syslog severity codes (RFC 5424).
     *
     * Defines eight severity levels from 0 (most severe) to 7 (least severe).
     * Level 0 (EMERG) indicates a system is unusable.
     * Level 7 (DEBUG) indicates debug-level diagnostic information.
     */
    typedef enum
    {
        ZH_EMERG,   /*!< System is unusable */
        ZH_ALERT,   /*!< Action must be taken immediately */
        ZH_CRIT,    /*!< Critical conditions */
        ZH_ERR,     /*!< Error conditions */
        ZH_WARNING, /*!< Warning conditions */
        ZH_NOTICE,  /*!< Normal but significant condition */
        ZH_INFO,    /*!< Informational messages */
        ZH_DEBUG    /*!< Debug-level messages */
    } zh_syslog_severity_code_t;

    /**
     * @brief Syslog client initialization configuration structure.
     *
     * Contains network parameters required to establish connection
     * to the remote syslog server.
     *
     * @note IP address must be a valid IPv4 string (e.g., "192.168.1.1").
     *       Maximum length is 15 characters plus null terminator (16 bytes).
     * @warning Port value should be within valid range (typically 514 for syslog).
     */
    typedef struct
    {
        char syslog_ip[16];   /*!< Syslog server IPv4 address as a null-terminated string */
        uint32_t syslog_port; /*!< Syslog server UDP port number (e.g., 514) */
    } zh_syslog_init_config_t;

    /**
     * @brief Initialize syslog client.
     *
     * Creates a UDP socket and configures connection parameters
     * for the remote syslog server.
     *
     * @param[in] config Pointer to syslog client initialization configuration structure. Can point to a temporary variable
     *
     * @return ESP_OK on success
     * @return ESP_ERR_INVALID_ARG if config is NULL
     * @return ESP_FAIL if socket creation fails
     *
     * @note Connection to the network must be established before calling this function.
     * @note Before initialize the syslog client recommend initialize zh_syslog_init_config_t structure with default values.
     */
    esp_err_t zh_syslog_init(const zh_syslog_init_config_t *config);

    /**
     * @brief Deinitialize syslog client.
     *
     * Closes the UDP socket and releases resources.
     * After calling this function, zh_syslog_init() must be called again
     * before any subsequent zh_syslog_send() calls.
     *
     * @return ESP_OK on success
     * @return ESP_ERR_INVALID_STATE if zh_syslog_init() was not called previously
     */
    esp_err_t zh_syslog_deinit(void);

    /**
     * @brief Send a syslog message to the configured server.
     *
     * Formats and sends a syslog message according to RFC 5424 structure
     * via the pre-configured UDP socket.
     *
     * @param[in] facility Syslog facility code (0-23)
     * @param[in] severity Syslog severity code (0-7)
     * @param[in] hostname Pointer to device hostname that generated the message. Only alphanumeric characters without spaces
     * @param[in] app_name Pointer to application name that generated the message. Only alphanumeric characters without spaces
     * @param[in] message Pointer to the message payload to send
     *
     * @return ESP_OK on success
     * @return ESP_ERR_INVALID_ARG if hostname, app_name, or message is NULL
     * @return ESP_ERR_INVALID_STATE if zh_syslog_init() was not called previously
     * @return ESP_ERR_NO_MEM if memory allocation fails
     * @return ESP_FAIL if UDP send operation fails
     *
     * @note This function is not thread-safe. Caller must ensure mutual exclusion
     *       if multiple tasks may call this function concurrently.
     */
    esp_err_t zh_syslog_send(const zh_syslog_facility_code_t facility, const zh_syslog_severity_code_t severity, const char *hostname, const char *app_name, const char *message);

#ifdef __cplusplus
}
#endif