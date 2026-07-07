#ifndef __XSTAR_KERNEL_COMMAND_H__
#define __XSTAR_KERNEL_COMMAND_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>

struct command_t {
	struct list_head_t list;
	const char * name;
	const char * desc;

	void (*usage)(void);
	int (*exec)(int argc, char ** argv);
};

struct command_t * search_command(const char * name);
int register_command(struct command_t * cmd);
int unregister_command(struct command_t * cmd);
void command_foreach(void (*cb)(struct command_t * cmd, void * data), void * data);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_KERNEL_COMMAND_H__ */
