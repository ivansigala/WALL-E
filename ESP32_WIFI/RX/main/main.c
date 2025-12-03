/* ROBOT BRIDGE (Connects to MCXN947 Robot) */
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_event.h"
#include "driver/gpio.h"
#include "driver/spi_slave.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_mac.h"
#include "nvs_flash.h"

#define ESP_CHANNEL 1
#define SPI_PAYLOAD_SIZE 40 

// Pin Config (ESP32-C3 Super Mini)
#define SPI_MOSI_GPIO 8
#define SPI_MISO_GPIO 9
#define SPI_CLK_GPIO  4
#define SPI_CS_GPIO   3

static const char *TAG = "RobotBridge";

/* TARGET: MAC Address of the REMOTE CONTROL ESP32 */
/* Ensure this matches your Remote ESP32's MAC */
static uint8_t peer_mac[ESP_NOW_ETH_ALEN] = {0x8c, 0xd0, 0xb2, 0xa7, 0xed, 0xe4}; 

/* --- DATA STRUCTURES --- */

/* Command: Remote -> Robot (Via SPI RX -> ESP-NOW TX) */
typedef struct __attribute__((packed)) {
    uint32_t header;        
    float vx;               
    float vy;               
    float phi;              
    uint32_t buttons;       
    uint32_t timestamp;     
} RemoteCommand_t;

/* Telemetry: Robot -> Remote (Via ESP-NOW RX -> SPI TX) */
typedef struct __attribute__((packed)) {
    uint32_t packet_header; 
    float speed_m1;
    float speed_m2;
    float speed_m3;
    float speed_m4;
    uint16_t adc_m1;
    uint16_t adc_m2;
    uint16_t adc_m3;
    uint16_t adc_m4;
    uint32_t reserved_flags;
    uint32_t timestamp;
} RobotTelemetry_t;

/* Global Storage */
/* 1. Command received from Air, waiting to be sent to Robot MCU via MISO */
static RemoteCommand_t latest_command_from_remote = {0}; 
static portMUX_TYPE data_mutex = portMUX_INITIALIZER_UNLOCKED;

/* 2. Telemetry received from Robot MCU via MOSI, saved for record keeping */
static RobotTelemetry_t last_sent_telemetry = {0}; 

/* SPI Buffers */
WORD_ALIGNED_ATTR uint8_t spi_tx_buf[SPI_PAYLOAD_SIZE]; // MISO
WORD_ALIGNED_ATTR uint8_t spi_rx_buf[SPI_PAYLOAD_SIZE]; // MOSI

/* --- CALLBACKS --- */

/* Received Data from Remote via ESP-NOW */
void recv_cb(const esp_now_recv_info_t *info, const uint8_t *data, int len){
    if (len == SPI_PAYLOAD_SIZE) { 
        taskENTER_CRITICAL(&data_mutex);
        memcpy(&latest_command_from_remote, data, sizeof(RemoteCommand_t));
        taskEXIT_CRITICAL(&data_mutex);
    }
}

/* Send Complete Callback */
void send_cb(const uint8_t *mac, esp_now_send_status_t status) {
    // Optional: Add error counting here if needed
}

/* --- MAIN SPI TASK --- */
static void spi_slave_task(void *pv){
    spi_slave_transaction_t t;
    esp_err_t ret;
    static uint32_t transaction_count = 0;

    ESP_LOGI(TAG, "SPI Slave Task Started. Waiting for Master...");

    while(1){
        /* 1. PREPARE DATA FOR MISO (Command to Robot) */
        // Load the latest command we got from the Remote into the SPI TX buffer
        memset(spi_tx_buf, 0, SPI_PAYLOAD_SIZE);
        taskENTER_CRITICAL(&data_mutex);
        memcpy(spi_tx_buf, &latest_command_from_remote, sizeof(RemoteCommand_t));
        taskEXIT_CRITICAL(&data_mutex);

        /* 2. SETUP TRANSACTION */
        memset(&t, 0, sizeof(t));
        t.length = SPI_PAYLOAD_SIZE * 8; // Bits
        t.tx_buffer = spi_tx_buf;
        t.rx_buffer = spi_rx_buf;
        
        /* 3. WAIT FOR TRANSACTION (Blocking) */
        // The task sleeps here until the Robot MCU (Master) pulls CS low
        ret = spi_slave_transmit(SPI2_HOST, &t, portMAX_DELAY);

        if(ret == ESP_OK){
            /* 4. HANDLE RECEIVED DATA (Telemetry from Robot) */
            
            // A. Save the packet to information structure as requested
            memcpy(&last_sent_telemetry, spi_rx_buf, sizeof(RobotTelemetry_t));

            // B. Send immediately via ESP-NOW (No extra task)
            esp_err_t wifi_ret = esp_now_send(peer_mac, spi_rx_buf, SPI_PAYLOAD_SIZE);
            
            /* 5. LOGGING (Throttled) */
            transaction_count++;
            if(transaction_count % 100 == 0){
                ESP_LOGI(TAG, "SYNC | CMD_VX: %.2f | TEL_M1: %.2f | ESP-NOW: %s", 
                         latest_command_from_remote.vx, 
                         last_sent_telemetry.speed_m1,
                         (wifi_ret == ESP_OK) ? "OK" : "FAIL");
            }
        }
    }
}

/* --- INITIALIZATION --- */
void app_main(void){
    // 1. NVS & WiFi Init
    nvs_flash_init();
    esp_netif_init();
    esp_event_loop_create_default();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();
    
    // 2. ESP-NOW Init
    esp_now_init();
    esp_now_register_recv_cb(recv_cb);
    esp_now_register_send_cb(send_cb);
    
    // 3. Register Peer (Remote Control)
    esp_now_peer_info_t peer = {};
    memcpy(peer.peer_addr, peer_mac, 6);
    peer.channel = ESP_CHANNEL;
    peer.encrypt = false;
    esp_now_add_peer(&peer);

    // 4. SPI Slave Init
    spi_bus_config_t buscfg = {
        .mosi_io_num=SPI_MOSI_GPIO, 
        .miso_io_num=SPI_MISO_GPIO, 
        .sclk_io_num=SPI_CLK_GPIO, 
        .quadwp_io_num=-1, 
        .quadhd_io_num=-1
    };
    spi_slave_interface_config_t slvcfg = {
        .mode=0, 
        .spics_io_num=SPI_CS_GPIO, 
        .queue_size=3, 
        .flags=0
    };
    spi_slave_initialize(SPI2_HOST, &buscfg, &slvcfg, SPI_DMA_CH_AUTO);

    // 5. Start Single Task
    xTaskCreate(spi_slave_task, "spi", 4096, NULL, 5, NULL);
}