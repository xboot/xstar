#include <win/win.h>

char * win_file_cwd(void)
{
	static char path[4096] = { 0 };
	static char * cwd = NULL;

	if(!cwd)
	{
		if(GetModuleFileName(NULL, path, sizeof(path)) > 0)
		{
			char * last_slash = strrchr(path, '\\');
			if(last_slash)
			{
				*last_slash = '\0';
				if(SetCurrentDirectory(path))
					cwd = path;
			}
		}
	}
	return cwd ? cwd : "C:\\";
}

int win_file_open(const char * path, const char * mode)
{
	int flags = _O_RDONLY | _O_BINARY;
	int plus = 0;

	while(*mode)
	{
		switch(*mode++)
		{
		case 'r':
			flags = _O_RDONLY | _O_BINARY;
			break;
		case 'w':
			flags = _O_WRONLY | _O_CREAT | _O_TRUNC | _O_BINARY;
			break;
		case 'a':
			flags = _O_WRONLY | _O_CREAT | _O_APPEND | _O_BINARY;
			break;
		case '+':
			plus = 1;
			break;
		}
	}
	if(plus)
		flags = (flags & ~(_O_RDONLY | _O_WRONLY)) | _O_RDWR;

	return _open(path, flags, (_S_IREAD | _S_IWRITE));
}

int win_file_close(int fd)
{
	int ret = _close(fd);
	return ret < 0 ? 0 : 1;
}

int win_file_isdir(const char * path)
{
	struct stat st;
	int ret = 0;

	if((stat(path, &st) == 0) && S_ISDIR(st.st_mode))
		ret = 1;
	return ret;
}

int win_file_isfile(const char * path)
{
	struct stat st;
	int ret = 0;

	if((stat(path, &st) == 0) && S_ISREG(st.st_mode))
		ret = 1;
	return ret;
}

int win_file_mode(const char * path)
{
	struct stat st;

	if(stat(path, &st) == 0)
		return st.st_mode & 0777;
	return 0;
}

int win_file_mkdir(const char * path)
{
	int ret = 0;

	if(_mkdir(path) == 0)
		ret = 1;
	return ret;
}

int win_file_remove(const char * path)
{
	struct stat st;
	int ret = 0;

	if(stat(path, &st) == 0)
	{
		if(S_ISDIR(st.st_mode))
			ret = (_rmdir(path) == 0) ? 1 : 0;
		else if(S_ISREG(st.st_mode))
			ret = (_unlink(path) == 0) ? 1 : 0;
	}
	return ret;
}

int win_file_access(const char * path, const char * mode)
{
	struct stat st;

	if(stat(path, &st) == 0)
		return 1;
	return 0;
}

void win_file_walk(const char * path, void (*cb)(const char * dir, const char * name, void * data), const char * dir, void * data)
{
	WIN32_FIND_DATA findData;
	HANDLE hFind;
	char searchPath[MAX_PATH];

	snprintf(searchPath, sizeof(searchPath), "%s\\*", path);
	hFind = FindFirstFile(searchPath, &findData);
	if(hFind == INVALID_HANDLE_VALUE)
		return;
	do {
		if(strcmp(findData.cFileName, ".") == 0)
			continue;
		else if(strcmp(findData.cFileName, "..") == 0)
			continue;
		if(cb)
			cb(dir, findData.cFileName, data);
	} while(FindNextFile(hFind, &findData));
	FindClose(hFind);
}

ssize_t win_file_read(int fd, void * buf, size_t count)
{
	ssize_t ret = _read(fd, buf, count);
	return (ret > 0) ? ret: 0;
}

ssize_t win_file_write(int fd, const void * buf, size_t count)
{
	ssize_t ret = _write(fd, buf, count);
	return (ret > 0) ? ret: 0;
}

int64_t win_file_seek(int fd, int64_t offset)
{
	int64_t len = (int64_t)_lseeki64(fd, 0, SEEK_END);
	if(offset < 0)
		offset = 0;
	else if(offset > len)
		offset = len;
	return (int64_t)_lseeki64(fd, offset, SEEK_SET);
}

int64_t win_file_tell(int fd)
{
	return (int64_t)_lseeki64(fd, 0, SEEK_CUR);
}

int64_t win_file_length(int fd)
{
	__int64 off, ret;
	off = _lseeki64(fd, 0, SEEK_CUR);
	ret = _lseeki64(fd, 0, SEEK_END);
	_lseeki64(fd, off, SEEK_SET);
	return (ret > 0) ? (int64_t)ret: 0;
}

void win_file_sync(int fd)
{
	_commit(fd);
}
