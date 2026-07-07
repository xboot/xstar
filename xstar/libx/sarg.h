#ifndef __XSTAR_LIBX_SARG_H__
#define __XSTAR_LIBX_SARG_H__

#ifdef __cplusplus
extern "C" {
#endif

struct sarg_t {
	int argc;
	char ** argv;
	struct {
		int index;
		int literal;
	} iter;
};

void sarg_init(struct sarg_t * sarg, int argc, char ** argv);
int sarg_valid(struct sarg_t * sarg, const char ** opts, int min, int max);
int sarg_has(struct sarg_t * sarg, const char * name);
const char * sarg_at(struct sarg_t * sarg, int index);
const char * sarg_get(struct sarg_t * sarg, const char * name, const char * def);
int sarg_get_int(struct sarg_t * sarg, const char * name, int def);
unsigned int sarg_get_uint(struct sarg_t * sarg, const char * name, unsigned int def);
long sarg_get_long(struct sarg_t * sarg, const char * name, long def);
unsigned long sarg_get_ulong(struct sarg_t * sarg, const char * name, unsigned long def);
double sarg_get_double(struct sarg_t * sarg, const char * name, double def);

void sarg_iter_reset(struct sarg_t * sarg);
const char * sarg_iter_next(struct sarg_t * sarg);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_LIBX_SARG_H__ */
