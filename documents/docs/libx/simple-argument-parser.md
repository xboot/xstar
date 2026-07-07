# 简单参数解析器 (sarg)

轻量级命令行参数解析器,零分配、无外部依赖。把命令行参数划分为**选项**与**位置参数**两类,提供存在性检测、取值、类型化解析、合法性校验与迭代遍历。

核心取舍:**取值只认 `=` 附着形式**(`--name=val`)。裸选项一律是布尔标志,**绝不吞掉**后续 token——因此 `--verbose pos1 pos2` 中 `pos1`/`pos2` 永远是位置参数。以"无二义"换"少便利",换来的解析规则极简且可预测。

## 命令行模型

sarg 把每个 argv token 归为三类之一:

| token 形式 | 归类 | 说明 |
|-----------|------|------|
| `-x`、`--name`、`--name=val` | 选项 | 以 `-` 开头,第二字符为 `-` 或字母 |
| `--` | 终止符 | 其后所有 token 强制按位置参数处理,自身不计入 |
| 其余 | 位置参数 | 包括负数 `-1`、单独的 `-` |

要点:

- **负数与单 `-` 是位置参数**:`-1`、`-2`、`-` 不被当作选项,可直接作位置参数使用。
- **`-` 后跟字母一律是选项**:`-file.txt`、`-abc` 都被视为单个选项(不拆分),需作位置参数时用 `--` 分隔,如 `-- -file.txt`。
- **`--` 之后不再解析选项**:让形似选项的值(如文件名 `-weird`)能作为位置参数传入。

## 取值与匹配

**选项值只能用 `=` 附着**:`=` 必须与选项名在同一 token 内。

| 写法 | 含义 |
|------|------|
| `--flag` | 裸选项(布尔标志),无值 |
| `--flag=val` | 取值,值为 `val` |
| `--flag=` | 取值,值为空串(与"未出现"不同) |
| `--eq=a=b` | 取值,值为 `a=b`(值可含 `=`) |
| `--flag bar` | **不取值**;`bar` 是位置参数 |

各 API 的 `name` 参数**不带 `=`**,传纯选项名即可(如 `"--port"`)。匹配按 `name` 长度做前缀比较,并要求紧随其后是 `\0`(裸形式)或 `=`(取值形式),因此 `--out` 不会误匹配 `--output=bar`,`--foo` 与 `--foobar` 互不干扰,也不做缩写补全。这一规则对 `sarg_has`、`sarg_get`、`sarg_valid` 三者一致。

## API

```c
struct sarg_t {
    int argc;
    char ** argv;
    struct {
        int index;     /* 迭代游标:下一个待扫描的 argv 下标 */
        int literal;   /* 迭代状态:是否已遇到 "--" */
    } iter;
};
```

`argc`/`argv` 直接引用调用方数组,不拷贝。`iter` 是位置参数迭代器的内部状态,调用方无需直接读写。API 按用途分五组。

### 初始化

```c
void sarg_init(struct sarg_t * sarg, int argc, char ** argv);
```

绑定 argc/argv 并复位迭代游标。

### 选项

```c
int sarg_has(struct sarg_t * sarg, const char * name);
const char * sarg_get(struct sarg_t * sarg, const char * name, const char * def);
```

- `sarg_has` — 选项是否出现。裸 `--name` 与 `--name=val` 均算出现;`--` 之后的 token 被忽略。返回 1/0。做纯 token 存在性匹配,不区分选项与位置参数。
- `sarg_get` — 取选项值(仅 `--name=val`),缺失时返回 `def`。重复出现取**最后一个**(last-wins,倒序扫描)。返回指向 argv 的指针,不可释放。对 `--flag=`(空串值)返回 `""`(非 def、非 NULL),刻意保留"用户写了 `=` 但留空"这条信息,调用方可据此区分「未出现」与「写了留空」。

> 布尔标志用 `sarg_has` 检测,带值选项用 `sarg_get` 取值——两者职责互补:`has` 认裸形式与取值形式,`get` 只认取值形式。

### 位置参数

```c
const char * sarg_at(struct sarg_t * sarg, int index);
void sarg_iter_reset(struct sarg_t * sarg);
const char * sarg_iter_next(struct sarg_t * sarg);
```

- `sarg_at` — 按下标(0 起)取位置参数,跳过所有选项 token。越界返回 NULL。每次从 argv 开头重扫,O(index)。
- `sarg_iter_reset` / `sarg_iter_next` — 顺序遍历位置参数,耗尽返回 NULL,单趟 O(n)。游标状态在 `sarg_t` 内共享,重开遍历需 `reset`;`has`/`get`/`at` 只读、不碰游标,可在循环内安全调用。

### 合法性校验

```c
int sarg_valid(struct sarg_t * sarg, const char ** opts, int min, int max);
```

校验 argv 是否符合命令的选项与位置参数约束,返回 1 合法、0 非法。`sarg` 为 NULL 时返回 0。

