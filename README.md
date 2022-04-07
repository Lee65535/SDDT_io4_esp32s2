# ESP32-S2 Ongeki IO4

Forked from GEEKiDoS's esp32-s2 ongeki con 

esp-idf version: 4.4

### esp-idf Files Need to Be Modified:

All files below are in your esp-idf folder (typically C:\Users\<you>\esp-idf\ if Windows)

- components/tinyusb/Kconfig
- components/tinyusb/additions/include/tusb_config.h

(above files are attached)

- components/tinyusb/addition/src/descriptors_control.c (modify hid_description_report at line 24)

