#ifndef INIT_H
#define INIT_H

#include "driver/uart.h"    
#include "nvs_flash.h"

#define TXD_PIN 18
#define RXD_PIN 19
#define TX_Config 4
#define RX_Config 5

typedef enum
{
	Normal,
	StopWifi
}Operations;

typedef struct
{
	char Url[150];
	char WifiSSID[32];
	char WifiPassword[60];
	char DeviceID[70];
} Configurations;

static const int RX_BUF_SIZE = 1024;
bool nvs_started = false;

// initialise UART
void init(){

    const uart_config_t uart_config ={

	.baud_rate = 9600,
	.data_bits = UART_DATA_8_BITS,
	.parity = UART_PARITY_DISABLE,
	.stop_bits = UART_STOP_BITS_1,
	.flow_ctrl = UART_HW_FLOWCTRL_DISABLE };

	uart_param_config(UART_NUM_1, &uart_config);
	uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
	uart_driver_install(UART_NUM_1, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
//.....................

	const uart_config_t uart_config2 ={

	.baud_rate = 9600,
	.data_bits = UART_DATA_8_BITS,
	.parity = UART_PARITY_DISABLE,
	.stop_bits = UART_STOP_BITS_1,
	.flow_ctrl = UART_HW_FLOWCTRL_DISABLE };

	uart_param_config(UART_NUM_2, &uart_config2);
	uart_set_pin(UART_NUM_2, TX_Config, RX_Config, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
	uart_driver_install(UART_NUM_2, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
}

//Init NVS Flash
void init_nvs(){

	esp_err_t err_v;
	err_v = nvs_flash_init();
	if (err_v == ESP_ERR_NVS_NO_FREE_PAGES || err_v == ESP_ERR_NVS_NEW_VERSION_FOUND){
		ESP_ERROR_CHECK(nvs_flash_erase());
		err_v = nvs_flash_init();
	}
	ESP_ERROR_CHECK(err_v);

    	if (err_v == ESP_OK)
		{
			printf("initializing nvs");
			nvs_started = true;

		}

	esp_err_t err_v2;
	err_v2 = nvs_flash_init_partition("configs");

	if (err_v2 == ESP_ERR_NVS_NO_FREE_PAGES || err_v2 == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase_partition("configs"));
		err_v2 = nvs_flash_init_partition("configs");
	}
	if (err_v2 == ESP_OK)
	{
		printf("initializing nvs");
		nvs_started = true;
	}
	ESP_ERROR_CHECK(err_v2);
}

//reset struct data
void resetConfigurationDat(Configurations *config){

	memset(config->Url, 0, sizeof(config->Url));
	memset(config->WifiSSID, 0, sizeof(config->WifiSSID));
	memset(config->WifiPassword, 0, sizeof(config->WifiPassword));
	memset(config->DeviceID, 0, sizeof(config->DeviceID));
}

//process ConfigurationData
Operations process_ConfigData(char *data, Configurations *config){

	int start = -1, end = -1;

	if(data[0] == '<')
		start = 0;
	
	else if(strstr(data,"URL:"))
	{
		data = strstr(data, ":");
		int pos = 0;
		for (int i = 2; i < strlen(data);++i)
		{	
			if(data[i] == '>')
				break;
			config->Url[pos++] = data[i];
			config->Url[pos] = NULL;
		}
		return Normal;
	}

	if(start!=-1){
		for (int i = start; i < strlen(data);++i){
			if(data[i] == '>'){
				end = i;
				break;
			}
		}

			//data:rtt>
	if(start!=-1 && end!=-1){
		int pos = 0;
		for (int i = start + 1; i < strlen(data);++i){

			if(data[i] == ':'){

				pos = 0;
				for (int j = i + 1; j < strlen(data);++j){

					if(data[j] == '>')
						break;

					config->WifiPassword[pos++] = data[j];
					config->WifiPassword[pos] = NULL;
				}
					break;
			}
			config->WifiSSID[pos++] = data[i];
			config->WifiSSID[pos] = NULL;
		}
		return StopWifi;
	}//process

	}

	return Normal;
}

#endif //INIT_H
