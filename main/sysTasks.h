#ifndef SYS_TASK_H
#define SYS_TASK_H

//basic FreeRTOS headers
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

#include "esp_wifi.h"

#include "clientServer.h"

#define Log "DEBUG"

Configurations dataConfigs;

// static SemaphoreHandle_t GetID;

uint8_t BarData[500];
uint8_t ConfigData[200];

//default values
char CONFIG_WIFI_SSID[32] = "project";
char CONFIG_WIFI_PASSSWORD[60] = "projectdev";

#define BIT_0 (1 << 0)
#define nvs_partName "configs"
#define nvs_storageName "configdata"

bool wifiConnectionStatus = false;
static EventGroupHandle_t s_wifi_event_group;

//Send Data
void sendData(const char *data,uart_port_t port_num){
	
	const int len = strlen(data);
	uart_write_bytes(port_num, data, len);
}

void SetConfigurations(Configurations *);
void GetNvsData(Configurations *);

void getDeviceId()
{	
	if(strlen(deviceID)<=1){

		if(wifiConnectionStatus){
			post_content(ACQUIREID_URL, AccessKey, GETDEVICE_ID);
			sprintf(dataConfigs.DeviceID, "%s", deviceID);
			printf("%s\n",dataConfigs.DeviceID);
			SetConfigurations(&dataConfigs);
		}
		else
			ESP_LOGD(Log, "wifi not connected");
	}
	else
		ESP_LOGD(Log, "DeviceId available");
}

