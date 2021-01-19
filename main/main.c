#include "sysTasks.h"

void app_main(void)
{
  init();
  init_nvs();

  GetNvsData(&dataConfigs);
  initialProcess(&dataConfigs);

  wifi_init_sta();
  createDataJsonObject();

  xTaskCreatePinnedToCore(BarCodeRx_Task, "BarCodeUart_Rx_Task", 4096, NULL, 13, NULL,tskNO_AFFINITY);
  xTaskCreate(ConfigDataRx_Task, "ConfigUart_Rx_Task", 4096, NULL, 11, NULL);
}