- **`opts`** — **NULL 终结**的合法选项名数组,匹配规则同 `sarg_has`(见上)。可为 NULL,等价于"无合法选项":出现任何选项即非法。常配合复合字面量内联:`(const char *[]){ "-f", "-i", NULL }`。
- **`min`/`max`** — **位置参数数量**上下界。`min` 为下界(负数视同 0),`max` 为上界(`-1` 表示不限)。扫描整条 argv(含 `--` 之后,`--` 本身不计)计数,超出 `[min, max]` 即非法。
- **不细分原因**:只回答"合不合法",不返回是哪一项违规。调用方失败时通常直接打印 usage 并退出。

```c
if(!sarg_valid(&sarg, (const char *[]){ "-f", "-i", NULL }, 0, 1))
{
    usage();
    return -1;
}
```

> sarg 本身对未知选项静默忽略(无 schema);`sarg_valid` 是补这个缺口的可选校验层,由调用方按需调用。

### 类型化取值

```c
int sarg_get_int(struct sarg_t * sarg, const char * name, int def);
unsigned int sarg_get_uint(struct sarg_t * sarg, const char * name, unsigned int def);
long sarg_get_long(struct sarg_t * sarg, const char * name, long def);
unsigned long sarg_get_ulong(struct sarg_t * sarg, const char * name, unsigned long def);
double sarg_get_double(struct sarg_t * sarg, const char * name, double def);
```

在 `sarg_get` 基础上把 `--name=val` 解析为目标类型,省去手写 `strtol`/`strtod`。共享约定:

| 项 | 行为 |
|----|------|
| 取值形式 | 仅 `--name=val`;裸 `--name` 返回 `def`(存在性用 `sarg_has`) |
| 整数进制 | 自动:`0x` 十六进制、`0` 八进制、否则十进制(`--n=010` 为八进制 8) |
| 整串校验 | 必须完整解析;空串、尾部残留(`12abc`)、非数字(`abc`)返回 `def` |
| 缺失/非法 | 一律返回 `def`,无错误通道 |
| 无符号负值 | `get_uint`/`get_ulong` 拒绝 `-` 开头,返回 `def` |
| double | 支持 `1.5`、`1e3`、负数;非法返回 `def` |

> **整型溢出不检测**(atoi 风格):溢出时静默截断或取饱和值(`get_int` 截断为 `int`、`get_long` 取 `LONG_MAX`/`LONG_MIN`、`get_uint`/`get_ulong` 取 `UINT_MAX`/`ULONG_MAX`),**不返回 `def`**。端口、ID 等需严格范围的参数,请用 `sarg_get` 取原串自行 `strtol` 并检查 `errno`/`endptr`/范围。

```c
int port          = sarg_get_int(&sarg, "--port", 8080);
unsigned int sz   = sarg_get_uint(&sarg, "--size", 4096);
long off          = sarg_get_long(&sarg, "--offset", 0);
unsigned long crc = sarg_get_ulong(&sarg, "--crc", 0);
double scale      = sarg_get_double(&sarg, "--scale", 1.0);
```

## 示例

```c
struct sarg_t sarg;
sarg_init(&sarg, argc, argv);

/* 校验:仅允许 -f/-i,位置参数至多 1 个 */
if(!sarg_valid(&sarg, (const char *[]){ "-f", "-i", NULL }, 0, 1))
{
    usage();
    return -1;
}

/* 布尔标志 */
if(sarg_has(&sarg, "--verbose"))
    printf("verbose mode\n");

/* 取值(必须用 = 形式) */
const char * host = sarg_get(&sarg, "--host", "127.0.0.1");
const char * port = sarg_get(&sarg, "--port", "8080");

/* 迭代所有位置参数 */
sarg_iter_reset(&sarg);
const char * arg;
while((arg = sarg_iter_next(&sarg)) != NULL)
    printf("pos: %s\n", arg);
```

命令行 `prog --verbose --host=0.0.0.0 --port=9000 file1.txt file2.txt`:`--verbose` 命中标志,`--host`/`--port` 取到值,`file1.txt`/`file2.txt` 为位置参数。

## 限制

- **不支持空格取值**:`--foo bar` 中 `bar` 是位置参数;写 `--foo=bar`。
- **不支持短选项紧贴取值**:`-fbar` 是名为 `-fbar` 的选项;短选项取值写 `-f=bar`。
- **`-` 后跟字母整体作选项**:`-abc` 不拆成 `-a -b -c`;这类值用 `--name=-file.txt` 或 `-- -file.txt`。
- **不支持缩写**:`--out` 不匹配 `--output`。
- **无错误回报**:未知选项静默忽略,无报错回调;需要时可先用 `sarg_valid` 校验。
- **返回值生命周期**:返回指针指向 argv,不可释放,随 argv 存活。
- **迭代器游标共享**:同一 `sarg_t` 不能并行跑两轮迭代,重开需 `sarg_iter_reset`。
