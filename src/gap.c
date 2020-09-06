/*
 * gap.c
 */
#include <Arduino.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"
#include "stack/gap_api.h"
#include "stack/bt_types.h"
#include "osi/allocator.h"

#include "gap.h"
#include "parse.h"
#include "gp_int.h"
#include "gp.h"
#define GAP_DBG_PRINTF(...) //printf(__VA_ARGS__)
static xSemaphoreHandle bt_scan_cb_semaphore = NULL;
#define WAIT_BT_CB() xSemaphoreTake(bt_scan_cb_semaphore, portMAX_DELAY)
#define SEND_BT_CB() xSemaphoreGive(bt_scan_cb_semaphore)
static esp_hid_scan_result_t *bt_scan_results = NULL;
static size_t num_bt_scan_results = 0;

static bool is_connected = false;
bool targetFound = false;

uint16_t gap_handle_hidc = GAP_INVALID_HANDLE;
uint16_t gap_handle_hidi = GAP_INVALID_HANDLE;

static tL2CAP_ERTM_INFO ertm_info = {0};
static tL2CAP_CFG_INFO cfg_info = {0};

static uint16_t gap_init_service(char *, uint16_t psm, uint8_t security_id,uint8_t * addr);
static void gap_event_handle(uint16_t handle, uint16_t event);
/*******************************************************************************
**
** Function         gp_gap_update_connected
**
** Description      This updates the is_connected flag by checking if the
**                  GAP handles return valid L2CAP channel IDs.
**
** Returns          void
**
*******************************************************************************/
static void gp_gap_update_connected()
{
   // printf("gap_handle_hidc %d\n",GAP_ConnGetL2CAPCid(gap_handle_hidc) != 0);
   // printf("gap_handle_hidi %d\n",GAP_ConnGetL2CAPCid(gap_handle_hidi) != 0);
    is_connected = GAP_ConnGetL2CAPCid(gap_handle_hidc) != 0
                && GAP_ConnGetL2CAPCid(gap_handle_hidi) != 0;
}
// static bool is_target_addr(uint8_t *addr) {
// //   for (int i = 0; i < BD_ADDR_LEN; i++) {
// //     if (addr[i] != gamepad_btaddr[i]) {
// //       return false;
// //     }
// //   }

//   return true;
// }

/********************************************************************************/
/*                      P U B L I C    F U N C T I O N S                        */
/********************************************************************************/
/*******************************************************************************
**
** Function         gp_gap_is_connected
**
** Description      This returns whether a GP controller is connected, based
**                  on whether a successful handshake has taken place.
**
** Returns          void
**
*******************************************************************************/
bool gp_gap_is_connected()
{
    return is_connected;
}



void esp_hid_scan_results_free(esp_hid_scan_result_t *results)
{
    esp_hid_scan_result_t *r = NULL;
    while (results) {
        r = results;
        results = results->next;
        if (r->name != NULL) {
            free((char *)r->name);
        }
        free(r);
    }
}

static esp_hid_scan_result_t *find_scan_result(esp_bd_addr_t bda, esp_hid_scan_result_t *results)
{
    esp_hid_scan_result_t *r = results;
    while (r) {
        if (memcmp(bda, r->bda, sizeof(esp_bd_addr_t)) == 0) {
            return r;
        }
        r = r->next;
    }
    return NULL;
}

static void add_bt_scan_result(esp_bd_addr_t bda, esp_bt_cod_t *cod, esp_bt_uuid_t *uuid, uint8_t *name, uint8_t name_len, int rssi)
{
    esp_hid_scan_result_t *r = find_scan_result(bda, bt_scan_results);
    if (r) {
        //Some info may come later
        if (r->name == NULL && name && name_len) {
            char *name_s = (char *)malloc(name_len + 1);
            if (name_s == NULL) {
                ESP_LOGE(TAG, "Malloc result name failed!");
                return;
            }
            memcpy(name_s, name, name_len);
            name_s[name_len] = 0;
            r->name = (const char *)name_s;
        }
        if (r->bt.uuid.len == 0 && uuid->len) {
            memcpy(&r->bt.uuid, uuid, sizeof(esp_bt_uuid_t));
        }
        if (rssi != 0) {
            r->rssi = rssi;
        }
        return;
    }

    r = (esp_hid_scan_result_t *)malloc(sizeof(esp_hid_scan_result_t));
    if (r == NULL) {
        ESP_LOGE(TAG, "Malloc bt_hidh_scan_result_t failed!");
        return;
    }
    memcpy(r->bda, bda, sizeof(esp_bd_addr_t));
    memcpy(&r->bt.cod, cod, sizeof(esp_bt_cod_t));
    memcpy(&r->bt.uuid, uuid, sizeof(esp_bt_uuid_t));
    r->rssi = rssi;
    r->name = NULL;
    if (name_len && name) {
        char *name_s = (char *)malloc(name_len + 1);
        if (name_s == NULL) {
            free(r);
            ESP_LOGE(TAG, "Malloc result name failed!");
            return;
        }
        memcpy(name_s, name, name_len);
        name_s[name_len] = 0;
        r->name = (const char *)name_s;
    }
    r->next = bt_scan_results;
    bt_scan_results = r;
    num_bt_scan_results++;
}

