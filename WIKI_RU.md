# zh_syslog — Компонент syslog UDP клиента для ESP-IDF

## Содержание

- [Обзор](#обзор)
- [Возможности](#возможности)
- [Установка](#установка)
- [Справочник API](#справочник-api)
- [Примеры использования](#примеры-использования)
- [Технические характеристики](#технические-характеристики)
- [Коды ошибок](#коды-ошибок)
- [Вклад в проект](#вклад-в-проект)
- [Лицензия](#лицензия)

---

## Обзор

`zh_syslog` — это легковесная библиотека syslog UDP клиента для ESP-IDF (Espressif IoT Development Framework). Она предоставляет реализацию формирования и передачи syslog-сообщений совместимых со стандартом RFC 5424 через UDP дейтаграммы. Библиотека разработана специально для платформ ESP32/ESP-IDF и бесшовно интегрируется с приложениями на базе FreeRTOS.

Библиотека позволяет устройствам ESP32 отправлять структурированные журнальные сообщения на удалённый syslog-сервер, что способствует централизованному управлению журналами и мониторингу в IoT-развертываниях. Сообщения форматируются в соответствии со стандартом RFC 5424, поддерживая коды facility (0-23) и уровни важности (0-7) для точной категоризации журналов.

---

## Возможности

- **Соответствие RFC 5424**: Сообщения форматируются в соответствии со стандартом syslog RFC 5424
- **Стандартные коды Facility**: Поддержка всех 24 стандартных кодов facility syslog (KERN, USER, DAEMON, LOCAL0-LOCAL7 и т.д.)
- **Уровни важности**: Поддержка всех 8 стандартных уровней severity syslog (EMERG, ALERT, CRIT, ERR, WARNING, NOTICE, INFO, DEBUG)
- **UDP транспорт**: Использует дейтаграммные сокеты для эффективной передачи журналов
- **Память совместимая с DMA**: Использует выделения MALLOC_CAP_8BIT для потенциального использования DMA
- **Структурированное логирование**: Включает поля hostname, app name и message в соответствии с RFC 5424

---

## Установка

Перейдите в каталог компонентов вашего проекта:

```bash
cd ../ваш_проект/components
```

Клонируйте репозиторий:

```bash
git clone https://github.com/aZholtikov/zh_syslog
```

В вашем приложении подключите заголовочный файл:

```c
#include "zh_syslog.h"
```

Компонент будет автоматически собран вместе с вашим проектом.

---

## Справочник API

### Структура zh_syslog_init_config_t

Структура конфигурации для инициализации syslog клиента.

**Поля (внутренние):**

| Поле | Тип | Описание |
| ------ | ------ | ---------- |
| `syslog_ip` | `char[16]` | IPv4-адрес syslog сервера в виде нуль-терминированной строки (например, "192.168.1.1"). Максимум 15 символов плюс нуль-терминатор. |
| `syslog_port` | `uint32_t` | UDP-порт syslog сервера (обычно 514). |

**Примечание:** IP-адрес должен быть валидной IPv4-строкой. Порт должен быть в допустимом диапазоне (стандартный порт syslog — 514).

---

### Перечисление zh_syslog_facility_code_t

Стандартные коды facility syslog в соответствии с RFC 5424 (0-23).

**Значения:**

| Значение | Перечисление | Описание |
| ---------- | -------------- | ---------- |
| 0 | `ZH_KERN` | Сообщения ядра |
| 1 | `ZH_USER` | Пользовательские сообщения |
| 2 | `ZH_MAIL` | Почтовая система |
| 3 | `ZH_DAEMON` | Системные демоны |
| 4 | `ZH_AUTH` | Сообщения безопасности/авторизации |
| 5 | `ZH_SYSLOG` | Сообщения от самого syslogd |
| 6 | `ZH_LPR` | Подсистема принтеров |
| 7 | `ZH_NEWS` | Сетевая подсистема новостей |
| 8 | `ZH_UUCP` | Подсистема UUCP |
| 9 | `ZH_CRON` | Подсистема cron |
| 10 | `ZH_AUTHPRIV` | Сообщения безопасности (приватные) |
| 11 | `ZH_FTP` | FTP демон |
| 12 | `ZH_NTP` | Подсистема NTP |
| 13 | `ZH_SECURITY` | Журнал аудита |
| 14 | `ZH_CONSOLE` | Журнал уведомлений |
| 15 | `ZH_SOLARIS_CRON` | Планировщик задач |
| 16-23 | `ZH_LOCAL0`-`ZH_LOCAL7` | Локальное/пользовательское использование |

---

### Перечисление zh_syslog_severity_code_t

Стандартные коды severity syslog в соответствии с RFC 5424 (0-7).

**Значения:**

| Значение | Перечисление | Описание |
| ---------- | -------------- | ---------- |
| 0 | `ZH_EMERG` | Система неработоспособна |
| 1 | `ZH_ALERT` | Необходимо немедленное действие |
| 2 | `ZH_CRIT` | Критические условия |
| 3 | `ZH_ERR` | Ошибки |
| 4 | `ZH_WARNING` | Предупреждения |
| 5 | `ZH_NOTICE` | Обычные, но значимые условия |
| 6 | `ZH_INFO` | Информационные сообщения |
| 7 | `ZH_DEBUG` | Сообщения отладки |

---

### zh_syslog_init()

Инициализирует syslog клиент путём создания UDP сокета и настройки адреса целевого сервера.

**Параметры:**

- `config` — Указатель на `zh_syslog_init_config_t` с адресом и портом syslog сервера. Может указывать на временную переменную. Не должен быть NULL

**Возвращает:**

- `ESP_OK` — Успех
- `ESP_ERR_INVALID_ARG` — Указатель config равен NULL
- `ESP_FAIL` — Создание сокета не удалось

**Пример:**

```c
zh_syslog_init_config_t config = ZH_SYSLOG_INIT_CONFIG_DEFAULT();
strcpy(config.syslog_ip, "192.168.1.100");
esp_err_t ret = zh_syslog_init(&config);
if (ret != ESP_OK) {
    // Обработка ошибки инициализации
}
```

**Примечание:** Подключение к сети должно быть установлено перед вызовом этой функции. Указатель config может быть временным.

---

### zh_syslog_deinit()

Деинициализирует syslog клиент и освобождает ресурсы.

**Возвращает:**

- `ESP_OK` — Успех
- `ESP_ERR_INVALID_STATE` — Клиент не был инициализирован

**Примечание:** После вызова этой функции `zh_syslog_init()` должна быть вызвана заново перед любыми последующими вызовами `zh_syslog_send()`.

---

### zh_syslog_send()

Отправляет отформатированное syslog-сообщение на настроенный сервер.

**Параметры:**

- `facility` — Код facility syslog (0-23) из `zh_syslog_facility_code_t`
- `severity` — Код severity syslog (0-7) из `zh_syslog_severity_code_t`
- `hostname` — Указатель на имя устройства (только алфавитно-цифровые символы, без пробелов). Не должен быть NULL
- `app_name` — Указатель на имя приложения (только алфавитно-цифровые символы, без пробелов). Не должен быть NULL
- `message` — Указатель на строку содержимого сообщения. Не должен быть NULL

**Возвращает:**

- `ESP_OK` — Успех
- `ESP_ERR_INVALID_ARG` — hostname, app_name или message равен NULL
- `ESP_ERR_INVALID_STATE` — Клиент не инициализирован
- `ESP_ERR_NO_MEM` — Ошибка выделения памяти (буфер 1024 байта)
- `ESP_FAIL` — Операция отправки UDP не удалась

**Пример:**

```c
zh_syslog_send(ZH_USER, ZH_INFO, "esp32_device", "my_app", "Приложение запущено");
```

**Примечание:** Эта функция не является потокобезопасной. Вызывающий должен обеспечить взаимное исключение, если несколько задач могут вызывать эту функцию одновременно. Формат сообщения соответствует RFC 5424: `<priority>1 timestamp hostname app-name procid msgid - message`.

---

## Примеры использования

### Базовый пример: Инициализация и отправка сообщений

```c
#include "zh_syslog.h"

void app_main(void)
{
    esp_log_level_set("zh_syslog", ESP_LOG_ERROR);
    // Инициализация конфигурации со значениями по умолчанию
    zh_syslog_init_config_t config = ZH_SYSLOG_INIT_CONFIG_DEFAULT();
    strcpy(config.syslog_ip, "192.168.1.100"); // IP syslog сервера

    // Инициализация syslog клиента
    esp_err_t ret = zh_syslog_init(&config);
    if (ret != ESP_OK)
    {
        printf("Ошибка инициализации syslog: %s\n", esp_err_to_name(ret));
        return;
    }

    // Отправка информационного сообщения
    zh_syslog_send(ZH_USER, ZH_INFO, "my_device", "my_app", "Приложение запущено");

    // Отправка предупреждения
    zh_syslog_send(ZH_USER, ZH_WARNING, "my_device", "my_app", "Низкий заряд батареи");

    // Отправка сообщения об ошибке
    zh_syslog_send(ZH_KERN, ZH_ERR, "my_device", "kernel", "Ошибка чтения диска");

    // Деинициализация при завершении
    zh_syslog_deinit();
}
```

---

### Пример: Интеграция с Wi-Fi и периодическая отправка журналов

```c
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "zh_syslog.h"

#define WIFI_SSID "ваш_ssid"
#define WIFI_PASS "ваш_password"
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

    // Инициализация syslog после подключения Wi-Fi
    zh_syslog_init_config_t config = ZH_SYSLOG_INIT_CONFIG_DEFAULT();
    strcpy(config.syslog_ip, "192.168.1.100");
    zh_syslog_init(&config);

    // Периодическая отправка журнальных сообщений
    for (int i = 0; i < 10; i++) {
        char msg[64];
        snprintf(msg, sizeof(msg), "Сердцебитие #%d", i + 1);
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

### Пример: Использование кодов Facility и Severity

```c
#include "zh_syslog.h"

void app_main(void)
{
    esp_log_level_set("zh_syslog", ESP_LOG_ERROR);
    zh_syslog_init_config_t config = ZH_SYSLOG_INIT_CONFIG_DEFAULT();
    strcpy(config.syslog_ip, "192.168.1.100");
    zh_syslog_init(&config);

    // Критическая ошибка ядра
    zh_syslog_send(ZH_KERN, ZH_CRIT, "esp32", "kernel", "Недостаточно памяти");

    // Предупреждение пользовательского приложения
    zh_syslog_send(ZH_USER, ZH_WARNING, "esp32", "app", "Таймаут чтения сенсора");

    // Уведомление подсистемы аутентификации
    zh_syslog_send(ZH_AUTH, ZH_NOTICE, "esp32", "auth", "Пользователь успешно вошёл");

    // Локальный facility для пользовательского логирования
    zh_syslog_send(ZH_LOCAL0, ZH_DEBUG, "esp32", "debugger", "Состояние переменной: 0x1234");

    zh_syslog_deinit();
}
```

---

## Технические характеристики

| Параметр | Значение |
| ---------- | ---------- |
| **Размер сообщения** | 1024 байта (фиксированный буфер) |
| **Транспортный протокол** | UDP (SOCK_DGRAM) |
| **Порт по умолчанию** | 514 |
| **Стандарт форматирования** | RFC 5424 |
| **Выделение памяти** | heap_caps_calloc (MALLOC_CAP_8BIT) |
| **Потокобезопасность** | Не потокобезопасна (ответственность вызывающего) |
| **Версия ESP-IDF** | >= 5.0 |
| **Платформа** | Серия ESP32 |
| **Язык** | C (C99) |

---

## Коды ошибок

| Код ошибки | Описание |
| ------------ | ---------- |
| `ESP_OK` | Операция выполнена успешно |
| `ESP_ERR_INVALID_ARG` | Неверный аргумент (NULL config, NULL hostname/app_name/message) |
| `ESP_ERR_INVALID_STATE` | Клиент не инициализирован (zh_syslog_init не вызвана) |
| `ESP_ERR_NO_MEM` | Ошибка выделения памяти (не хватает памяти для буфера 1024 байта) |
| `ESP_FAIL` | Создание сокета не удалось или операция отправки UDP не удалась |

---

## Вклад в проект

Вклад приветствуется! Чтобы внести свой вклад:

1. Сделайте форк репозитория
2. Создайте ветку функции (`git checkout -b feature/AmazingFeature`)
3. Закоммитьте ваши изменения (`git commit -m 'Add some AmazingFeature'`)
4. Отправьте в ветку (`git push origin feature/AmazingFeature`)
5. Откройте Pull Request

Пожалуйста, убедитесь, что ваш код следует существующему стилю и включает соответствующую документацию.

---

## Лицензия

Этот проект лицензирован по лицензии Apache, версия 2.0 - см. файл [LICENSE](LICENSE) для подробной информации.

### Apache License, Version 2.0

Авторское право (c) 2026 Алексей Жолтиков

Лицензировано по лицензии Apache License, Version 2.0 (далее — "Лицензия");
вы не можете использовать этот файл, кроме случаев, предусмотренных Лицензией.
Копию Лицензии можно получить по адресу:

    http://www.apache.org/licenses/LICENSE-2.0

Если иное не требуется действующим законодательством или не согласовано в письменном виде,
программное обеспечение, распространяемое по Лицензии, распространяется на условиях "КАК ЕСТЬ",
БЕЗ КАКИХ-ЛИБО ГАРАНТИЙ, явных или подразумеваемых, включая, но не ограничиваясь, гарантии
ТОВАРНОГО СОСТОЯНИЯ, ПРИГОДНОСТИ ДЛЯ КОНКРЕТНОЙ ЦЕЛИ И НЕНАРУШЕНИЯ ПРАВ.
Смотрите Лицензию для получения конкретных прав и ограничений.

---

## Дополнительные заметки

- **Надёжность UDP**: Эта библиотека использует UDP, который не гарантирует доставку сообщений. Для надёжной доставки реализуйте приложение-уровень подтверждения.
- **Производительность**: O(n) для форматирования сообщения (n = длина сообщения). Операция отправки блокирующая.
- **Лучшие практики**:
  - Всегда проверяйте возвращаемые значения от zh_syslog_init() и zh_syslog_send()
  - Вызывайте zh_syslog_deinit() перед выходом из приложения или выгрузкой модуля
  - Используйте коды facility для категоризации источников журналов (KERN для ОС, USER для приложения, LOCAL0-7 для пользовательских)
  - Держите hostname и app_name алфавитно-цифровыми (без пробелов) в соответствии с RFC 5424
  - Настройте соответствующий уровень фильтрации журналов на syslog-сервере, чтобы избежать избыточного трафика

---

*Сгенерировано для zh_syslog v1.0.0*
