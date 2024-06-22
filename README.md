# ESP32_LCD

ESP32 project to display a GUI on ILI9341 320x240 LCD with touch. GUI consist of the clock and stopwatch. 

To set up the ESP-IDF, go through this [link](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/linux-macos-setup.html)

# Used packages
- [LVGL](https://github.com/lvgl/lvgl)
- [LVGL Drivers](https://github.com/lvgl/lvgl_esp32_drivers)
- [esp-idf](https://github.com/espressif/esp-idf)

In order to use LVGL and LVGL driver to display GUI with widgets, specific version of them( git commit hash) must be used.

# Useful esp-idf commands

- To flash the project executable:

    Navigate to folder where repo is cloned  
        
    take the ownership of the USB port where ESP32 is connected
    
        sudo chown <USER> <PORT>
    
    e.g. USER = Monty, PORT = /dev/ttyUSB0
    
    flash the project executable and start monitoring
    
        idf.py -p <PORT> flash monitor
        
    To only monitor the process via serial port
    
        idf.py monitor
        
        

# Project Configuration 

The GPIO pins of the ESP32 are connected to the IO pins of the ILI9341 LCD as shown in the image below. These pin values can be set 
set in sdkconfig file using menuconfig in esp-idf (_idf.py menuconfig_). It can certainly vary depending on the used GPIO pin for particular LCD IO pin and can be set using menuconfig. The below GPIO pin numbers are shown only for the project setup shown in the image below.

## LCD Pin Assignments to ESP32 GPIO number

| Menuconfig Variable Name | GPIO PIN number |
|--------------------------|-----------------|
| CONFIG_LV_TOUCH_SPI_MISO |       34        |
| CONFIG_LV_TOUCH_SPI_MOSI |       14        |
| CONFIG_LV_TOUCH_SPI_CLK  |       26        |
| CONFIG_LV_TOUCH_SPI_CS   |       27        |
| CONFIG_LV_TOUCH_PIN_IRQ  |       13        |
|        DC                |       35        |

## Touchpanel Configuration for ILI93411 320x240

The values of X min/max and Y min/max indicate the raw values from XPT2046 controller when a respective corner of the LCD screen is touched. 
The below values determined when the LCD is used in _landscape_ mode. Depending on the mode (landscape or portrait), the values in italic fonts should be changed/swapped.

| Menuconfig Variable Name | values |
|--------------------------|--------|
|  CONFIG_LV_HOR_RES_MAX   |  320   |
|  CONFIG_LV_VER_RES_MAX   |  240   |
|  CONFIG_LV_TOUCH_X_MIN   |  183   | 
|  CONFIG_LV_TOUCH_Y_MIN   |  134   |
|  CONFIG_LV_TOUCH_X_MAX   |  1959  |
|  CONFIG_LV_TOUCH_Y_MAX   |  1843  |

The below values should be constant as X/Y values require inversion (y means marked in menuconfig GUI)

| Menuconfig Variable Name   | values |
|----------------------------|--------|
| CONFIG_LV_TOUCH_INVERT_X   |   y    |
| CONFIG_LV_TOUCH_INVERT_Y   |   y    |
| CONFIG_LV_TOUCH_DETECT_IRQ |   y    |

The below values are assigned depending on the GPIO pin assignment

| Menuconfig Variable Name   | GPIO PIN number |
|----------------------------|-----------------|
| CONFIG_LV_TOUCH_SPI_MISO   |   25            |
| CONFIG_LV_TOUCH_SPI_MOSI   |   33            |
| CONFIG_LV_TOUCH_SPI_CLK    |   26            |
| CONFIG_LV_TOUCH_SPI_CS     |   32            |
| CONFIG_LV_TOUCH_PIN_IRQ    |   13            |

## Clock Tab Image

<img src="https://github.com/ishansheth/ESP32_LCD/blob/gui_development/images/PXL_20220716_171200853.MP.jpg" alt="drawing" width="400" height="400"/>

## Stopwatch Tab Image

<img src="https://github.com/ishansheth/ESP32_LCD/blob/gui_development/images/PXL_20220811_092030499.MP.jpg" alt="drawing" width="400" height="400"/>

# References

1. [FreeeRTOS Reference books](https://www.freertos.org/Documentation/RTOS_book.html)
2. [ESP-IDF API guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/index.html)
3. [LVGL Widgets documentation](https://docs.lvgl.io/7.11/widgets/index.html)
4. [LVGL with ESP-IDF project](https://docs.lvgl.io/7.11/get-started/espressif.html)
4. [Sample ESP-IDF project with LVGL](https://github.com/lvgl/lv_port_esp32)
5. [Example ESP32 project with LCD](https://github.com/nopnop2002/esp-idf-ili9340)




