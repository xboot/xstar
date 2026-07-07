# 扩展指南 (extending)

本文介绍如何为 XSTAR 音频框架编写自定义组件，包括 **三种扩展方式**的对比以及 **effect / source / sink** 的完整实现模板。

## 三种扩展方式对比

| 方式 | 适用场景 | 是否注册 | 复杂度 |
|------|---------|---------|--------|
| 回调式 source | 快速集成外部音频数据 | 否，`audio_source_alloc_custom()` 即可 | ★☆☆ |
| 自定义 effect | 新的信号处理算法：滤波/特效/分析 | 是，`register_audio_effect()` | ★★★ |
| 自定义 source/sink | 新文件格式/新硬件接口/新分析工具 | 否，实现 `xxx_pdata_t` + 钩子函数 | ★★☆ |

---

## 方式一：自定义 effect 插件（最常用）

### 模板代码

```c
#include <kernel/audio/effect.h>

/* 私有数据结构 */
struct myeffect_pdata_t {
    float param1;
    int   param2;
    /* 状态变量（如果需要） */
};

static void * myeffect_create(void)
{
    struct myeffect_pdata_t * pdat;
    pdat = xos_mem_malloc(sizeof(struct myeffect_pdata_t));
    if(!pdat)
        return NULL;
    pdat->param1 = 1.0f;
    pdat->param2 = 0;
    return pdat;
}

static void myeffect_setup(void * ctx, struct json_value_t * v)
{
    struct myeffect_pdata_t * pdat = (struct myeffect_pdata_t *)ctx;
    if(v && (v->type == JSON_OBJECT))
    {
        for(int i = 0; i < v->u.object.length; i++)
        {
            struct json_value_t * o = v->u.object.values[i].value;
            switch(shash(v->u.object.values[i].name))
            {
            case 0x01234567: /* "param1" — 用 shash("param1") 替换 */
                if(o && (o->type == JSON_DOUBLE))
                    pdat->param1 = (float)o->u.dbl;
                break;
            /* 更多参数... */
            }
        }
    }
}

static void myeffect_prepare(void * ctx, struct audio_frame_t * input)
{
    /* 可选：根据 input->rate/channel/frames 重新分配缓冲区 */
}

static struct audio_frame_t * myeffect_process(void * ctx, struct audio_frame_t * input)
{
    struct myeffect_pdata_t * pdat = (struct myeffect_pdata_t *)ctx;
    float * pi = input->samples;

    for(int n = 0; n < input->frames; n++)
    {
        for(int c = 0; c < input->channel; c++)
        {
            pi[c] = pi[c] * pdat->param1;   /* 示例：幅度缩放 */
        }
        pi += input->channel;
    }
    return input;  /* 原地处理或返回新 buffer */
}

static void myeffect_destroy(void * ctx)
{
    struct myeffect_pdata_t * pdat = (struct myeffect_pdata_t *)ctx;
    if(pdat)
        xos_mem_free(pdat);
}

static struct audio_effect_t myeffect = {
    .name    = "myeffect",
    .create  = myeffect_create,
    .setup   = myeffect_setup,
    .prepare = myeffect_prepare,
    .process = myeffect_process,
    .destroy = myeffect_destroy,
};

static void myeffect_init(void)   { register_audio_effect(&myeffect); }
static void myeffect_exit(void)   { unregister_audio_effect(&myeffect); }
core_initcall(myeffect_init);
core_exitcall(myeffect_exit);
```

### 5 个钩子说明

| 钩子 | 时机 | 必选 | 职责 |
|------|------|------|------|
| `create` | 分配 filter 时 | 是 | 分配私有数据结构并初始化默认值 |
| `setup` | 每次 JSON 配置/重配置 | 是 | 解析 JSON 更新参数（可能被多次调用） |
| `prepare` | 每次 `process` 之前 | 是 | 根据入参格式预分配输出缓冲区 |
| `process` | 每帧处理 | 是 | 核心算法：处理输入帧 → 返回输出帧 |
| `destroy` | 释放 filter 时 | 是 | 释放私有数据 |