void print_uuid(esp_bt_uuid_t *uuid)
{
    if (uuid->len == ESP_UUID_LEN_16) {
        GAP_DBG_PRINTF("UUID16: 0x%04x", uuid->uuid.uuid16);
    } else if (uuid->len == ESP_UUID_LEN_32) {
        GAP_DBG_PRINTF("UUID32: 0x%08x", uuid->uuid.uuid32);
    } else if (uuid->len == ESP_UUID_LEN_128) {
        GAP_DBG_PRINTF("UUID128: %02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x", uuid->uuid.uuid128[0],
                       uuid->uuid.uuid128[1], uuid->uuid.uuid128[2], uuid->uuid.uuid128[3],
                       uuid->uuid.uuid128[4], uuid->uuid.uuid128[5], uuid->uuid.uuid128[6],
                       uuid->uuid.uuid128[7], uuid->uuid.uuid128[8], uuid->uuid.uuid128[9],
                       uuid->uuid.uuid128[10], uuid->uuid.uuid128[11], uuid->uuid.uuid128[12],
                       uuid->uuid.uuid128[13], uuid->uuid.uuid128[14], uuid->uuid.uuid128[15]);
    }
}


static void handle_bt_device_result(struct disc_res_param *disc_res)
{
    GAP_DBG_PRINTF("BT : " ESP_BD_ADDR_STR, ESP_BD_ADDR_HEX(disc_res->bda));
    uint32_t codv = 0;
    esp_bt_cod_t *cod = (esp_bt_cod_t *)&codv;
    int8_t rssi = 0;
    uint8_t *name = NULL;
    uint8_t name_len = 0;
    esp_bt_uuid_t uuid;

    uuid.len = ESP_UUID_LEN_16;
    uuid.uuid.uuid16 = 0;

    for (int i = 0; i < disc_res->num_prop; i++) {
        esp_bt_gap_dev_prop_t *prop = &disc_res->prop[i];
        if (prop->type != ESP_BT_GAP_DEV_PROP_EIR) {
          //  GAP_DBG_PRINTF(", %s: ", gap_bt_prop_type_names[prop->type]);
        }
        if (prop->type == ESP_BT_GAP_DEV_PROP_BDNAME) {
            name = (uint8_t *)prop->val;
            name_len = strlen((const char *)name);
            GAP_DBG_PRINTF("%s", (const char *)name);
        } else if (prop->type == ESP_BT_GAP_DEV_PROP_RSSI) {
            rssi = *((int8_t *)prop->val);
            GAP_DBG_PRINTF("%d", rssi);
        } else if (prop->type == ESP_BT_GAP_DEV_PROP_COD) {
            memcpy(&codv, prop->val, sizeof(uint32_t));
            GAP_DBG_PRINTF("major: %s, minor: %d, service: 0x%03x", esp_hid_cod_major_str(cod->major), cod->minor, cod->service);
        } else if (prop->type == ESP_BT_GAP_DEV_PROP_EIR) {
            uint8_t len = 0;
            uint8_t *data = 0;

            data = esp_bt_gap_resolve_eir_data((uint8_t *)prop->val, ESP_BT_EIR_TYPE_CMPL_16BITS_UUID, &len);
            if (data == NULL) {
                data = esp_bt_gap_resolve_eir_data((uint8_t *)prop->val, ESP_BT_EIR_TYPE_INCMPL_16BITS_UUID, &len);
            }
            if (data && len == ESP_UUID_LEN_16) {
                uuid.len = ESP_UUID_LEN_16;
                uuid.uuid.uuid16 = data[0] + (data[1] << 8);
                GAP_DBG_PRINTF(", "); print_uuid(&uuid);
                continue;
            }

            data = esp_bt_gap_resolve_eir_data((uint8_t *)prop->val, ESP_BT_EIR_TYPE_CMPL_32BITS_UUID, &len);
            if (data == NULL) {
                data = esp_bt_gap_resolve_eir_data((uint8_t *)prop->val, ESP_BT_EIR_TYPE_INCMPL_32BITS_UUID, &len);
            }
            if (data && len == ESP_UUID_LEN_32) {
                uuid.len = len;
                memcpy(&uuid.uuid.uuid32, data, sizeof(uint32_t));
                GAP_DBG_PRINTF(", "); print_uuid(&uuid);
                continue;
            }

            data = esp_bt_gap_resolve_eir_data((uint8_t *)prop->val, ESP_BT_EIR_TYPE_CMPL_128BITS_UUID, &len);
            if (data == NULL) {
                data = esp_bt_gap_resolve_eir_data((uint8_t *)prop->val, ESP_BT_EIR_TYPE_INCMPL_128BITS_UUID, &len);
            }
            if (data && len == ESP_UUID_LEN_128) {
                uuid.len = len;
                memcpy(uuid.uuid.uuid128, (uint8_t *)data, len);
                GAP_DBG_PRINTF(", "); print_uuid(&uuid);
                continue;
            }

            //try to find a name
            if (name == NULL) {
                data = esp_bt_gap_resolve_eir_data((uint8_t *)prop->val, ESP_BT_EIR_TYPE_CMPL_LOCAL_NAME, &len);
                if (data == NULL) {
                    data = esp_bt_gap_resolve_eir_data((uint8_t *)prop->val, ESP_BT_EIR_TYPE_SHORT_LOCAL_NAME, &len);
                }
                if (data && len) {
                    name = data;
                    name_len = len;
                    GAP_DBG_PRINTF(", NAME: ");
                    for (int x = 0; x < len; x++) {
                        GAP_DBG_PRINTF("%c", (char)data[x]);
                    }
                }
            }
        }
    }
    GAP_DBG_PRINTF("\n");

    if (cod->major == ESP_BT_COD_MAJOR_DEV_PERIPHERAL || (find_scan_result(disc_res->bda, bt_scan_results) != NULL)) {
        add_bt_scan_result(disc_res->bda, cod, &uuid, name, name_len, rssi);
    }
}




