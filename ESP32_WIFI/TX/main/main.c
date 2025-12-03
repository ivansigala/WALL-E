/* ROBOT BRIDGE (Connects to MCXN947 Robot) */
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_event.h"
#include "driver/gpio.h"
#include "driver/spi_slave.h"
#include "freertos/FreeRTOS.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_mac.h"
#include "nvs_flash.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#define ESP_CHANNEL 1
#define SPI_PAYLOAD_SIZE 40 

// Pin Config
#define SPI_MOSI_GPIO 8
#define SPI_MISO_GPIO 9
#define SPI_CLK_GPIO  4
#define SPI_CS_GPIO   3

static const char *TAG = "RobotBridge";

/* TARGET: MAC Address of the REMOTE CONTROL ESP32 */
static uint8_t peer_mac[ESP_NOW_ETH_ALEN] = {0xdc, 0x06, 0x75, 0x67, 0x93, 0x0c}; 

/* Structures */
typedef struct __attribute__((packed)) {
    uint32_t header; float vx; float vy; float phi; uint32_t buttons; uint32_t timestamp;
} RemoteCommand_t;

typedef struct __attribute__((packed)) {
    uint32_t header; float s1; float s2; float s3; float s4; uint16_t a1; uint16_t a2; uint16_t a3; uint16_t a4; uint32_t flags; uint32_t ts;
} RobotTelemetry_t;

/* Global Storage */
static RemoteCommand_t latest_command = {0}; 
static portMUX_TYPE data_mutex = portMUX_INITIALIZER_UNLOCKED;
static QueueHandle_t send_queue;

/* SPI Buffers */
WORD_ALIGNED_ATTR uint8_t spi_tx_buf[SPI_PAYLOAD_SIZE];
WORD_ALIGNED_ATTR uint8_t spi_rx_buf[SPI_PAYLOAD_SIZE];

/* Callbacks */
void recv_cb(const esp_now_recv_info_t *info, const uint8_t *data, int len){
    /* FIX: Accept 40 bytes (SPI_PAYLOAD_SIZE), even if struct is smaller */
    if (len == SPI_PAYLOAD_SIZE) { 
        taskENTER_CRITICAL(&data_mutex);
        /* We are the Robot, so we RECEIVE Commands from the Remote */
        memcpy(&latest_command, data, sizeof(RemoteCommand_t));
        taskEXIT_CRITICAL(&data_mutex);
    }
}

void send_cb(const uint8_t *mac, esp_now_send_status_t status) {}

/* Tasks */
static void spi_slave_task(void *pv){
    spi_slave_transaction_t t;
    esp_err_t ret;
    uint8_t packet[SPI_PAYLOAD_SIZE];

    while(1){
        /* 1. Prepare MISO (Command to Robot MCU) */
        memset(spi_tx_buf, 0, SPI_PAYLOAD_SIZE);
        taskENTER_CRITICAL(&data_mutex);
        memcpy(spi_tx_buf, &latest_command, sizeof(RemoteCommand_t));
        taskEXIT_CRITICAL(&data_mutex);

        /* 2. Transaction */
        memset(&t, 0, sizeof(t));
        t.length = SPI_PAYLOAD_SIZE * 8;
        t.tx_buffer = spi_tx_buf;
        t.rx_buffer = spi_rx_buf;
        
        ret = spi_slave_transmit(SPI2_HOST, &t, portMAX_DELAY);

        if(ret == ESP_OK){
            /* 3. Handle MOSI (Telemetry from Robot MCU) -> Send to Air */
            memcpy(packet, spi_rx_buf, SPI_PAYLOAD_SIZE);
            xQueueSend(send_queue, &packet, 0);

        ESP_LOGI(TAG, "SYNC | CMD_VX: %.2f | CMD_VY: %.2f | CMD_PHI: %.2f | TEL_M1: %.2f", 
                    latest_command.vx, latest_command.vy, latest_command.phi,
                    ((RobotTelemetry_t*)packet)->s1);
            
        }
    }
}

static void send_task(void *pv){
    uint8_t packet[SPI_PAYLOAD_SIZE];
    while(xQueueReceive(send_queue, &packet, portMAX_DELAY)){
        esp_now_send(peer_mac, packet, SPI_PAYLOAD_SIZE);
    }
}

/* Init */
void app_main(void){
    nvs_flash_init();
    esp_netif_init();
    esp_event_loop_create_default();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();
    
    esp_now_init();
    esp_now_register_recv_cb(recv_cb);
    esp_now_register_send_cb(send_cb);
    
    esp_now_peer_info_t peer = {};
    memcpy(peer.peer_addr, peer_mac, 6);
    peer.channel = ESP_CHANNEL;
    peer.encrypt = false;
    esp_now_add_peer(&peer);

    send_queue = xQueueCreate(10, SPI_PAYLOAD_SIZE);

    spi_bus_config_t buscfg = {.mosi_io_num=8, .miso_io_num=9, .sclk_io_num=4, .quadwp_io_num=-1, .quadhd_io_num=-1};
    spi_slave_interface_config_t slvcfg = {.mode=0, .spics_io_num=3, .queue_size=3, .flags=0};
    spi_slave_initialize(SPI2_HOST, &buscfg, &slvcfg, SPI_DMA_CH_AUTO);

    xTaskCreate(spi_slave_task, "spi", 4096, NULL, 5, NULL);
    xTaskCreate(send_task, "send", 4096, NULL, 5, NULL);
}