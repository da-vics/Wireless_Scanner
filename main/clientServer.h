#ifndef CLIENT_SERVER_H
#define CLIENT_SERVER_H

#include "esp_log.h"
#include <stdio.h>
#include <string.h>

#include "esp_http_client.h"
#include "cJSON.h"
#include "inits.h"

#define TAG "TEST"
#define acquire_word "{\"MasterKey\":\"SMARTERDATA\"}"

cJSON *postdata;

int ret = 0;
bool GottenId = false;
char buffer[300], deviceID[70];
//Server Address default
char ACQUIRE_ID[150] = "http://192.168.43.240:80/fieldadmin/api/RegisterFieldDevice";

//init postjson object 
void createDataJsonObject()
{
	postdata = cJSON_CreateObject();
	cJSON_AddStringToObject(postdata, "DeviceId", "temp");
	cJSON_AddStringToObject(postdata, "Data", "temp");
}

//post contents
void post_content(char *URL,char *data)
{
	esp_http_client_config_t config = {
		.url = URL,
		// .cert_pem = (char *) server_root_cert_pem_start,
	};

	esp_http_client_handle_t client = esp_http_client_init(&config);

	// POST
	esp_http_client_set_url(client, URL);
	esp_http_client_set_method(client, HTTP_METHOD_POST);
	esp_http_client_set_header(client, "Content-Type", "application/json");
	esp_http_client_set_post_field(client, data, strlen(data));
	esp_err_t err = esp_http_client_perform(client);

	if (err == ESP_OK)
	{

		int statusCode = esp_http_client_get_status_code(client);
		ESP_LOGI(TAG, "HTTP POST Status = %d, content_length = %d", statusCode, esp_http_client_get_content_length(client));

		switch (statusCode)
		{

		case 200:
			ret = esp_http_client_read(client, (char *)buffer, 300);
			buffer[ret] = '\0';
			if (strstr(buffer, "deviceID"))
				GottenId = true;
			printf("%s\n", buffer);
			cJSON *json = cJSON_Parse(buffer);
			sprintf(deviceID, "%s", cJSON_GetObjectItem(json, "deviceID")->valuestring);
			printf("%s\n", deviceID);

			// char *test = cJSON_Print(postdata);
			// printf("%s\n", test);
			break;

		case 404:
			break;
		case 501:
			break;
		default:
			break;
		}
	        } 
			
			else {
	            ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
	        }
	        esp_http_client_cleanup(client);
}


#endif //CLIENT_SERVER_H
