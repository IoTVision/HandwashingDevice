#include <stdio.h>
#include "main.h"
#define RELAY    GPIO_NUM_12
#define LCD_BACKLIGHT GPIO_NUM_27
#define ULTRASONIC_TRIGGER GPIO_NUM_26
#define ULTRASONIC_ECHO GPIO_NUM_25
ClassLCDI2C lcdI2C;
extern "C"{
    void app_main(void);
}
const char* TAG = "main";
TaskHandle_t tUltrasonic;

void init()
{
    gpio_config_t io_cfg = {
        .pin_bit_mask = 1ULL << RELAY | 1ULL << LCD_BACKLIGHT | 1ULL << ULTRASONIC_TRIGGER,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_cfg);
}

esp_err_t i2c_init()
{
    i2c_dev_t i2cdev ;
    i2cdev.cfg.scl_io_num = (gpio_num_t)I2C_MASTER_SCL_IO;
    i2cdev.cfg.sda_io_num = (gpio_num_t)I2C_MASTER_SDA_IO;
    i2cdev.cfg.sda_pullup_en = GPIO_PULLUP_ENABLE;
    i2cdev.cfg.scl_pullup_en = GPIO_PULLUP_ENABLE;
    i2cdev.cfg.mode = I2C_MODE_MASTER;
    i2cdev.cfg.master.clk_speed = 100000;
    i2cdev.addr = 0x27;
    i2cdev.port = I2C_NUM_0;

    ESP_RETURN_ON_ERROR(i2cdev_init(),"I2C dev","Init Error");
    ESP_RETURN_ON_ERROR(lcdI2C.begin(&i2cdev),"LCD","begin error this address");
    ESP_RETURN_ON_ERROR(lcdI2C.testPCF(),"I2C dev","Address device not found");
    return ESP_OK;
}

static void Task_Ultrasonic(void *Parameter)
{
    HC_SR04_init();
    uint32_t distance;
    for(;;){
        distance = ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
        // ESP_LOGI("HC_SR04","distance %lu",(uint32_t)(distance/2));
        if(distance/2 < 100000) printf("%lu \n",(uint32_t)(distance/2));
    }
}

void app_main(void)
{
    // gpio_set_level(LCD_BACKLIGHT,1);
    init();
    i2c_init();
    pwm_init();
    
    lcdI2C.print("SpiritBoi",0,0);
    xTaskCreate(Task_Ultrasonic,"TaskUltrasonic",2048,NULL,2,&tUltrasonic);
    while(1){
        gen_trig_output();
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
}
