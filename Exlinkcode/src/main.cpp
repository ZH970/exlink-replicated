#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <lvgl.h>
#include <Wire.h>
#include "CST816T.h"
#include <WiFi.h>
#include "ui.h"
#include "INA.h"
#include "stdio.h"
#include "driver/ledc.h"
#include "driver/pcnt.h"
#include "soc/pcnt_struct.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <esp_task_wdt.h>

#define I2C_SDA 18 // 触摸SDA引脚
#define I2C_SCL 21 // 触摸SCL引脚
#define RST_N_PIN 16
#define INT_N_PIN -1
#define RIGHT_PIN 38
#define LEFT_PIN 39
#define PUSH_PIN 40
#define PCNT_UNIT PCNT_UNIT_0                                         // 使用PCNT单元0
#define LONG_PRESS_THRESHOLD 300                                      // 定义长按时间阈值
#define SHORT_PRESS_THRESHOLD 10                                      // 定义短按时间阈值
#define SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"           // 服务UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E" // 接收特征UUID
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E" // 发送特征UUID

int adcValues[SAMPLE_COUNT];                                   // 存储ADC采样数值
char maxValueStr[20], minValueStr[20], peakToPeakValueStr[20]; // 存最大最小峰峰值

volatile int pulseCount = 0;
float frequency = 0.0;
char freqencyStr[20];
int16_t count = 0;

unsigned long previousMillis = 0; // 记录上次读取计数的时间

const long interval = 125; // 设置脉冲读取间隔为125毫秒

INA_Class INA;
int8_t request_index = 0;
char voltageStr[20], currentStr[20], powerStr[20], mAHStr[20]; // 存储ina226测量数据
uint64_t vol, cur, wat;
double v, a, w;
volatile uint8_t deviceNumber = UINT8_MAX;
volatile uint64_t sumBusMillVolts = 0;
volatile int64_t sumBusMicroAmps = 0;
volatile uint8_t readings = 0;
volatile int64_t mAH = 0;

int ina266_flag = 0;
int mcp4107_flag = 0;
int pwm_flag = 0;
int i2cscan_flag = 0;
int uart_helper_flag = 0;
int DSO_flag = 0;
int BluetoothSerial_flag = 0;
int FREcount_flag = 0;

int pwm_Freq = 0;
int pwm_Duty = 0;

int new_pwm_Freq = 0;
int new_pwm_Duty = 0;

int mcp4107_value = 0x7E;
long currentBaudRate = 115200; // 定义初始波特率
lv_indev_t *indev_keypad;
byte error, address;
int nDevices;
// 在这里设置屏幕尺寸
static const uint32_t screenWidth = 320;
static const uint32_t screenHeight = 240;
// lvgl显示存储数组
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * screenHeight / 2];
CST816T cst816t(I2C_SDA, I2C_SCL, RST_N_PIN, INT_N_PIN);
hw_timer_t *tim1 = NULL;
TFT_eSPI tft = TFT_eSPI();

BLEServer *pServer = NULL;            // BLEServer指针pServer
BLECharacteristic *pTxCharacteristic; // BLECharacteristic指针 pTxCharacteristic
BLECharacteristic *pRxCharacteristic; // BLECharacteristic指针 pRxCharacteristic
bool deviceConnected = false;         // 本次连接状态
bool oldDeviceConnected = false;      // 上次连接状态
uint8_t txValue = 0;

class MyServerCallbacks : public BLEServerCallbacks // BLE连接回调函数
{
  void onConnect(BLEServer *pServer)
  {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer *pServer)
  {
    deviceConnected = false;
  }
};

class MyCallbacks : public BLECharacteristicCallbacks // BLE接收回调函数
{
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    std::string rxValue = pCharacteristic->getValue();

    if (rxValue.length() > 0)
    {

      //for (int i = 0; i < rxValue.length(); i++)
      //{
      Serial.print(rxValue.c_str());
      //}
      Serial.print("\n");
    }
  }
};

void mcp4017_init() // 数字电位器初始化
{
  Wire.beginTransmission(0x2F);
  Wire.write(mcp4107_value);
  Wire.endTransmission();
}

