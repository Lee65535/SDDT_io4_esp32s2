# ESP32-S2 Ongeki Con 

esp-idf version: 4.4

### esp-idf Files Need to Be Modified:

- components/tinyusb/Kconfig
- components/tinyusb/additions/include/tusb_config.h

(above files are attached)

- components/tinyusb/addition/src/descriptors_control.c (modify hid_description_report at line 24)

