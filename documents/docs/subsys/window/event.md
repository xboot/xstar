# 事件系统 (event)

窗口子系统的输入事件抽象层，统一管理按键、旋转编码器、鼠标、触摸、手柄和系统事件，通过 FIFO 队列将事件从输入设备传递到窗口。

## 事件类型

| 类型 | 值 | 说明 |
|------|------|------|
| `EVENT_TYPE_KEY_DOWN` | 0x0100 | 按键按下 |
| `EVENT_TYPE_KEY_UP` | 0x0101 | 按键释放 |
| `EVENT_TYPE_ROTARY_STEP` | 0x0200 | 旋转编码器步进 |
| `EVENT_TYPE_MOUSE_DOWN` | 0x0300 | 鼠标按键按下 |
| `EVENT_TYPE_MOUSE_MOVE` | 0x0301 | 鼠标移动 |
| `EVENT_TYPE_MOUSE_UP` | 0x0302 | 鼠标按键释放 |
| `EVENT_TYPE_MOUSE_WHEEL` | 0x0303 | 鼠标滚轮 |
| `EVENT_TYPE_TOUCH_BEGIN` | 0x0400 | 触摸开始 |
| `EVENT_TYPE_TOUCH_MOVE` | 0x0401 | 触摸移动 |
| `EVENT_TYPE_TOUCH_END` | 0x0402 | 触摸结束 |
| `EVENT_TYPE_JOYSTICK_LEFTSTICK` | 0x0500 | 手柄左摇杆 |
| `EVENT_TYPE_JOYSTICK_RIGHTSTICK` | 0x0501 | 手柄右摇杆 |
| `EVENT_TYPE_JOYSTICK_LEFTTRIGGER` | 0x0502 | 手柄左扳机 |
| `EVENT_TYPE_JOYSTICK_RIGHTTRIGGER` | 0x0503 | 手柄右扳机 |
| `EVENT_TYPE_JOYSTICK_BUTTONDOWN` | 0x0504 | 手柄按键按下 |
| `EVENT_TYPE_JOYSTICK_BUTTONUP` | 0x0505 | 手柄按键释放 |
| `EVENT_TYPE_SYSTEM_EXIT` | 0x1000 | 系统退出 |

## 数据结构

```c
struct event_t {
    void * device;              /* 事件来源设备 */
    enum event_type_t type;     /* 事件类型 */
    ktime_t timestamp;          /* 事件时间戳 */

    union {
        struct { uint32_t key; } key_down;          /* 按键按下 */
        struct { uint32_t key; } key_up;             /* 按键释放 */
        struct { int32_t delta; } rotary_step;       /* 旋转步进 */
        struct { int32_t x, y; uint32_t button; } mouse_down;   /* 鼠标按下 */
        struct { int32_t x, y; } mouse_move;         /* 鼠标移动 */
        struct { int32_t x, y; uint32_t button; } mouse_up;     /* 鼠标释放 */
        struct { int32_t dx, dy; } mouse_wheel;      /* 鼠标滚轮 */
        struct { int32_t x, y; uint32_t id; } touch_begin;  /* 触摸开始 */
        struct { int32_t x, y; uint32_t id; } touch_move;   /* 触摸移动 */
        struct { int32_t x, y; uint32_t id; } touch_end;    /* 触摸结束 */
        struct { int32_t x, y; } joystick_left_stick;   /* 左摇杆 */
        struct { int32_t x, y; } joystick_right_stick;  /* 右摇杆 */
        struct { int32_t v; } joystick_left_trigger;    /* 左扳机 */
        struct { int32_t v; } joystick_right_trigger;   /* 右扳机 */
        struct { uint32_t button; } joystick_button_down;  /* 手柄按下 */
        struct { uint32_t button; } joystick_button_up;    /* 手柄释放 */
    } e;
};
```

## 按键定义

按键码与 ASCII 编码兼容（32-127），同时扩展了功能键（1-31）和 Latin-1 补充字符（160-255）。常用功能键：