void mcp4017_task() // 数字电位器任务
{
  if (mcp4107_flag == 1) //9V
  {
    Wire.beginTransmission(0x2F);
    mcp4107_value = 0x12;
    Wire.write(mcp4107_value);
    Wire.endTransmission();
  }
  if (mcp4107_flag == 2) //5V
  {
    Wire.beginTransmission(0x2F);
    mcp4107_value = 0x27;
    Wire.write(mcp4107_value);
    Wire.endTransmission();
  }
  if (mcp4107_flag == 3) //3.3V
  {
    Wire.beginTransmission(0x2F);
    mcp4107_value = 0x48;
    Wire.write(mcp4107_value);
    Wire.endTransmission();
  }

  if (mcp4107_flag == 4)
  {
    mcp4107_value--;
    if (mcp4107_value < 0x10)
      mcp4107_value = 0x10;
    Wire.beginTransmission(0x2F);
    Wire.write(mcp4107_value);
    Wire.endTransmission();
    mcp4107_flag = 0;
  }

  if (mcp4107_flag == 5)
  {
    mcp4107_value++;
    if (mcp4107_value > 0x7E)
      mcp4107_value = 0x7E;
    Wire.beginTransmission(0x2F);
    Wire.write(mcp4107_value);
    Wire.endTransmission();
    mcp4107_flag = 0;
  }
}

void buzzer_task() // 蜂鸣器任务
{
  ledcWrite(1, 3);
  delay(100);
  ledcWrite(1, 0);
}

void pwm_task() // pwm输出任务
{

  if (pwm_flag == 1)
  {

    const char *freq_str = lv_textarea_get_text(fre);
    const char *duty_str = lv_textarea_get_text(duty);
    pwm_Freq = atoi(freq_str);
    pwm_Duty = constrain(atoi(duty_str), 0, 100); // 限制占空比在 0-100 之间
    pwm_Duty = map(pwm_Duty, 0, 100, 0, 255);

    if (pwm_Freq != new_pwm_Freq | pwm_Duty != new_pwm_Duty)
    {
      ledcAttachPin(5, 2);       // 将引脚连接到通道2
      ledcSetup(2, pwm_Freq, 8); // 设置通道2为8位分辨率
      ledcWrite(2, pwm_Duty);    // 通道2输出
      new_pwm_Freq = pwm_Freq;
      new_pwm_Duty = pwm_Duty;
    }
  }

  if (pwm_flag == 0)
  {
    new_pwm_Freq = 0;
    new_pwm_Duty = 0;
    ledcWrite(2, 0);
  }
}

void i2cscan_task() // I2C扫描任务
{
  if (i2cscan_flag == 1)
  {

    Wire.begin(6, 7);
    lv_textarea_add_text(i2c_extarea, "Scanning...\n");

    nDevices = 0;
    for (address = 1; address < 127; address++)
    {
      Wire1.begin(6, 7);
      Wire1.beginTransmission(address); // 从指定的地址开始向I2C从设备进行传输
      error = Wire1.endTransmission();  // 停止与从机的数据传输
      if (error == 0)
      {
        char i2c_address[20];
        lv_textarea_add_text(i2c_extarea, "I2C device address 0x");
        if (address < 16)
          lv_textarea_add_text(i2c_extarea, "0");
        sprintf(i2c_address, "%X\n", address);
        lv_textarea_add_text(i2c_extarea, i2c_address);

        nDevices++;
      }
      else if (error == 4)
      {
        char i2c_address[20];
        lv_textarea_add_text(i2c_extarea, "Unknown error at address 0x");
        if (address < 16)
          lv_textarea_add_text(i2c_extarea, "0");
        sprintf(i2c_address, "%X\n", address);
        lv_textarea_add_text(i2c_extarea, i2c_address);
      }
    }
    if (nDevices == 0)
      lv_textarea_add_text(i2c_extarea, "No I2C devices found\n");
    else
      lv_textarea_add_text(i2c_extarea, "done\n");

    i2cscan_flag = 0;
  }
}

