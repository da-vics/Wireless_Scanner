#include "sysTasks.h"

Configurations dataConfigs;
void app_main(void)
{
  init();
  init_nvs();
  wifi_init_sta();

  GetNvsData(&dataConfigs);
  initialProcess(&dataConfigs);

  if(strlen(dataConfigs.DeviceID)>1)
    printf("%s\n","yes");
  else
    printf("%s\n","no");

  xTaskCreate(BarCodeRx_Task, "DataUart_Rx_Task", 4096, NULL, 12, NULL);
  xTaskCreate(ConfigDataRx_Task, "DataUart_Rx_Task", 4096, NULL, 11, NULL);
}