| 按键 | 值 | 说明 |
|------|------|------|
| `KB_KEY_POWER` | 1 | 电源键 |
| `KB_KEY_UP` | 2 | 方向键上 |
| `KB_KEY_DOWN` | 3 | 方向键下 |
| `KB_KEY_LEFT` | 4 | 方向键左 |
| `KB_KEY_RIGHT` | 5 | 方向键右 |
| `KB_KEY_ENTER` | 13 | 确认键 |
| `KB_KEY_BACK` | 11 | 返回键 |
| `KB_KEY_HOME` | 10 | Home 键 |
| `KB_KEY_MENU` | 12 | 菜单键 |
| `KB_KEY_DELETE` | 127 | 删除键 |

## 鼠标按键

| 常量 | 位 | 说明 |
|------|------|------|
| `MOUSE_BUTTON_LEFT` | bit 0 | 左键 |
| `MOUSE_BUTTON_RIGHT` | bit 1 | 右键 |
| `MOUSE_BUTTON_MIDDLE` | bit 2 | 中键 |
| `MOUSE_BUTTON_X1` | bit 3 | 侧键 1 |
| `MOUSE_BUTTON_X2` | bit 4 | 侧键 2 |

## 手柄按键

手柄按键采用位掩码，支持方向键、A/B/X/Y、Back/Start/Guide、肩键和摇杆按下：

| 常量 | 位 | 说明 |
|------|------|------|
| `JOYSTICK_BUTTON_A` | bit 4 | A 键 |
| `JOYSTICK_BUTTON_B` | bit 5 | B 键 |
| `JOYSTICK_BUTTON_X` | bit 6 | X 键 |
| `JOYSTICK_BUTTON_Y` | bit 7 | Y 键 |
| `JOYSTICK_BUTTON_START` | bit 9 | Start 键 |
| `JOYSTICK_BUTTON_GUIDE` | bit 10 | Guide 键 |

## 事件推送 API

输入设备驱动通过以下函数推送事件到窗口系统：

| 函数 | 说明 |
|------|------|
| `push_event_key_down(device, key)` | 推送按键按下事件 |
| `push_event_key_up(device, key)` | 推送按键释放事件 |
| `push_event_rotary_step(device, delta)` | 推送旋转编码器步进事件 |
| `push_event_mouse_button_down(device, x, y, button)` | 推送鼠标按下事件 |
| `push_event_mouse_button_up(device, x, y, button)` | 推送鼠标释放事件 |
| `push_event_mouse_move(device, x, y)` | 推送鼠标移动事件 |
| `push_event_mouse_wheel(device, dx, dy)` | 推送鼠标滚轮事件 |
| `push_event_touch_begin(device, x, y, id)` | 推送触摸开始事件 |
| `push_event_touch_move(device, x, y, id)` | 推送触摸移动事件 |
| `push_event_touch_end(device, x, y, id)` | 推送触摸结束事件 |
| `push_event_joystick_left_stick(device, x, y)` | 推送左摇杆事件 |
| `push_event_joystick_right_stick(device, x, y)` | 推送右摇杆事件 |
| `push_event_joystick_left_trigger(device, v)` | 推送左扳机事件 |
| `push_event_joystick_right_trigger(device, v)` | 推送右扳机事件 |
| `push_event_joystick_button_down(device, button)` | 推送手柄按键按下事件 |
| `push_event_joystick_button_up(device, button)` | 推送手柄按键释放事件 |

## 事件消费

事件通过 `window_pump_event()` 从窗口的事件 FIFO 队列中取出：

```c
struct event_t e;
while(window_pump_event(w, &e))
{
    switch(e.type)
    {
    case EVENT_TYPE_KEY_DOWN:
        LOG("key down: %d\n", e.e.key_down.key);
        break;
    case EVENT_TYPE_TOUCH_BEGIN:
        LOG("touch: (%d, %d) id=%u\n", e.e.touch_begin.x, e.e.touch_begin.y, e.e.touch_begin.id);
        break;
    default:
        break;
    }
}
```

## 说明

- 所有 `push_event_*` 函数在内部构造 `event_t` 结构体并调用 `push_event()` 注入窗口事件队列
- `device` 字段标识事件来源设备，可用于区分多个同类输入设备
- 事件时间戳通过 `ktime_get()` 自动填充
- 鼠标按键和手柄按键使用位掩码，支持同时按下多个键
- 触摸事件通过 `id` 区分多点触控的不同触点
- 摇杆坐标和扳机值为原始 ADC 值，使用前需进行校准和归一化