**两个重要约定**：
1. `setup` 可能被外部通过 `audio_filter_setup()` 多次调用以修改参数
2. `prepare` 与 `process` 成对出现——`prepare` 做资源预检/分配，`process` 做实际计算

### 关于 shash 哈希值的获取

在源码目录执行以下命令获取字段名的 shash 值：

```bash
# 用现成的 shash 工具（位于 developments/shash/）
echo -n "myparam" | ../developments/shash/shash
```

或临时编写 C 代码：
```c
#include <stdio.h>
#include "libx/string-hash.h"
int main(void) { printf("0x%08x\n", shash("myparam")); return 0; }
```

---

## 方式二：自定义 source

### 文件型 source 模板

```c
#include <kernel/audio/source.h>

struct audio_source_myfmt_pdata_t {
    int    rate;
    int    channel;
    int    pos;
    /* 文件句柄、解码器状态等 */

    struct audio_frame_t output;
    float * samples;
    int    nsample;
};

static int audio_source_myfmt_seek(struct audio_source_t * s, int offset)
{
    struct audio_source_myfmt_pdata_t * pdat = s->priv;
    /* 实现 seek 逻辑 */
    return pdat->pos;
}

static int audio_source_myfmt_tell(struct audio_source_t * s)
{
    struct audio_source_myfmt_pdata_t * pdat = s->priv;
    return pdat->pos;
}

static int audio_source_myfmt_length(struct audio_source_t * s)
{
    struct audio_source_myfmt_pdata_t * pdat = s->priv;
    return /* 总长度（帧数）*/;
}

static struct audio_frame_t * audio_source_myfmt_read(struct audio_source_t * s)
{
    struct audio_source_myfmt_pdata_t * pdat = s->priv;
    int frames = pdat->rate * pdat->channel / 200;  /* ~5ms */

    /* 解码 pdat->rate * pdat->channel / 200 个样本到 pdat->samples */

    pdat->output.rate = pdat->rate;
    pdat->output.channel = pdat->channel;
    pdat->output.frames = frames;
    pdat->output.samples = pdat->samples;
    return &pdat->output;
}

static int audio_source_myfmt_ioctl(struct audio_source_t * s, const char * cmd, void * arg)
{
    /* 支持 get/set-volume 或其他自定义命令 */
    return -1;
}

static void audio_source_myfmt_destroy(struct audio_source_t * s)
{
    struct audio_source_myfmt_pdata_t * pdat = s->priv;
    if(pdat)
    {
        if(pdat->samples)
            xos_mem_free(pdat->samples);
        xos_mem_free(pdat);
    }
}

/* 工厂函数 */
struct audio_source_t * audio_source_alloc_from_myfmt(const char * path)
{
    struct audio_source_myfmt_pdata_t * pdat;
    struct audio_source_t * s;

    pdat = xos_mem_malloc(sizeof(struct audio_source_myfmt_pdata_t));
    if(!pdat)
        return NULL;

    /* 初始化 */
    pdat->rate = 48000;
    pdat->channel = 2;
    pdat->pos = 0;
    pdat->nsample = 0;
    pdat->samples = NULL;

    s = audio_source_alloc();
    if(!s)
    {
        xos_mem_free(pdat);
        return NULL;
    }

    s->priv    = pdat;
    s->seek    = audio_source_myfmt_seek;
    s->tell    = audio_source_myfmt_tell;
    s->length  = audio_source_myfmt_length;
    s->read    = audio_source_myfmt_read;
    s->ioctl   = audio_source_myfmt_ioctl;
    s->destroy = audio_source_myfmt_destroy;

    return s;
}
```

### 快速回调方式（无需注册）

如果只需要一个简单的数据源，使用 `audio_source_alloc_custom()` 即可：

