# 路径操作 (path)

文件系统路径操作工具，判断绝对/相对路径，提取文件名、目录名和扩展名。

## API

```c
int is_absolute_path(const char * path);
int is_relative_path(const char * path);
char * path_basename(char * path);
char * path_dirname(char * path);
const char * path_fileext(const char * filename);
```

- `is_absolute_path` — 是否绝对路径（识别 `"/"` 开头和 Windows 盘符路径如 `"C:\"`）
- `is_relative_path` — 是否相对路径（非绝对路径即为相对路径）
- `path_basename` — 提取文件名（原地修改）
- `path_dirname` — 提取目录名（原地修改）
- `path_fileext` — 提取扩展名

## 使用示例

```c
printf("%d\n", is_absolute_path("/usr/local/bin"));  /* 1 */
printf("%d\n", is_absolute_path("C:\\Windows"));     /* 1 */
printf("%d\n", is_relative_path("src/main.c"));      /* 1 */

char buf1[] = "/usr/local/bin/test.txt";
printf("%s\n", path_basename(buf1));

char buf2[] = "/usr/local/bin/test.txt";
printf("%s\n", path_dirname(buf2));

printf("%s\n", path_fileext("archive.tar.gz"));
```
