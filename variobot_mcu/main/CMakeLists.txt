idf_component_register(
  SRC_DIRS "src"
  INCLUDE_DIRS "include"
  PRIV_REQUIRES esp_driver_gpio esp_driver_pcnt esp_timer log
)