//Wifi event Handler
static esp_err_t event_handler(void *ctx, system_event_t *event){

	switch (event->event_id){

	case SYSTEM_EVENT_STA_START:
	{
		esp_wifi_connect();
		break;
	}

	case SYSTEM_EVENT_STA_GOT_IP:
	{
		ESP_LOGI(TAG, "got ip:%s", ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
		xEventGroupSetBits(s_wifi_event_group, BIT_0);
		wifiConnectionStatus = true;
		break;
	}

	case SYSTEM_EVENT_STA_DISCONNECTED:
	{
		wifiConnectionStatus = false;
		esp_wifi_connect();
		xEventGroupClearBits(s_wifi_event_group, BIT_0);
		break;
	}

	default:
		break;
	}
	return ESP_OK;
}

//
void wifi_init_(Configurations *configs)
{
	tcpip_adapter_init();
	s_wifi_event_group = xEventGroupCreate();
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

	wifi_config_t wifi_config = {
		.sta = {.ssid = " ", .password = " ", .bssid_set = false},
	};

	sprintf((char *)wifi_config.sta.password, "%s", (char *)configs->WifiPassword);
	sprintf((char *)wifi_config.sta.ssid, "%s", (char *)configs->WifiSSID);

	wifi_config.sta.bssid_set = false;

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
	ESP_ERROR_CHECK(esp_wifi_start());

	ESP_LOGI(TAG, "wifi_init_sta finished.");
	ESP_LOGI(TAG, "connect to ap SSID:%s password:%s", wifi_config.sta.ssid, wifi_config.sta.password);

	xEventGroupWaitBits(s_wifi_event_group, BIT_0, false, true, 8000 / portTICK_PERIOD_MS);
}

//Start Wifi Task
void wifi_init_sta()
{
	s_wifi_event_group = xEventGroupCreate();
	tcpip_adapter_init();
	ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

	wifi_config_t wifi_config = {
		.sta = {.ssid = " ", .password = " ", .bssid_set = false},
	};

	sprintf((char *)wifi_config.sta.password, "%s", (char *)CONFIG_WIFI_PASSSWORD);
	sprintf((char *)wifi_config.sta.ssid, "%s", (char *)CONFIG_WIFI_SSID);

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
	ESP_ERROR_CHECK(esp_wifi_start());
	ESP_LOGI(TAG, "wifi_init_sta finished.");
	ESP_LOGI(TAG, "connect to ap SSID:%s password:%s", CONFIG_WIFI_SSID, CONFIG_WIFI_PASSSWORD);
	xEventGroupWaitBits(s_wifi_event_group, BIT_0, false, true, 5000 / portTICK_PERIOD_MS);
}

void initialProcess(Configurations *const);

void processScannerData(){

	char *tempDat = strstr((char*)BarData,"config:");

	if(tempDat){

		bool reconnect = false;
		Configurations config;
		tempDat = strstr(tempDat,"{");
		printf("%s\n", tempDat);
		cJSON *json = cJSON_Parse(tempDat);

		sprintf(config.DeviceID, "%s", deviceID);
		sprintf(config.Url_Upload, "%s", cJSON_GetObjectItem(json, "Url_Upload")->valuestring);
		sprintf(config.Url_Key, "%s", cJSON_GetObjectItem(json, "Url_Key")->valuestring);
		sprintf(config.WifiSSID, "%s", cJSON_GetObjectItem(json, "WifiSSID")->valuestring);
		sprintf(config.WifiPassword, "%s", cJSON_GetObjectItem(json, "WifiPassword")->valuestring);
		sprintf(config.Location, "%s", cJSON_GetObjectItem(json, "Location")->valuestring);
		char *key = cJSON_GetObjectItem(json, "AccessKey")->valuestring;

		if(!strcmp(config.WifiSSID,CONFIG_WIFI_SSID) || !strcmp(config.WifiPassword,CONFIG_WIFI_PASSSWORD))
			reconnect = true;

		char temp1[15] , temp2[35];

		int pos = 0;
		bool next = false;

		for (int i = 0; i < strlen(key); ++i){ 

			if(key[i] == ':'){
				next = true;
				pos = 0;
				continue;
			}

			if(!next){
				temp1[pos++] = key[i];
				temp1[pos] = '\0';
			}

			if(next){
				temp2[pos++] = key[i];
				temp2[pos] = '\0';
			}
		}

		cJSON *dat;
		dat = cJSON_CreateObject();
		cJSON_AddStringToObject(dat, temp1, temp2);
		sprintf(config.AcessKey,"%s",cJSON_Print(dat));
		printf("%s\n",config.AcessKey);

		SetConfigurations(&config);
		initialProcess(&config);	

		if(reconnect){
			ESP_ERROR_CHECK(esp_wifi_stop());
			wifi_init_(&config);
		}

		}//

		else{
			char temp[700];
			printf("%s\n", (char *)BarData);
			sprintf(temp, "%s%s%s", (char *)BarData, ":", Location);
			printf("%s\n", temp);
			cJSON_GetObjectItem(postdata, "Data")->valuestring = temp;
			cJSON_GetObjectItem(postdata, "deviceID")->valuestring = deviceID;
			char *CurrentData = cJSON_Print(postdata);
			post_content(DATAPOST_URL, CurrentData, UPLOAD_DATA);
		}

}

//BarCode RX Task
static void BarCodeRx_Task(void *args){

	while (1){

		int rx = uart_read_bytes(UART_NUM_1, BarData, RX_BUF_SIZE * 2, 1000 / portTICK_RATE_MS);

		if (rx > 0){
			printf("%s\n", BarData);
			processScannerData();
			memset(BarData, 0, sizeof(BarData));
		}//
			vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}//

//ConfigData RX Task
static void ConfigDataRx_Task(void *args){

	while (1){

		if(!GottenId)
		getDeviceId();

		int rx = uart_read_bytes(UART_NUM_2, ConfigData, RX_BUF_SIZE * 2, 1000 / portTICK_RATE_MS);
		if (rx > 0){

			Configurations config;
			Operations ops;
			printf("%s\n",(char*)ConfigData);
			GetNvsData(&config);
			ops = process_ConfigData((char *)ConfigData,&config);
			SetConfigurations(&config);
			initialProcess(&config);
			memset(ConfigData, 0, sizeof(ConfigData));
			if(ops == StopWifi){

				ESP_ERROR_CHECK(esp_wifi_stop());
				wifi_init_(&config);
			}
		}
			vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}

//update nvs data
void SetConfigurations(Configurations *configs){

	nvs_handle handle;
	ESP_ERROR_CHECK(nvs_open_from_partition(nvs_partName,nvs_storageName, NVS_READWRITE, &handle));

	ESP_ERROR_CHECK(nvs_set_blob(handle, nvs_storageName, (void *)configs, sizeof(Configurations)));
	nvs_commit(handle);
	nvs_close(handle);
}

//get Data from nvs
void GetNvsData(Configurations *configs){

	nvs_handle handle;
	ESP_ERROR_CHECK(nvs_open_from_partition(nvs_partName, nvs_storageName, NVS_READWRITE, &handle));
	size_t config_size = sizeof(Configurations);
	resetConfigurationDat(configs);

	esp_err_t result = nvs_get_blob(handle, nvs_storageName, (void *)configs, &config_size);
	nvs_close(handle);

	switch (result){
		
	case ESP_ERR_NOT_FOUND:
	case ESP_ERR_NVS_NOT_FOUND:
		ESP_LOGE("storage", "value not found");
		break;

	case ESP_ERR_NVS_INVALID_LENGTH:
	ESP_LOGE("storage", "length issues");
	break;

	case ESP_OK:{
		ESP_LOGI("storage", "found");
		break;
	}

	default:
	ESP_LOGI("storage", "nothing");
		break;
	}
}//

void initialProcess(Configurations *const configs){

	if (strlen(configs->WifiSSID) > 1)
	{
		memset(CONFIG_WIFI_SSID, 0, sizeof(CONFIG_WIFI_SSID));
		memcpy((void *)&CONFIG_WIFI_SSID, (void *)configs->WifiSSID, sizeof(configs->WifiSSID));
	}

	if (strlen(configs->WifiPassword) > 1)
	{
		memset(CONFIG_WIFI_PASSSWORD, 0, sizeof(CONFIG_WIFI_PASSSWORD));
		memcpy((void *)&CONFIG_WIFI_PASSSWORD, (void *)configs->WifiPassword, sizeof(configs->WifiPassword));
	}

	if (strlen(configs->Url_Key) > 1)
	{
		memset(ACQUIREID_URL, 0, sizeof(ACQUIREID_URL));
		memcpy((void *)&ACQUIREID_URL, (void *)configs->Url_Key, sizeof(configs->Url_Key));
	}

	if (strlen(configs->Url_Upload) > 1)
	{
		memset(DATAPOST_URL, 0, sizeof(DATAPOST_URL));
		memcpy((void *)&DATAPOST_URL, (void *)configs->Url_Upload, sizeof(configs->Url_Upload));
	}

	if(strlen(configs->DeviceID)>1)
	{
		memset(deviceID, 0, sizeof(deviceID));
		memcpy((void *)&deviceID, (void *)configs->DeviceID, sizeof(configs->DeviceID));
	}

	if(strlen(configs->DeviceID)>1)
	{
		memset(AccessKey, 0, sizeof(AccessKey));
		memcpy((void *)&AccessKey, (void *)configs->AcessKey, sizeof(configs->AcessKey));
	}

	if(strlen(configs->Location)>1)
	{
		memset(Location, 0, sizeof(Location));
		memcpy((void *)&Location, (void *)configs->Location, sizeof(configs->Location));
	}

	ESP_LOGI(Log,"%s\n", CONFIG_WIFI_PASSSWORD);
	ESP_LOGI(Log,"%s\n", CONFIG_WIFI_SSID);
	ESP_LOGI(Log,"%s\n", ACQUIREID_URL);
	ESP_LOGI(Log,"%s\n", DATAPOST_URL);
	ESP_LOGI(Log,"%s\n", deviceID);
	ESP_LOGI(Log,"%s\n", AccessKey);
	ESP_LOGI(Log,"%s\n", Location);
}

#endif //SYS_TASK_H