void uart_helper_task() // 串口助手任务
{

  if (uart_helper_flag == 1)
  {

    char buf[10];
    lv_dropdown_get_selected_str(uart_list, buf, sizeof(buf));
    long newBaudRate = 0;

    if (strcmp(buf, "1200") == 0)
    {
      newBaudRate = 1200;
    }
    else if (strcmp(buf, "2400") == 0)
    {
      newBaudRate = 2400;
    }
    else if (strcmp(buf, "4800") == 0)
    {
      newBaudRate = 4800;
    }
    else if (strcmp(buf, "9600") == 0)
    {
      newBaudRate = 9600;
    }
    else if (strcmp(buf, "19200") == 0)
    {
      newBaudRate = 19200;
    }
    else if (strcmp(buf, "43000") == 0)
    {
      newBaudRate = 43000;
    }
    else if (strcmp(buf, "76800") == 0)
    {
      newBaudRate = 76800;
    }
    else if (strcmp(buf, "115200") == 0)
    {
      newBaudRate = 115200;
    }
    else if (strcmp(buf, "128000") == 0)
    {
      newBaudRate = 128000;
    }
    else if (strcmp(buf, "230400") == 0)
    {
      newBaudRate = 230400;
    }
    else if (strcmp(buf, "256000") == 0)
    {
      newBaudRate = 256000;
    }
    else if (strcmp(buf, "460800") == 0)
    {
      newBaudRate = 460800;
    }
    else if (strcmp(buf, "921600") == 0)
    {
      newBaudRate = 921600;
    }
    if (newBaudRate != currentBaudRate)
    {
      Serial.end();                  // 结束当前串口
      Serial.begin(newBaudRate);     // 以新波特率开始串口
      currentBaudRate = newBaudRate; // 更新当前波特率
    }
    if (Serial.available() > 0)
    {
      String input = Serial.readStringUntil('\n');  // 当为\n或者超时 才会换行
      lv_textarea_add_text(uart_extarea, input.c_str());
      lv_textarea_add_text(uart_extarea, "\n");
    }
  }
}

void DSO_task() // 简易示波器任务
{

  if (DSO_flag == 1)
  {
    for (int i = 0; i < SAMPLE_COUNT; i++)
    {
      adcValues[i] = analogRead(4); // Read ADC values
      lv_chart_set_next_value(DSO_chart, DSO_ser, adcValues[i]);
    }

    float maxValue;
    float minValue = 4095;
    float peakToPeakValue;

    for (int i = 0; i < SAMPLE_COUNT; i++)
    {
      if (adcValues[i] > maxValue)
      {
        maxValue = adcValues[i];
      }
      if (adcValues[i] < minValue)
      {
        minValue = adcValues[i];
      }
    }
    maxValue = maxValue / 4095 * 3.3;
    minValue = minValue / 4095 * 3.3;

    peakToPeakValue = maxValue - minValue; // 计算峰峰值

    sprintf(maxValueStr, "%0.2f", maxValue);
    sprintf(minValueStr, "%0.2f", minValue);
    sprintf(peakToPeakValueStr, "%0.2f", peakToPeakValue);
  }
}