static void gap_callback(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param) {
  switch (event) {
    case ESP_BT_GAP_DISC_STATE_CHANGED_EVT: {
        ESP_LOGV(TAG, "BT GAP DISC_STATE %s", (param->disc_st_chg.state == ESP_BT_GAP_DISCOVERY_STARTED) ? "START" : "STOP");
        if (param->disc_st_chg.state == ESP_BT_GAP_DISCOVERY_STOPPED) {
            SEND_BT_CB();
        }
        break;
    }

    case ESP_BT_GAP_DISC_RES_EVT: {
      //处理扫描结果
      handle_bt_device_result(&param->disc_res);
      

      break;
    }
    default:
      break;
  }
}

static void gap_event_handle(uint16_t handle, uint16_t event) {
  switch (event) {
        case GAP_EVT_CONN_OPENED:
        case GAP_EVT_CONN_CLOSED:{
            uint8_t was_connected = is_connected;
           
            gp_gap_update_connected();

            if(was_connected != is_connected){
                gp_connect_event(is_connected);
            }else
            {
                
            }
            break;
        }
    // case GAP_EVT_CONN_OPENED: {
    //   ESP_LOGI("ESP32BT", "GAP Connection Opened: %d", handle);
    //   esp_bt_gap_cancel_discovery();
    //   break;
    // }

    // case GAP_EVT_CONN_CLOSED: {
    //   targetFound = false;
    //   esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_INQUIRY, 0x30, 0x0);
    //   break;
    // }

    case GAP_EVT_CONN_DATA_AVAIL: {
      BT_HDR *p_buf;

      GAP_ConnBTRead(handle, &p_buf);

      if (p_buf->len > 0) {
          //ESP_LOGE("ESP32BT", "%s GAP Data Available", __func__);
          //parse_packet(p_buf->len, p_buf->offset, p_buf->data);
          //printf("offset is %d\n",p_buf->offset);
          gp_parse_packet(p_buf->data );
      }

      osi_free(p_buf);

      break;
    }
  }
}