```c
int my_audio_callback(float * samples, int nsample, void * data)
{
    /* 填充 samples[0..nsample-1] */
    /* 返回 -1 停止，>= 0 继续 */
    return 0;
}

struct audio_source_t * src = audio_source_alloc_custom(48000, 2, my_audio_callback, NULL);
```

---

## 方式三：自定义 sink

```c
#include <kernel/audio/sink.h>

struct audio_sink_myfmt_pdata_t {
    /* 自定义状态 */
};

static void audio_sink_myfmt_write(struct audio_sink_t * s, struct audio_frame_t * af)
{
    struct audio_sink_myfmt_pdata_t * pdat = s->priv;
    /* 处理 af->samples[0..af->frames*af->channel-1] */
    /* 例如写入文件、送入分析器、传给硬件等 */
}

static int audio_sink_myfmt_ioctl(struct audio_sink_t * s, const char * cmd, void * arg)
{
    switch(shash(cmd))
    {
    case 0x10cbc7b7: /* "audio-sink-set-volume" */
        if(arg) {
            int * p = arg;
            /* 设置音量 */
            return 0;
        }
        break;
    case 0xe04cfa2b: /* "audio-sink-get-volume" */
        if(arg) {
            int * p = arg;
            p[0] = /* 当前音量 */;
            return 0;
        }
        break;
    }
    return -1;
}

static void audio_sink_myfmt_destroy(struct audio_sink_t * s)
{
    struct audio_sink_myfmt_pdata_t * pdat = s->priv;
    if(pdat)
        xos_mem_free(pdat);
}

struct audio_sink_t * audio_sink_alloc_myfmt(void)
{
    struct audio_sink_myfmt_pdata_t * pdat;
    struct audio_sink_t * s;

    pdat = xos_mem_malloc(sizeof(struct audio_sink_myfmt_pdata_t));
    if(!pdat)
        return NULL;

    s = audio_sink_alloc();
    if(!s)
    {
        xos_mem_free(pdat);
        return NULL;
    }

    s->priv    = pdat;
    s->write   = audio_sink_myfmt_write;
    s->ioctl   = audio_sink_myfmt_ioctl;
    s->destroy = audio_sink_myfmt_destroy;

    return s;
}
```

---

## 常见陷阱与最佳实践

### 内存管理
- `create` 中分配的资源必须在 `destroy` 中释放
- `prepare` 中如果预分配 buffer，应在 `destroy` 中释放
- 若 `process` 需要输出新的帧（与输入不同格式），在 `prepare` 中分配、在 `destroy` 中释放

### 性能建议
- 每个 `process` 调用处理约 5ms 数据（~200 采样/帧@48kHz），避免过重的每样本开销
- 使用 `shash` 替代字符串比较（已在框架中统一实现）
- 尽量减少 `prepare` 中的内存分配次数（只在 nsample 增大时 realloc）

### initcall 时机
- effect 插件用 `core_initcall()` 注册
- 如果依赖系统的某些组件（如 XFS），使用 `driver_initcall()` 或 `postcore_initcall()`
- 参考 [AGENTS.md](../guide/architecture-design) 的 initcall 级别说明

### 标准 ioctl 命令
sink 应支持的标准命令（source 同理）：

| 命令 | shash 值 | 用途 |
|------|----------|------|
| `"audio-sink-get-volume"` | `0xe04cfa2b` | 获取音量 [0, 1000] |
| `"audio-sink-set-volume"` | `0x10cbc7b7` | 设置音量 [0, 1000] |

### JSON 配置解析
- 始终检查 `v->type == JSON_OBJECT` 后再遍历
- 每个字段名先检查类型再赋值
- 默认值已在 `create` 中设定，`setup` 只需覆盖

---

## 更多资源

- 源码中现有的 effect 实现是**最好的参考**：`effect-iir.c`（复杂状态）、`effect-volume.c`（简单处理）、`effect-resample.c`（输出格式变化）
- [典型使用示例](./usage-examples) 展示各组件如何组合
- [音效插件详解](./effect-plugins) 列出所有内建 effect 的 JSON 配置