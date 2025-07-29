#include <Arduino.h>
#include "ui.h"
#include "event.h"

static int32_t touch_start_x;
static int32_t touch_start_y;

int i = 0;

// 声明一个静态变量来存储上一次点击的时间
static uint32_t last_click_time = 0;
// 设置双击的时间阈值（单位为毫秒）
#define DOUBLE_CLICK_TIME_THRESHOLD 100

void anim_cb1(void *obj, int32_t v)
{
    // 更新文本对象的位置
    lv_obj_set_x(obj, v);
}

void anim_cb2(void *obj, int32_t v)
{
    // 更新文本对象的位置
    lv_obj_set_y(obj, v);
}

void anim_end_callback()
{

    ui_Screen1_screen_init();
}

void btn1_event_cb(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        if (slider_update_timer)
        {
            lv_timer_del(slider_update_timer); // 删除定时器1
            slider_update_timer = NULL;        // 清空指针以避免悬空指针
        }
        pinmap_init();
    }
}

void btn2_event_cb(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_CLICKED)
    {   
        lv_obj_clean(lv_scr_act());
        if (slider_update_timer)
        {
            lv_timer_del(slider_update_timer); // 删除定时器1
            slider_update_timer = NULL;        // 清空指针以避免悬空指针
        }
        power_init();
    }
}

void btn3_event_cb(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (slider_update_timer)
    {
        lv_timer_del(slider_update_timer); // 删除定时器1
        slider_update_timer = NULL;        // 清空指针以避免悬空指针
    }
    if (event_code == LV_EVENT_CLICKED)
    {
        pwm_init();
    }
}

void btn4_event_cb(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (slider_update_timer)
    {
        lv_timer_del(slider_update_timer); // 删除定时器1
        slider_update_timer = NULL;        // 清空指针以避免悬空指针
    }
    if (event_code == LV_EVENT_CLICKED)
    {
        uarthelper_init();
    }
}

void btn5_event_cb(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        if (slider_update_timer)
        {
            lv_timer_del(slider_update_timer); // 删除定时器1
            slider_update_timer = NULL;        // 清空指针以避免悬空指针
        }
        i2c_init();
    }
}

void btn6_event_cb(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        if (slider_update_timer)
        {
            lv_timer_del(slider_update_timer); // 删除定时器1
            slider_update_timer = NULL;        // 清空指针以避免悬空指针
        }
        voltmeter_init();
    }
}

void btn7_event_cb(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        if (slider_update_timer)
        {
            lv_timer_del(slider_update_timer); // 删除定时器1
            slider_update_timer = NULL;        // 清空指针以避免悬空指针
        }
        DSO_init();
    }
}

void btn8_event_cb(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        if (slider_update_timer)
        {
            lv_timer_del(slider_update_timer); // 删除定时器1
            slider_update_timer = NULL;        // 清空指针以避免悬空指针
        }
        wirelessuart_init();
    }
}

void btn9_event_cb(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        if (slider_update_timer)
        {
            lv_timer_del(slider_update_timer); // 删除定时器1
            slider_update_timer = NULL;        // 清空指针以避免悬空指针
        }
        FREcount_init();
    }
}

void btn10_event_cb(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        if (slider_update_timer)
        {
            lv_timer_del(slider_update_timer); // 删除定时器1
            slider_update_timer = NULL;        // 清空指针以避免悬空指针
        }
        readme_init();
    }
}

void poweronbtn_event_cb(lv_event_t *e)
{

    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        i++;
        if (i % 2 == 1)
        {
            digitalWrite(1, HIGH);
            lv_obj_set_style_text_color(poweron_label, lv_color_hex(0x00FF7F), 0);
        }
        else
        {
            digitalWrite(1, LOW);
            lv_obj_set_style_text_color(poweron_label, lv_color_hex(0xFF0000), 0);
        }
    }
}

void pwm_btn_event_cb(lv_event_t *e)
{

    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        i++;
        if (i % 2 == 1)
        {
            pwm_flag = 1;
            lv_obj_set_style_bg_color(pwm_btn, lv_color_hex(0x00FF7F), 0);
        }
        else
        {
            pwm_flag = 0;
            lv_obj_set_style_bg_color(pwm_btn, lv_color_hex(0xFF0000), 0);
        }
    }
}

void fre_event_cb(lv_event_t * e) {
    lv_obj_t * fre = lv_event_get_target(e);
    const char * text = lv_textarea_get_text(fre);

    // 检查输入的值是否为数字
    int value = atoi(text);
    if (value < 0 || value > 100000) {
        // 如果超出范围，重置为最大/最小值
        if (value < 0) {
            lv_textarea_set_text(fre, "0");
        } else if (value > 100000) {
            lv_textarea_set_text(fre, "100000");
        }
    }
}

