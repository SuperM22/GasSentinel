set(component_srcs "aux.c")

idf_component_register(SRCS "${component_srcs}"
                       PRIV_REQUIRES esp_adc
                       PRIV_REQUIRES esp_wifi 
                       PRIV_REQUIRES driver
                       PRIV_REQUIRES nvs_flash
                       INCLUDE_DIRS ".")
