# go

跳转到指定地址执行程序。

## 用法

```
go address [args ...]
```

## 说明

- 以 `int func(int argc, char ** argv)` 方式调用指定内存地址
- 剩余参数将传递给被调用函数
- 用于加载和运行 RAM 中的裸机程序

## 示例

```bash
go 0x80200000 arg1 arg2
```