void keypad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data) // 按键扫描函数（可识别长按短按）
{
  static uint32_t last_key = 0;
  static uint32_t push_start_time = 0; // 记录按下的时间
  static bool push_pressed = false;    // 记录PUSH_PIN是否按下
  uint8_t key_state = 0;
  uint32_t act_key;

  // 读取按键状态
  if (digitalRead(RIGHT_PIN) == LOW)
  {
    key_state = 1; // 按键 1
  }
  else if (digitalRead(LEFT_PIN) == LOW)
  {
    key_state = 2; // 按键 2
  }
  else if (digitalRead(PUSH_PIN) == LOW)
  {
    if (!push_pressed)
    {
      push_pressed = true;        // 标记PUSH_PIN按下
      push_start_time = millis(); // 记录按下时间
    }
    key_state = 3; // 按键 3
  }
  else
  {
    key_state = 0;        // 没有按键按下
    push_pressed = false; // 重置PUSH_PIN状态
  }

  if (key_state != 0)
  {
    data->state = LV_INDEV_STATE_PR;

    switch (key_state)
    {
    case 1:
      act_key = LV_KEY_NEXT; // 短按 RIGHT_PIN
      last_key = act_key;
      break;
    case 2:
      act_key = LV_KEY_PREV; // 短按 LEFT_PIN
      last_key = act_key;
      break;
    case 3: // 检测PUSH_PIN的长按和短按
      if (push_pressed)
      {
        if (millis() - push_start_time >= LONG_PRESS_THRESHOLD)
        {
          lv_event_send(lv_scr_act(), LV_EVENT_LONG_PRESSED, NULL);
          last_key = act_key;
        }
        if (push_pressed && (millis() - push_start_time < LONG_PRESS_THRESHOLD))
        {
          act_key = LV_KEY_ENTER; // 短按处理
          last_key = act_key;
        }
      }
      break;
    default:
      break;
    }
    buzzer_task(); // 按下后触发蜂鸣器
  }
  else
  {
    data->state = LV_INDEV_STATE_REL;
  }

  data->key = last_key; // 更新状态
}

void ina266_task() // 功率监测任务
{

  if (ina266_flag == 1)
  {
    vol = INA.getBusMilliVolts(deviceNumber);
    cur = INA.getBusMicroAmps(deviceNumber) / 5000;
    wat = INA.getBusMicroWatts(deviceNumber);

    if (cur > 20 * 1000)
    {
      cur = 0;
    }

    v = vol / 1000.0;
    a = cur / 1000.0;
    w = v * a;

    if (v > 10)
    {
      sprintf(voltageStr, "%0.2f", v);
    }
    else
    {
      sprintf(voltageStr, "%0.3f", v);
    }

    if (a > 10)
    {
      sprintf(currentStr, "%0.2f", a);
    }
    else
    {
      sprintf(currentStr, "%0.3f", a);
    }

    if (w > 10)
    {
      sprintf(powerStr, "%0.2f", w);
    }
    else
    {
      sprintf(powerStr, "%0.3f", w);
    }

    mAH += cur;
    sprintf(mAHStr, "%0.3f", mAH / (60 * 60) / 1000.0);
  }
}

void BluetoothSerial_task() // 无线串口任务（基于BLE实现）
{
  if (BluetoothSerial_flag == 1)
  {
    BLEDevice::init("Exlink"); // 创建一个BLE设备

    pServer = BLEDevice::createServer();            // 创建一个BLE服务
    pServer->setCallbacks(new MyServerCallbacks()); // 设置回调

    BLEService *pService = pServer->createService(SERVICE_UUID); // 创建蓝牙服务器

    // 创建发送特征，添加描述符，设置通知权限
    pTxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);
    pTxCharacteristic->addDescriptor(new BLE2902());
    // 创建接收特征，设置回调函数，设置可写权限
    pRxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE);
    pRxCharacteristic->setCallbacks(new MyCallbacks());
    pService->start();                  // 启动服务
    pServer->getAdvertising()->start(); // 开始广播
    BluetoothSerial_flag = 2;
  }
  if (BluetoothSerial_flag == 2)
  {
    char buf[10];
    lv_dropdown_get_selected_str(wireless_uart_list, buf, sizeof(buf));
    long newBaudRate = 0;

    if (strcmp(buf, "1200") == 0)
    {
      newBaudRate = 1200;
    }
    else if (strcmp(buf, "2400") == 0)
    {
      newBaudRate = 2400;
    }
    else if (strcmp(buf, "4800") == 0)
    {
      newBaudRate = 4800;
    }
    else if (strcmp(buf, "9600") == 0)
    {
      newBaudRate = 9600;
    }
    else if (strcmp(buf, "19200") == 0)
    {
      newBaudRate = 19200;
    }
    else if (strcmp(buf, "43000") == 0)
    {
      newBaudRate = 43000;
    }
    else if (strcmp(buf, "76800") == 0)
    {
      newBaudRate = 76800;
    }
    else if (strcmp(buf, "115200") == 0)
    {
      newBaudRate = 115200;
    }
    else if (strcmp(buf, "128000") == 0)
    {
      newBaudRate = 128000;
    }
    else if (strcmp(buf, "230400") == 0)
    {
      newBaudRate = 230400;
    }
    else if (strcmp(buf, "256000") == 0)
    {
      newBaudRate = 256000;
    }
    else if (strcmp(buf, "460800") == 0)
    {
      newBaudRate = 460800;
    }
    else if (strcmp(buf, "921600") == 0)
    {
      newBaudRate = 921600;
    }
    if (newBaudRate != currentBaudRate)
    {
      Serial.end();                  // 结束当前串口
      Serial.begin(newBaudRate);     // 以新波特率开始串口
      currentBaudRate = newBaudRate; // 更新当前波特率
    }

    if (deviceConnected) // 连接时执行串口转发
    {
      if (Serial.available() > 0)
      {
        String strValue = Serial.readStringUntil('\n');
        pTxCharacteristic->setValue((uint8_t *)strValue.c_str(), strValue.length());
        pTxCharacteristic->notify();
        lv_textarea_add_text(wireless_uart_extarea, strValue.c_str());
        lv_textarea_add_text(wireless_uart_extarea, "\n");
        delay(100);
      }
    }

    if (!deviceConnected && oldDeviceConnected) // 未连接时执行蓝牙广播
    {
      delay(500);
      pServer->startAdvertising();
      Serial.println("start advertising");
      oldDeviceConnected = deviceConnected;
    }
    if (deviceConnected && !oldDeviceConnected) // 更新蓝牙状态
    {
      oldDeviceConnected = deviceConnected;
    }
  }
}

