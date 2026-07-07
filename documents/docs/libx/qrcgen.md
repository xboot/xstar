# 二维码生成 (qrcgen)

QR Code 生成器，支持版本 1-40、四种纠错等级、八种掩码模式。

## 类型

```c
enum qrcgen_ecc_t {
	QRCGEN_ECC_LOW,
	QRCGEN_ECC_MEDIUM,
	QRCGEN_ECC_QUARTILE,
	QRCGEN_ECC_HIGH
};

enum qrcgen_mask_t {
	QRCGEN_MASK_AUTO,
	QRCGEN_MASK_0,
	QRCGEN_MASK_1,
	QRCGEN_MASK_2,
	QRCGEN_MASK_3,
	QRCGEN_MASK_4,
	QRCGEN_MASK_5,
	QRCGEN_MASK_6,
	QRCGEN_MASK_7
};
```

## API

```c
int qrcgen_encode_text(const char * txt, uint8_t * tmp, uint8_t * qrc, enum qrcgen_ecc_t ecc, int minv, int maxv, enum qrcgen_mask_t mask, int boost);
int qrcgen_encode_binary(uint8_t * buf, int len, uint8_t * qrc, enum qrcgen_ecc_t ecc, int minv, int maxv, enum qrcgen_mask_t mask, int boost);
int qrcgen_get_size(uint8_t * qrc);
int qrcgen_get_pixel(uint8_t * qrc, int x, int y);
char * qrcgen_tostring(const char * txt, int invert);
```

- `qrcgen_encode_text` — 编码文本为 QR Code
- `qrcgen_encode_binary` — 编码二进制数据
- `qrcgen_get_size` — 获取 QR Code 边长（模块数）
- `qrcgen_get_pixel` — 获取指定位置像素（1=黑, 0=白）
- `qrcgen_tostring` — 便捷接口：编码文本并返回字符串表示

## 使用示例

```c
uint8_t tmp[QRCGEN_BUFFER_LEN_MAX];
uint8_t qrc[QRCGEN_BUFFER_LEN_MAX];

if(qrcgen_encode_text("Powered by xstar", tmp, qrc, QRCGEN_ECC_MEDIUM, 1, 40, QRCGEN_MASK_AUTO, 1))
{
	int size = qrcgen_get_size(qrc);
	for(int y = 0; y < size; y++)
	{
		for(int x = 0; x < size; x++)
			xos_printf("%c", qrcgen_get_pixel(qrc, x, y) ? '#' : ' ');
		xos_printf("\n");
	}
}

char * str = qrcgen_tostring("Powered by xstar", 0);
if(str)
{
	xos_printf("%s\n", str);
	xos_mem_free(str);
}
```
