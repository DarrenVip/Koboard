#include "gamepad.h"

#include <esp_bt_main.h>
#include <esp_bt_defs.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define CONSTRAIN_8BIT(a) MIN(MAX(a, 0), 255)
static const char *TAG = "BT_HIDH";
extern "C" {
  #include  "gp.h"
  #include "gap.h"
}

gamepad::gamepad() {}

bool gamepad::begin() {
    size_t results_len = 0;
    esp_hid_scan_result_t *results = NULL;

    gpSetEventObjectCallback(this, &gamepad::_event_callback);
    gpSetConnectionObjectCallback(this, &gamepad::_connection_callback);
    if(start())
    {
        
    }
    
    printf("SCAN...\n");
    //start scan for HID devices
    esp_hid_scan(5, &results_len, &results);
    printf("SCAN: %u results\n", results_len);
    if (results_len) {
        esp_hid_scan_result_t *r = results;
        esp_hid_scan_result_t *cr = NULL;
        while (r) {
            printf("RSSI: %d, ", r->rssi);
            cr = r;
            printf("NAME: %s ", r->name ? r->name : "");
            //打印mac地址
            printf("bda mac: "ESP_BD_ADDR_STR, ESP_BD_ADDR_HEX(r->bda));
            printf("\n");
            r = r->next;
        }
        if (cr) {
            //连接指定的设备
            esp_hidh_dev_open(cr->bda);
            //esp_hidh_dev_open(cr->bda, cr->transport, cr->ble.addr_type);
        }
        //free the results
        esp_hid_scan_results_free(results);
    }
    
    return true;
}






bool gamepad::isConnected() {
    return gpIsConnected();
}




void gamepad::attach(callback_t callback) {
    _callback_event = callback;
}


void gamepad::attachOnConnect(callback_t callback) {
    _callback_connect = callback;

}

void gamepad::attachOnDisconnect(callback_t callback) {
    _callback_disconnect = callback;
}

void gamepad::_event_callback(void *object, gp_t data, gp_event_t event) {
    gamepad* This = (gamepad*) object;

    memcpy(&This->data, &data, sizeof(gp_t));
    memcpy(&This->event, &event, sizeof(gp_event_t));

    if (This->_callback_event){
        This->_callback_event();
    }
}


void gamepad::_connection_callback(void *object, uint8_t is_connected) {
    gamepad* This = (gamepad*) object;

    if (is_connected) {
      //  delay(250);    // ToDo: figure out how to know when the channel is free again so this delay can be removed

        if (This->_callback_connect){
            This->_callback_connect();
        }
    }
    else {
        if (This->_callback_disconnect){
            This->_callback_disconnect();
        }
    }
}

#if !defined(NO_GLOBAL_INSTANCES)
  gamepad GP;
#endif