void duty_event_cb(lv_event_t * e) {
    lv_obj_t * duty = lv_event_get_target(e);
    const char * text = lv_textarea_get_text(duty);

    // 检查输入的值是否为数字
    int value = atoi(text);
    if (value < 0 || value > 100) {
        // 如果超出范围，重置为最大/最小值
        if (value < 0) {
            lv_textarea_set_text(duty, "0");
        } else if (value > 100) {
            lv_textarea_set_text(duty, "100");
        }
    }
}

void i2conbtn_event_cb(lv_event_t *e)
{

    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        i++;
        if (i % 2 == 1)
        {
            i2cscan_flag = 1;
            lv_obj_set_style_bg_color(i2con, lv_color_hex(0x00FF7F), 0);
        }
        else
        {
            i2cscan_flag = 0;
            lv_obj_set_style_bg_color(i2con, lv_color_hex(0xFF0000), 0);
        }
    }
}

void uart_btn_event_cb(lv_event_t *e)
{

    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        i++;
        if (i % 2 == 1)
        {
            uart_helper_flag = 1;
            lv_obj_set_style_bg_color(uart_btn, lv_color_hex(0x00FF7F), 0);
        }
        else
        {
            uart_helper_flag = 0;
            lv_obj_set_style_bg_color(uart_btn, lv_color_hex(0xFF0000), 0);
        }
    }
}

void wireless_uart_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        i++;
        if (i % 2 == 1)
        {
            BluetoothSerial_flag = 1;
            lv_obj_set_style_bg_color(wireless_uart_btn, lv_color_hex(0x00FF7F), 0);
        }
        else
        {
            BluetoothSerial_flag = 0;
            lv_obj_set_style_bg_color(wireless_uart_btn, lv_color_hex(0xFF0000), 0);
        }
    }
}

void VUPbtn_event_cb(lv_event_t *e)
{

    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        mcp4107_flag = 4;
    }
}

void VDOWNbtn_event_cb(lv_event_t *e)
{

    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        mcp4107_flag = 5;
    }
}

void V11btn_event_cb(lv_event_t *e)
{

    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        mcp4107_flag = 1;
    }
}

void V5btn_event_cb(lv_event_t *e)
{

    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        mcp4107_flag = 2;
    }
}

void V3btn_event_cb(lv_event_t *e)
{

    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        mcp4107_flag = 3;
    }
}

void keyboard_event_handler(lv_event_t *e)
{

    lv_event_code_t code = lv_event_get_code(e); /* 获取事件类型 */
    lv_obj_t *target = lv_event_get_target(e);   /* 获取触发源 */
}

