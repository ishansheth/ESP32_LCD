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
        
        

# ILI9341 with XPT2046 pin connections

LCD ILI9341 comes with the touch controller XPT 2046. LCD and touch controller both are connected to ESP32 on SPI bus. 
As shown in the image of the back side of LCD, there are different set of pins for both of them and they needs to be connected to the GPIO pins of ESP32.

<img src="https://github.com/ishansheth/ESP32_LCD/blob/gui_development/images/ili9341_backside.jpg" alt="drawing" width="400" height="400"/>

After connecting LCD pins, too GPIO, these configuration must be done in the configuration of the project. For this esp-idf provides menuconfig.
`idf.py menuconfig` command opens the menuconfig. Select `Component Config->LVGL TFT Display controller->Display Pin Assignments` and enter the following values

## LCD Pin Assignments to ESP32 GPIO number

| Pin Name   |        Menuconfig              | GPIO PIN number |
|------------|--------------------------------|-----------------|
| SDO(MISO ) |        GPIO for MOSI           |       14        |
| SDI(MOSI)  |        GPIO for MISO           |       34        |
| SCK(CLK)   | GPIO for CLK(SCK/Serial Clock) |       23        |
|    CS      | GPIO for CS (Slave select)     |       27        |
|    DC      | GPIO for DC (Data/Command)     |       22        |
|    GND     |                                |  ESP32 GND Pin  |
|    VCC     |                                |  ESP32 Vcc Pin  |
|    LED     |                                |  ESP32 Vcc Pin  |
|    Reset   |        GPIO for Reset          |       12        |


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