void FREcount_task() // 数字频率计任务
{
  if (FREcount_flag == 1)
  {
    pcnt_config_t pcnt_config = {
        .pulse_gpio_num = 5,                // 输入引脚
        .ctrl_gpio_num = PCNT_PIN_NOT_USED, // 不使用控制引脚
        .lctrl_mode = PCNT_MODE_KEEP,       // 脉冲计数模式
        .hctrl_mode = PCNT_MODE_KEEP,       // 禁用高电平计数
        .pos_mode = PCNT_COUNT_INC,         // 正脉冲计数
        .neg_mode = PCNT_COUNT_DIS,         // 禁用负脉冲计数
        .unit = PCNT_UNIT,                  // 使用的PCNT单元
        .channel = PCNT_CHANNEL_0,
    };
    pcnt_unit_config(&pcnt_config); // 配置PCNT单元
    pcnt_counter_clear(PCNT_UNIT);  // 启动计数
    FREcount_flag = 2;
  }
  if (FREcount_flag == 2)
  {
    unsigned long currentMillis = millis();
    pcnt_get_counter_value(PCNT_UNIT, &count);
    unsigned long currentinterval = currentMillis - previousMillis;
    if (currentinterval >= interval) // 如果达到读取间隔
    {

      //  计算频率（假设每次读取的时间为1秒）
      //  unsigned long frequency = count/currentinterval*1000; // 频率 = 计数值（赫兹）
      sprintf(freqencyStr, "%d", count * 8); // 更新数据
      pcnt_counter_clear(PCNT_UNIT);         // 清零计数
      previousMillis = currentMillis;        // 更新上次读取计数的时间
    }
  }
}

// 屏幕打点函数
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{

  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors((uint16_t *)&color_p->full, w * h, true);
  tft.endWrite();

  lv_disp_flush_ready(disp);
}

// 触摸屏回调函数
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
  TouchInfos tp;
  tp = cst816t.GetTouchInfo();
  bool touched = tp.touching;
  if (!touched)
  {
    data->state = LV_INDEV_STATE_REL;
    // Serial.println("NO TOUCH");
  }
  if (touched)
  {
    data->state = LV_INDEV_STATE_PR;
    data->point.x = 320 - tp.y;
    data->point.y = tp.x;
    // Serial.println("TOUCHING");
  }
}

// 定时器中断服务函数
void tim1Interrupt()
{
  lv_tick_inc(1);
}

