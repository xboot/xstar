# 音效系统 (audioeffect)

可插拔的音效过滤器链。

## 音效结构体

```c
struct audio_effect_t {
    struct list_head_t list;
    const char * name;
    void * (*create)(void);
    void (*setup)(void * ctx, struct json_value_t * v);
    void (*prepare)(void * ctx, struct audio_frame_t * input);
    struct audio_frame_t * (*process)(void * ctx, struct audio_frame_t * input);
    void (*destroy)(void * ctx);
};
```

## 过滤器链

```c
struct audio_filter_t {
    struct audio_effect_node_t * nodes;
    int nlen;
};
```

## API

| 函数 | 说明 |
|------|------|
| `register_audio_effect(ae)` | 注册音效 |
| `unregister_audio_effect(ae)` | 注销音效 |
| `audio_filter_alloc(json, len)` | 从 JSON 配置创建过滤器链 |
| `audio_filter_free(filter)` | 释放过滤器链 |
| `audio_filter_bypass(filter, id, bypass)` | 设置绕过状态 |
| `audio_filter_setup(filter, id, json, len)` | 重新配置音效参数 |
| `audio_filter_process(filter, input)` | 处理音频帧 |

## 可用音效

| 音效 | 说明 |
|------|------|
| `"volume"` | 音量/增益调节 |
| `"compressor"` | 动态范围压缩器（软膝盖） |
| `"limiter"` | 峰值限幅器（输出保护） |
| `"panning"` | 立体声平衡 |
| `"resample"` | 采样率转换 |
| `"crystalizer"` | 音频增强 |
| `"iir"` | IIR 无限脉冲响应滤波器 |
| `"reshape"` | 声道/样本重排 |
| `"duplicate"` | 复制音频帧到独立 buffer（隔离后续原地修改） |
| `"tremolo"` | 颤音效果（振幅调制） |
| `"mono"` | 单声道下混 |

每个音效的 JSON 配置语法、参数取值与算法说明详见 [音效插件详解](./effect-plugins)。

## JSON 配置示例

```json
{
    "resample": { "rate": 48000 },
    "reshape":  { "channel": 2 },
    "volume":   { "factor": 0.7 }
}
```

JSON 顶层为对象，**键名为音效名**，**值为该音效的参数对象**。处理顺序按 JSON 中出现顺序串行执行。

> 📌 **未知键名会被静默跳过**：链中出现**未注册或拼写错误**的音效名（例如 `"limitter"`、`"echo"`）会被 `audio_filter_alloc()` 忽略，不会报错也不会中断后续音效的解析。已识别的音效仍按 JSON 中的原始顺序串接；若整个 JSON 中**没有任何**已注册的音效名，`audio_filter_alloc()` 返回 `NULL`。这意味着拼写错误不会引发崩溃，但也不会有任何提示，请仔细核对音效名称。

## 关键机制

### shash 字符串散列
所有 JSON 字段名解析时使用 `shash()` 计算哈希值，通过 `switch-case` 跳转代替运行时字符串比较，性能与可读性兼得。新增音效时需通过 `developments/shash/shash` 工具或代码计算字段名的哈希值。

### 5 个生命周期钩子
- `create` 分配私有数据；`destroy` 释放
- `setup` 解析 JSON 参数（可被多次调用）
- `prepare` 每次处理前预分配输出缓冲区
- `process` 核心算法，处理一帧数据

### bypass 应用场景
`audio_filter_bypass()` 可临时跳过某个节点而保留其状态。典型用法：当输入与目标参数已经匹配时跳过 resample/reshape，减少 CPU 占用。例如 [`sink-playback.c:40`](https://github.com/anomalyco/xstar) 中根据输入帧的 rate/channel 动态切换 bypass。

## 说明

过滤器链从 JSON 配置创建，支持运行时动态调整参数和绕过指定节点。Source 和 Sink 都内置 filter 支持，通过 `audio_source_filter_apply()` 和 `audio_sink_filter_apply()` 应用。若想扩展自定义音效，请参考 [扩展指南](./extending)。
