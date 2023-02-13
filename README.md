# Máy rửa tay

## Tổng quan

Máy rửa tay phục vụ cho học tập STEM, sử dụng cảm biến siêu âm HC-SR04 để phát hiện thay đổi khoảng cách, điều khiển relay để đóng ngắt máy bơm và có màn hình LCD 16x2 để hiển thị
![Alt text](Image/HandwashingDevice.jpg)

## Hướng dẫn sử dụng project

### Yêu cầu phần cứng

Phần cứng sử dụng kit ESP32-WROOM-32E tự build của IoTVisionLab, 1 cảm biến siêu âm 4 pin kết nối với header Grove 2.0mm, một màn hình LCD1602 được điều khiển nhờ IC mở rộng chân PCF8574, ngoài ra để nạp code và log dữ liệu lên máy tính cần sử dụng cáp type-C

### Yêu cầu thư viện

Thư viện sử dụng cho project là CodeFirmwareIoTVision có thể tải [ở đây](https://github.com/IoTVision/CodeFirmwareIoTVision.git) dùng lệnh sau:

```bash
git submodule add <đường link thư viện> <components>
```

#### Bảng kết nối phần cứng:

**Ghi chú**: 
- Có thể gán lại các chân sử dụng thông qua ``menuconfig`` 
- Vì chân I2C không kết nối điện trở kéo lên trên board nên cần cấu hình trở kéo nội
 
| GPIO|   Chức năng     |
|-----|-----------------|
|  27 | Đèn nền LCD     |
|  22 | SCL             |
|  21 | SDA             |
|  25 | HC_SR04 ECHO    |
|  26 | HC_SR04 TRIGGER |

#### Thứ tự chân LCD kết nối với PCF8574:

(**Lưu ý**: không có chân PCF8574 nối với đèn nền nên không có trong bảng)

| PCF8574   | LCD 16x2  |
| --------  | ----------|
|  0        | RS        |
|  2        | E         |
|  4        | D4        |
|  5        | D5        |
|  6        | D6        |
|  7        | D7        |

## Code

- Đổi tên main.c thành main.cpp, trong CMakeLists.txt thuộc thư mục main (không phải CMakeList chung của project) đổi main.c thành main.cpp như sau:

```
idf_component_register(SRCS "main.cpp"
                    INCLUDE_DIRS ".")
```
### Include thư viện:

```bash
esp_log.h
i2cdev.h
driver/gpio.h
LCD_I2C.h
PCF8574.h
HC_SR04.h
pwm.h
```
### Define các chân:

```bash
define I2C_MASTER_SCL_IO 22
define I2C_MASTER_SDA_IO 21
define RELAY    GPIO_NUM_12
define LCD_BACKLIGHT GPIO_NUM_27
define ULTRASONIC_TRIGGER GPIO_NUM_26
define ULTRASONIC_ECHO GPIO_NUM_25
```
*extern C*: nếu sử dụng main.cpp thì khai báo extern C như sau:

```bash
extern "C"{
    void app_main(void);
}
```

### Khởi tạo LCD
*ClassLCDI2C lcdI2C;*

### Khởi tạo GPIO

```bash
void GPIO_init()
{
    gpio_config_t io_cfg = {
        .pin_bit_mask =   1ULL << RELAY 
                        | 1ULL << LCD_BACKLIGHT 
                        | 1ULL << ULTRASONIC_TRIGGER,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_cfg);
}
```

- Giải thích code

    - Khởi tạo một biến io_cfg có kiểu gpio_config_t dùng để chứa thông tin cấu hình chân GPIO(nhấn F12 trên vscode vào gpio_config_t để xem thêm):

        - pin_bit_mask: cho phép chân GPIO có số thứ tự tương ứng hoạt động bằng cách dịch bit 1 (ép kiểu uint64_t format ULL unsigned long long) vào vị trí chân đó.

        - mode: cấu hình chức năng GPIO (gõ theo dạng GPIO_MODE_XXX để vscode gợi ý).

        - pull_up_en,pull_down_en: cho phép kéo trở lên mức cao và mức thấp (gõ theo dạng GPIO_PULLUP và GPIO_PULLDOWN để vscode gợi ý).

        - intr_type: cấu hình ngắt GPIO.

    - Sau khi đưa các thông tin cần thiết vào io_cfg thì dùng hàm gpio_config(); đưa địa chỉ của io_cfg vào để cấu hình phần cứng

### Khởi tạo I2C

```bash
esp_err_t i2c_init()
{
    i2cdev_init();

    i2c_dev_t i2cdev ;
    i2cdev.cfg.scl_io_num = (gpio_num_t)I2C_MASTER_SCL_IO;
    i2cdev.cfg.sda_io_num = (gpio_num_t)I2C_MASTER_SDA_IO;
    i2cdev.cfg.sda_pullup_en = GPIO_PULLUP_ENABLE;
    i2cdev.cfg.scl_pullup_en = GPIO_PULLUP_ENABLE;
    i2cdev.cfg.mode = I2C_MODE_MASTER;
    i2cdev.cfg.master.clk_speed = 100000;
    i2cdev.addr = 0x27;
    i2cdev.port = I2C_NUM_0;
    
    lcdI2C.begin(&i2cdev);
    return ESP_OK;
}
```

- Giải thích code
    - Khởi tạo i2cdev bằng hàm i2cdev_init()
    - Khởi tạo một struct i2cdev từ kiểu dữ liệu i2c_dev_t đưa cho phương thức begin của class lcdI2C để cấu hình cho PCF8574

### Task HC_SR04 Ultrasonic đọc thời gian ECHO trả về

```bash
static void Task_Ultrasonic(void *Parameter)
{
    HC_SR04_init();
    uint32_t echoTime;
    for(;;){
        echoTime = ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
        if(echoTime/2 < 100000) printf("%lu \n",(uint32_t)(echoTime/2));
    }
}
```

- Giải thích code
    - Thực hiện khởi tạo hàm HC_SR04_init();
    - Vì thư viện HC_SR04 sử dụng phương pháp Notify trong RTOS để gửi  giá trị thời gian đo được trong hàm ngắt về Task_Ultrasonic nên ta sử dụng hàm ulTaskNotifyTake(pdTRUE,portMAX_DELAY) để lấy giá trị đo được gán vào echoTime, đối số đầu tiên là xóa dữ liệu nhận được sau khi gán giá trị, đối số thứ 2 là chờ không giới hạn timeout để chương trình chờ tại hàm không chạy tiếp cho đến khi hàm ngắt gửi giá trị mới đến 

### Hàm tạo xung TRIGGER
```bash
void gen_trig_output(void)
{
    gpio_set_level(HC_SR04_TRIGGER_GPIO, 1); // set high
    esp_rom_delay_us(10);
    gpio_set_level(HC_SR04_TRIGGER_GPIO, 0); // set low
}
```
- Giải thích code
    Xuất 1 xung 10us tại chân TRIGGER

### app_main

```bash
void app_main(void)
{
    GPIO_init();
    i2c_init();
    
    lcdI2C.print("Test",0,0);
    xTaskCreate(Task_Ultrasonic,"TaskUltrasonic",2048,NULL,2,&tUltrasonic);
    while(1){
        gen_trig_output();
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
}
```

- Giải thích code
    - Gọi các hàm khởi tạo ở trên
    - sử dụng phương thức print của lcdI2C để in ra màn hình, đối số đầu là chuỗi truyền vào, 2 đối số còn lại là vị trí in x và y
    - Khởi tạo task "Task_Ultrasonic"
    - Trong vòng while(1), thực hiện phát xung TRIGGER cho cảm biến siêu âm mỗi 100ms