// lvgl任务处理函数
void lvgl_handler(void *pvParameters)
{
  while (1)
  {
    lv_timer_handler(); /* let the GUI do its work */
    delay(5);
  }
}

void setup()
{
  esp_task_wdt_delete(NULL);  // 删除当前任务的看门狗
  Serial.begin(115200);
  pinMode(1, OUTPUT);
  digitalWrite(1, LOW);
  pinMode(3, OUTPUT);
  ledcAttachPin(3, 1);
  ledcSetup(1, 1000, 8);
  ledcWrite(1, 0);
  pinMode(RIGHT_PIN, INPUT_PULLUP);
  pinMode(LEFT_PIN, INPUT_PULLUP);
  pinMode(PUSH_PIN, INPUT_PULLUP);
  analogReadResolution(12);            
  analogSetPinAttenuation(2, ADC_11db); 
  analogSetPinAttenuation(4, ADC_11db); 
  tft.init();
  tft.setRotation(3); // 设置显示方向 0
  cst816t.begin();    // 初始化触摸屏

  lv_init();
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * screenHeight / 2);
  static lv_disp_drv_t disp_drv; // 初始化显示器

  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  disp_drv.sw_rotate = 1;             // 屏幕镜像
  disp_drv.rotated = LV_DISP_ROT_180; // 屏幕旋转
  lv_disp_drv_register(&disp_drv);    // 注册显示屏

  static lv_indev_drv_t indev_drv1;
  lv_indev_drv_init(&indev_drv1);
  indev_drv1.type = LV_INDEV_TYPE_POINTER; // 设置为触摸屏类型
  indev_drv1.read_cb = my_touchpad_read;   // 注册回调函数
  lv_indev_drv_register(&indev_drv1);      // 注册输入设备

  static lv_indev_drv_t indev_drv2;
  lv_indev_drv_init(&indev_drv2);
  indev_drv2.read_cb = keypad_read;                  // 注册回调函数
  indev_drv2.type = LV_INDEV_TYPE_KEYPAD;            // 设置为按键类型
  indev_keypad = lv_indev_drv_register(&indev_drv2); // 注册输入设备

  // 启动定时器为lvgl提供心跳时钟
  tim1 = timerBegin(0, 80, true);
  timerAttachInterrupt(tim1, tim1Interrupt, true);
  timerAlarmWrite(tim1, 1000, true);
  timerAlarmEnable(tim1);

  ledcWrite(1, 3);
  Wire.begin(18, 21);
  uint8_t devicesFound = 0;
  while (deviceNumber == UINT8_MAX)
  {
    devicesFound = INA.begin(10, 10000);
    Serial.println(INA.getDeviceName(devicesFound - 1));
    for (uint8_t i = 0; i < devicesFound; i++)
    {

      if (strcmp(INA.getDeviceName(i), "INA226") == 0)
      {
        deviceNumber = i;
        INA.reset(deviceNumber);

        break;
      }
    }
    if (deviceNumber == UINT8_MAX)
    {
      Serial.print(F("No INA found. Waiting 5s and retrying...\n"));
      delay(5000);
    }
  }
  Serial.print(F("Found INA at device number "));
  Serial.println(deviceNumber);
  Serial.println();
  INA.setAveraging(4, deviceNumber);
  INA.setBusConversion(8244, deviceNumber);            // 最大转换时间8.244ms
  INA.setShuntConversion(8244, deviceNumber);          // 最大转换时间8.244ms
  INA.setMode(INA_MODE_CONTINUOUS_BOTH, deviceNumber); // 连续转换模式

  Serial.println("ui_init start");

  ui_init(); // 初始化ui界面
  Serial.println("ui_init done");
  Serial.println("Setup done");
  mcp4017_init();
  ledcWrite(1, 0);
}

void loop()
{
  // lvgl任务处理函数
  lv_task_handler();
  // 用户任务处理函数
  ina266_task();
  mcp4017_task();
  pwm_task();
  i2cscan_task();
  uart_helper_task();
  DSO_task();
  BluetoothSerial_task();
  FREcount_task();
}