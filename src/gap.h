
#include "esp_gap_bt_api.h"
#include "esp_bt_defs.h"
#include "./stack/bt_types.h"
#ifdef __cplusplus
extern "C"
{
#endif

//extern uint8_t gamepad_btaddr[6];
typedef struct esp_hidh_scan_result_s {
    struct esp_hidh_scan_result_s *next;

    esp_bd_addr_t bda;
    const char *name;
    int8_t rssi;
    union {
        struct {
            esp_bt_cod_t cod;
            esp_bt_uuid_t uuid;
        } bt;
        struct {
            esp_ble_addr_type_t addr_type;
            uint16_t appearance;
        } ble;
    };
} esp_hid_scan_result_t;
 bool start();
 esp_err_t esp_hid_scan(uint32_t seconds, size_t *num_results, esp_hid_scan_result_t **results);
 void esp_hid_scan_results_free(esp_hid_scan_result_t *results);
void esp_hidh_dev_open(uint8_t *addr);
#ifdef __cplusplus
}
#endif