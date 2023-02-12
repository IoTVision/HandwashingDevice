#ifndef _MAIN_H_
#define _MAIN_H_

#define I2C_MASTER_SCL_IO           CONFIG_I2C_MASTER_SCL      /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           CONFIG_I2C_MASTER_SDA      /*!< GPIO number used for I2C master data  */

#include "esp_log.h"
#include "i2cdev.h"
#include "PCF8574.h"
#include "LCD_I2C.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "HC_SR04.h"
#include "esp_check.h"
#include "pwm.h"
#endif