bool start() {
  //esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();

  // if (esp_bt_controller_init(&bt_cfg) != ESP_OK) {
  //     ESP_LOGE("ESP32BT", "Bluetooth controller initialize failed");
  //     return false;
  // }

  // if (esp_bt_controller_enable(ESP_BT_MODE_BTDM) != ESP_OK) {
  //     ESP_LOGE("ESP32BT", "Bluetooth controller enable failed");
  //     return false;
  // }

  // if (esp_bluedroid_init() != ESP_OK) {
  //   ESP_LOGE("ESP32BT", "Bluedroid initialize failed");
  //   return false;
  // }

  // if (esp_bluedroid_enable() != ESP_OK) {
  //   ESP_LOGE("ESP32BT", "Bluedroid enable failed");
  //   return false;
  // }
    bt_scan_cb_semaphore = xSemaphoreCreateBinary();
    if (bt_scan_cb_semaphore == NULL) {
        ESP_LOGE(TAG, "xSemaphoreCreateMutex failed!");
        vSemaphoreDelete(bt_scan_cb_semaphore);
        bt_scan_cb_semaphore = NULL;
        return ESP_FAIL;
    }

  if(!btStarted() && !btStart()){
        ESP_LOGE("ESP32BT","btStart failed");
        return false;
    }

     esp_bluedroid_status_t bt_state = esp_bluedroid_get_status();
    if(bt_state == ESP_BLUEDROID_STATUS_UNINITIALIZED){
        if (esp_bluedroid_init()) {
            ESP_LOGE("ESP32BT","esp_bluedroid_init failed");
            return false;
        }
    }

    if(bt_state != ESP_BLUEDROID_STATUS_ENABLED){
        if (esp_bluedroid_enable()) {
            ESP_LOGE("ESP32BT","esp_bluedroid_enable failed");
            return false;
        }
    }
  esp_bt_gap_register_callback(gap_callback);

  // if (esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_INQUIRY, 0x30, 0x0) != ESP_OK) {
  //   ESP_LOGE("ESP32BT", "Unable to start discovery service");
  //   return false;
  // }

  return true;
}

static uint16_t gap_init_service(char *name, uint16_t psm, uint8_t security_id,uint8_t * addr ) {
    uint16_t handle = GAP_ConnOpen(name, security_id, /*is_server=*/false, /*p_rem_bda=*/addr,
                     psm, &cfg_info, &ertm_info, /*security=*/0, /*chan_mode_mask=*/0,
                     gap_event_handle);

    if (handle == GAP_INVALID_HANDLE){
        printf("%s Registering GAP service %s failed", __func__, name);
    }else{
        printf( "[%s] GAP Service %s Initialized: %d\n", __func__, name, handle);
    }
    return handle;
}

 esp_err_t start_bt_scan(uint32_t seconds)
{
    esp_err_t ret = ESP_OK;
    if ((ret = esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_INQUIRY, (int)(seconds / 1.28), 0)) != ESP_OK) {
        ESP_LOGE(TAG, "esp_bt_gap_start_discovery failed: %d", ret);
        return ret;
    }
    return ret;
}
esp_err_t esp_hid_scan(uint32_t seconds, size_t *num_results, esp_hid_scan_result_t **results)
{
    if (num_bt_scan_results || bt_scan_results) {
        ESP_LOGE(TAG, "There are old scan results. Free them first!");
        return ESP_FAIL;
    }
    if (start_bt_scan(seconds) == ESP_OK) {
            WAIT_BT_CB();
    }else {
        return ESP_FAIL;
    }
    *num_results = num_bt_scan_results;
    *results = bt_scan_results;
    if (num_bt_scan_results) {
        while (bt_scan_results->next != NULL) {
            bt_scan_results = bt_scan_results->next;
        }
    } 

    num_bt_scan_results = 0;
    bt_scan_results = NULL;
    return ESP_OK;
}
void  esp_hidh_dev_open(uint8_t *addr)
{
 //   if (is_target_addr(addr) && !targetFound) {
        targetFound = true;
       // printf("Attempting to connect\n");
        gap_handle_hidc = gap_init_service( "HIDC", BT_PSM_HIDC, BTM_SEC_SERVICE_FIRST_EMPTY,addr);
        gap_handle_hidi = gap_init_service( "HIDI", BT_PSM_HIDI, BTM_SEC_SERVICE_FIRST_EMPTY+1,addr);
//      }
}

