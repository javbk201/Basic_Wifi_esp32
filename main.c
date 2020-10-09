#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event_loop.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"

#define WIFI_SSID ""
#define WIFI_PASS ""


xSemaphoreHandle wifi_on;

// Wifi event handler
static esp_err_t wifi_event_handler(void *ctx, system_event_t *event){
    switch(event->event_id){

       case SYSTEM_EVENT_STA_START:
           esp_wifi_connect();
           break;

	   case SYSTEM_EVENT_STA_GOT_IP:
          xSemaphoreGive(wifi_on);
          break;

	   default:
          break;
    }

	return ESP_OK;
}


// Main task
void main_task(){

	printf("Esperando Conexion\r\n");
	xSemaphoreTake(wifi_on, portMAX_DELAY);
	printf("ESP32 Conectado  [OK] \r\n");
	while(1){
		vTaskDelay(500);
	}
}


// Main application
void app_main(){
	//Inicialización de Stack TCP
	nvs_flash_init();
		tcpip_adapter_init();
	//Inicialización de Driver WiFi
	ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler, NULL));
	//Inicialización WiFi
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
	//Configurar SSID y PASS
	wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };
		ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
		ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
		ESP_ERROR_CHECK(esp_wifi_start());

	//Creacion Semaphoro Binario
	wifi_on = xSemaphoreCreateBinary();

	//Tarea principal
    xTaskCreate(main_task, "main_task", 20000, NULL, 5, NULL);
}
