# ESP32_LCD

ESP32 project to display a GUI on ILI9341 320x240 LCD with touch. GUI consist of the clock and stopwatch. 
Example project can be found [here](https://github.com/lvgl/lv_port_esp32)

# Used packages
- [LVGL](https://github.com/lvgl/lvgl)
- [LVGL Drivers](https://github.com/lvgl/lvgl_esp32_drivers)
- [esp-idf](https://github.com/espressif/esp-idf)

In order to use LVGL and LVGL driver to display GUI with widgets, specific version of them( git commit hash) must be used.


# Project setup image, Configuration

The following values are set in sdkconfig file using menuconfig in esp-idf (_idf.py menuconfig_)

## Touchpanel (XPT2046) Pin Assignments to ESP32 GPIO number

CONFIG_LV_TOUCH_SPI_MISO=12

CONFIG_LV_TOUCH_SPI_MOSI=14

CONFIG_LV_TOUCH_SPI_CLK=26

CONFIG_LV_TOUCH_SPI_CS=27

CONFIG_LV_TOUCH_PIN_IRQ=13

## Touchpanel Configuration (XPT2046)

The values of X min/max and Y min/max indicate the raw values from XPT2046 controller when a respective corner of the LCD screen is touched. 
The below values determined when the LCD is used in landscape mode. Depending on thee mode, the values should be swapped.

CONFIG_LV_TOUCH_X_MIN=183 (constant)

CONFIG_LV_TOUCH_Y_MIN=134 (constant)

CONFIG_LV_TOUCH_X_MAX=1959 (constant)

CONFIG_LV_TOUCH_Y_MAX=1843 (constant)

CONFIG_LV_TOUCH_INVERT_X=y

CONFIG_LV_TOUCH_INVERT_Y=y

CONFIG_LV_TOUCH_DETECT_IRQ=y

CONFIG_LV_TOUCH_SPI_MISO=12

CONFIG_LV_TOUCH_SPI_MOSI=14

CONFIG_LV_TOUCH_SPI_CLK=26

CONFIG_LV_TOUCH_SPI_CS=27

CONFIG_LV_TOUCH_PIN_IRQ=13

<img src="https://github.com/ishansheth/ESP32_LCD/blob/master/images/PXL_20220716_171200853.MP.jpg" alt="drawing" width="400" height="400"/>