void fretext_event_handler(lv_event_t *e)
{

    // 显示键盘
    lv_obj_clear_flag(fre_keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(duty_keyboard, LV_OBJ_FLAG_HIDDEN);
}

void dutytext_event_handler(lv_event_t *e)
{

    // 显示键盘
    lv_obj_clear_flag(duty_keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(fre_keyboard, LV_OBJ_FLAG_HIDDEN);
}

void clear_keyboard_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e); /* 获取事件类型 */
    lv_obj_t *target = lv_event_get_target(e);   /* 获取触发源 */
    lv_obj_t *textarea = lv_keyboard_get_textarea(target);
    if (code == LV_EVENT_VALUE_CHANGED)
    {
        /* 获取键盘按钮索引 */
        uint16_t id = lv_btnmatrix_get_selected_btn(target);
        /* 获取按钮文本 */
        const char *txt = lv_btnmatrix_get_btn_text(target, id);
        if (strcmp(txt, ".") == 0) 
        {
            lv_textarea_del_char(textarea);
        } 
        if (strcmp(txt, LV_SYMBOL_KEYBOARD) == 0) /* 判断是不是键盘图标被按下 */
        {
            lv_obj_add_flag(fre_keyboard, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(duty_keyboard, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

// 滑动条的事件回调
void slider_event_cb(lv_event_t *e)
{
    int value = lv_slider_get_value(slider); // 获取滑动条的值
    switch (11 - value)
    {
    case 1:
        lv_group_focus_obj(btn1);
        break;
    case 2:
        lv_group_focus_obj(btn2);
        break;
    case 3:
        lv_group_focus_obj(btn3);
        break;
    case 4:
        lv_group_focus_obj(btn4);
        break;
    case 5:
        lv_group_focus_obj(btn5);
        break;
    case 6:
        lv_group_focus_obj(btn6);
        break;
    case 7:
        lv_group_focus_obj(btn7);
        break;
    case 8:
        lv_group_focus_obj(btn8);
        break;
    case 9:
        lv_group_focus_obj(btn9);
        break;
    case 10:
        lv_group_focus_obj(btn10);
        break;
    default:
        break;
    }
}

void event_handler_back(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_indev_t *indev = lv_indev_get_act(); // 获取当前激活的输入设备
    if (indev)
    {
        lv_point_t touch_point;
        lv_indev_get_point(indev, &touch_point); // 获取触摸点
        if (code == LV_EVENT_PRESSED)
        {
            touch_start_x = touch_point.x;
        }
        else if (code == LV_EVENT_RELEASED)
        {
            int32_t delta_x = touch_point.x - touch_start_x; // 检测向左滑动
            if (abs(delta_x) > SWIPE_THRESHOLD && delta_x > 0)
            {
                ina266_flag = 0;
                mcp4107_flag = 0;
                pwm_flag = 0;
                i2cscan_flag = 0;
                uart_helper_flag = 0;
                DSO_flag = 0;
                BluetoothSerial_flag = 0;
                FREcount_flag = 0;
                digitalWrite(1, LOW);
                if (updatelabel_timer1)
                {
                    lv_timer_del(updatelabel_timer1); // 删除定时器1
                    updatelabel_timer1 = NULL;        // 清空指针以避免悬空指针
                }
                if (updatelabel_timer2)
                {
                    lv_timer_del(updatelabel_timer2); // 删除定时器1
                    updatelabel_timer2 = NULL;        // 清空指针以避免悬空指针
                }
                if (updatelabel_timer3)
                {
                    lv_timer_del(updatelabel_timer3); // 删除定时器1
                    updatelabel_timer3 = NULL;        // 清空指针以避免悬空指针
                }
                if (adddata_timer)
                {
                    lv_timer_del(adddata_timer); // 删除定时器2
                    adddata_timer = NULL;        // 清空指针以避免悬空指针
                }
                if (adddata_timer2)
                {
                    lv_timer_del(adddata_timer2); // 删除定时器2
                    adddata_timer2 = NULL;        // 清空指针以避免悬空指针
                }

                if (DSO_update_timer1)
                {
                    lv_timer_del(DSO_update_timer1); // 删除定时器2
                    DSO_update_timer1 = NULL;        // 清空指针以避免悬空指针
                }

                if (DSO_update_timer2)
                {
                    lv_timer_del(DSO_update_timer2); // 删除定时器2
                    DSO_update_timer2 = NULL;        // 清空指针以避免悬空指针
                }

                if (DSO_update_timer3)
                {
                    lv_timer_del(DSO_update_timer3); // 删除定时器2
                    DSO_update_timer3 = NULL;        // 清空指针以避免悬空指针
                }
                if (slider_update_timer)
                {
                    lv_timer_del(slider_update_timer); // 删除定时器1
                    slider_update_timer = NULL;        // 清空指针以避免悬空指针
                }
                if (FRE_label_update_timer)
                {
                    lv_timer_del(FRE_label_update_timer); // 删除定时器1
                    FRE_label_update_timer = NULL;        // 清空指针以避免悬空指针
                }
                ui_Screen1_screen_init();
            }
        }
    }
}

void longpress_event_handler_back(lv_event_t *e)
{
    ina266_flag = 0;
    mcp4107_flag = 0;
    pwm_flag = 0;
    i2cscan_flag = 0;
    uart_helper_flag = 0;
    DSO_flag = 0;
    BluetoothSerial_flag = 0;
    FREcount_flag = 0;
    digitalWrite(1, LOW);
    if (updatelabel_timer1)
    {
        lv_timer_del(updatelabel_timer1); // 删除定时器1
        updatelabel_timer1 = NULL;        // 清空指针以避免悬空指针
    }
    if (updatelabel_timer2)
    {
        lv_timer_del(updatelabel_timer2); // 删除定时器1
        updatelabel_timer2 = NULL;        // 清空指针以避免悬空指针
    }
    if (updatelabel_timer3)
    {
        lv_timer_del(updatelabel_timer3); // 删除定时器1
        updatelabel_timer3 = NULL;        // 清空指针以避免悬空指针
    }
    if (adddata_timer)
    {
        lv_timer_del(adddata_timer); // 删除定时器2
        adddata_timer = NULL;        // 清空指针以避免悬空指针
    }
    if (adddata_timer2)
    {
        lv_timer_del(adddata_timer2); // 删除定时器2
        adddata_timer2 = NULL;        // 清空指针以避免悬空指针
    }

    if (DSO_update_timer1)
    {
        lv_timer_del(DSO_update_timer1); // 删除定时器2
        DSO_update_timer1 = NULL;        // 清空指针以避免悬空指针
    }
    if (DSO_update_timer2)
    {
        lv_timer_del(DSO_update_timer2); // 删除定时器2
        DSO_update_timer2 = NULL;        // 清空指针以避免悬空指针
    }
    if (DSO_update_timer3)
    {
        lv_timer_del(DSO_update_timer3); // 删除定时器2
        DSO_update_timer3 = NULL;        // 清空指针以避免悬空指针
    }
    if (slider_update_timer)
    {
        lv_timer_del(slider_update_timer); // 删除定时器1
        slider_update_timer = NULL;        // 清空指针以避免悬空指针
    }
    if (FRE_label_update_timer)
    {
        lv_timer_del(FRE_label_update_timer); // 删除定时器1
        FRE_label_update_timer = NULL;        // 清空指针以避免悬空指针
    }
    ui_Screen1_screen_init();
}
