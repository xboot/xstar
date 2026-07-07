# 字符串哈希 (shash)

基于 djb2 算法的字符串哈希函数，用于哈希表键计算和 switch 字符串匹配等场景。

## 算法

djb2 算法，由 Daniel J. Bernstein 设计：

```
hash = ((hash << 5) + hash) + c
```

等价于 `hash = hash * 33 + c`，初始值为 5381。实现简单、分布均匀、计算快速。

## API

```c
uint32_t shash(const char * s);
```

计算字符串的 32 位哈希值。传入 NULL 返回 5381（初始值）。函数为 `static inline`，零调用开销。

## 典型场景：switch 匹配字符串

C 语言不支持 switch 字符串，通过 `shash` 将字符串转为整数即可实现：

```c
switch(shash(name))
{
case 0x41f60f4b: /* "aliceblue" */
    c->r = 0xf0; c->g = 0xf8; c->b = 0xff;
    break;
case 0x0f294442: /* "black" */
    c->r = 0x00; c->g = 0x00; c->b = 0x00;
    break;
case 0x7c94a78d: /* "blue" */
    c->r = 0x00; c->g = 0x00; c->b = 0xff;
    break;
default:
    break;
}
```

此场景下无需担心哈希碰撞——如果两个字符串哈希值相同，`case` 标签重复会导致编译错误，问题在编译期即可发现。注释中标注原始字符串便于阅读。

### 生成 case 值

#### 使用 shash 命令行工具

`shash` 是随项目提供的命令行工具（源码位于 `developments/shash/`），可直接在终端计算字符串的哈希值，方便编写 `switch-case` 代码时快速查询：

```
$ shash aliceblue black blue
0x41f60f4b: /* "aliceblue" */
0x0f294442: /* "black" */
0x7c94a78d: /* "blue" */
```

输出格式与 `case` 标签一致，可直接复制使用。

#### 在代码中生成

```c
printf("case 0x%08x: /* \"%s\" */\n", shash("aliceblue"), "aliceblue");
/* 输出: case 0x41f60f4b: "aliceblue" */
```

## 使用示例

### 命令分发

```c
switch(shash(cmd))
{
case 0x106149d3: /* "start" */
    do_start();
    break;
case 0x7c9e1b4b: /* "stop" */
    do_stop();
    break;
}
```
