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

<img src="https://github.com/ishansheth/ESP32_LCD/blob/gui_development/images/ili9341_backside.jpg" alt="drawing" width="800" height="400"/>

After connecting LCD pins, too GPIO, these configuration must be done in the configuration of the project. For this esp-idf provides menuconfig.
`idf.py menuconfig` command opens the menuconfig. Select `Component Config->LVGL TFT Display controller->Display Pin Assignments` and enter the following values

## LCD Pin Assignments to ESP32 GPIO number

| Pin Name   |        Menuconfig              | GPIO PIN number |
|------------|--------------------------------|-----------------|
| SDI(MOSI)  |        GPIO for MOSI           |       14        |
| SDO(MISO)  |        GPIO for MISO           |       34        |
| SCK(CLK)   | GPIO for CLK(SCK/Serial Clock) |       23        |
|    CS      | GPIO for CS (Slave select)     |       27        |
|    DC      | GPIO for DC (Data/Command)     |       22        |
|    GND     |            -                   |  ESP32 GND Pin  |
|    VCC     |            -                   |  ESP32 3v3 Pin  |
|    LED     |            -                   |  ESP32 3v3 Pin  |
|    Reset   |        GPIO for Reset          |       12        |

Select `Component Config->LVGL TFT Display controller->Select a display controller model` and select `ILI9341` from the options

## Touch controller XPT2046 pin connections with ESP32

To configure XPT2046 as a touch controller, select `Component Config->LVGL Touch controller->Select Touchpanel controller model` and select `XPT2056` from options. 
Since both LCD controller and touch controller are connected on SPI bus, the values of MISO, MOSI, and CLK is same, only CS (Slave Select) is connected to ddifferent GPIO pin. To configure 
touch controller, go to `menucinfig` and select `Component Config->LVGL Touch controller->Touchpanel (XPT2056) Pin Assignments`

|   Pin Name   | Menuconfig      | GPIO Pin Number |
|--------------|-----------------|-----------------|
| T_DO  (MISO) |  GPIO for MISO  |       34        |
| T_DIN (MOSI) |  GPIO for MOSI  |       14        |
|    T_CS      |  GPIO for CS    |       32        |
|    T_CLK     |  GPIO for CLK   |       23        |

LVGL touch drives also needs to know about the values/data provided by XPT2046 pin when the surface of LCD is touched at min/max X coordinate (lower left and upper right corner) and min/max Y coordinate (lower right ad upper left corner).
For this go to `Component Config->LVGL Touch controller->Touchpanel Configuration` 

|           Menuconfig          | values |
|-------------------------------|--------|
|  Minimum X coordinate value   |  183   | 
|  Minimum Y coordinate value   |  134   |
|  Maximum X coordinate value   |  1959  |
|  Maximum Y coordinate value   |  1843  |

The below values should be constant as X/Y values require inversion (y means marked in menuconfig GUI)

| Menuconfig Variable Name   | values |
|----------------------------|--------|
| Invert X coordinate value  |   y    |
| Invert X coordinate value  |   y    |

LCD resolution 320x240 canbe configured in `Component Config->LVGL configuration` 

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




