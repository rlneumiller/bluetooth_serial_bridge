#include <Arduino.h>
#include "BluetoothSerial.h" // Library for Bluetooth Serial
//#include <esp_bt_device.h>
#include <nvs_flash.h>
#include <esp_bt_main.h>

#include <SPIFFS.h>
#include <WiFi.h>
#include "esp_bt_device.h"
#include "esp_bt.h"
#include "esp_spp.h"

BluetoothSerial SerialBT;

#define SPP_TAG "SPP_ACCEPTOR_DEMO"
#define SPP_SERVER_NAME "ESP32_Printer"

static uint32_t esp_spp_handle = 0;

void esp_spp_cb(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
    switch (event) {
    case ESP_SPP_START_EVT:
        Serial.println("ESP_SPP_START_EVT");
        esp_bt_dev_set_device_name(SPP_SERVER_NAME);
        #if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 0, 0)
          esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
        #else
          esp_bt_gap_set_scan_mode(ESP_BT_SCAN_MODE_CONNECTABLE_DISCOVERABLE);
        #endif        
        //esp_bt_gap_set_scan_mode(ESP_BT_SCAN_MODE_CONNECTABLE_DISCOVERABLE);
        esp_spp_start_srv(ESP_SPP_SEC_AUTHENTICATE, ESP_SPP_ROLE_SLAVE, 0, SPP_SERVER_NAME);
        break;
    case ESP_SPP_DISCOVERY_COMP_EVT:
        Serial.println("ESP_SPP_DISCOVERY_COMP_EVT");
        break;
    case ESP_SPP_OPEN_EVT:
        Serial.println("ESP_SPP_OPEN_EVT");
        esp_spp_handle = param->open.handle;
        break;
    case ESP_SPP_CLOSE_EVT:
        Serial.println("ESP_SPP_CLOSE_EVT");
        esp_spp_handle = 0;
        break;
    case ESP_SPP_SRV_OPEN_EVT:
        Serial.println("ESP_SPP_SRV_OPEN_EVT");
        esp_spp_handle = param->open.handle;
        break;
    case ESP_SPP_CL_INIT_EVT:
        Serial.println("ESP_SPP_CL_INIT_EVT");
        break;
    case ESP_SPP_DATA_IND_EVT:
        Serial.printf("ESP_SPP_DATA_IND_EVT len=%d handle=%d\n", param->data_ind.len, param->data_ind.handle);
        Serial.write(param->data_ind.data, param->data_ind.len);
        break;
    case ESP_SPP_CONG_EVT:
        Serial.println("ESP_SPP_CONG_EVT");
        break;
    case ESP_SPP_WRITE_EVT:
        Serial.println("ESP_SPP_WRITE_EVT");
        break;
    case ESP_SPP_INIT_EVT:
        Serial.println("ESP_SPP_INIT_EVT");
        break;
    case ESP_SPP_UNINIT_EVT:
        Serial.println("ESP_SPP_UNINIT_EVT");
        break;
    case ESP_SPP_SRV_STOP_EVT:
        Serial.println("ESP_SPP_SRV_STOP_EVT");
        break;
    case ESP_SPP_VFS_REGISTER_EVT:
        Serial.println("ESP_SPP_VFS_REGISTER_EVT");
        break;
    case ESP_SPP_VFS_UNREGISTER_EVT:
        Serial.println("ESP_SPP_VFS_UNREGISTER_EVT");
        break;
    default:
        break;
    }
}

void setup() {
    if (!SPIFFS.begin(true)) {
        Serial.println("An error occurred while mounting SPIFFS");
        return;
    }    

    Serial.begin(115200);
    SerialBT.begin(SPP_SERVER_NAME); // Start Bluetooth serial with the given name

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    esp_bt_controller_mem_release(ESP_BT_MODE_BLE);
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        Serial.printf("initialize controller failed: %s\n", esp_err_to_name(ret));
        return;
    }
    ret = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT);
    if (ret) {
        Serial.printf("enable controller failed: %s\n", esp_err_to_name(ret));
        return;
    }
    ret = esp_bluedroid_init();
    if (ret) {
        Serial.printf("init bluetooth failed: %s\n", esp_err_to_name(ret));
        return;
    }
    ret = esp_bluedroid_enable();
    if (ret) {
        Serial.printf("enable bluetooth failed: %s\n", esp_err_to_name(ret));
        return;
    }
    ret = esp_spp_init(ESP_SPP_MODE_CB);
    if (ret) {
        Serial.printf("spp init failed: %s\n", esp_err_to_name(ret));
        return;
    }
    esp_spp_register_callback(esp_spp_cb);
}

void loop() {
    static uint8_t data[100];
    int length;

    if (Serial.available()) {
        length = Serial.readBytes(data, sizeof(data));
        if (esp_spp_handle) {
            esp_spp_write(esp_spp_handle, length, data);
        } else {
            Serial.println("Bluetooth not connected");
        }
    }
}

extern "C" {
void app_main() {
    // Your setup code here
    setup(); 

    // Your main loop code here
    while (true) {
        loop();
    }
}
}