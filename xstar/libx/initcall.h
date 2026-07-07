#ifndef __XSTAR_LIBX_INITCALL_H__
#define __XSTAR_LIBX_INITCALL_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*initcall_t)(void);
typedef void (*exitcall_t)(void);

#define __define_initcall(level, fn) \
	static const initcall_t __initcall_##level##_##fn \
	__attribute__((__used__, __section__("xstar_initcall_"#level))) = fn

#define __define_exitcall(level, fn) \
	static const exitcall_t __exitcall_##level##_##fn \
	__attribute__((__used__, __section__("xstar_exitcall_"#level))) = fn

#define pure_initcall(fn)		__define_initcall(0, fn)
#define machine_initcall(fn)	__define_initcall(1, fn)
#define core_initcall(fn)		__define_initcall(2, fn)
#define postcore_initcall(fn)	__define_initcall(3, fn)
#define driver_initcall(fn)		__define_initcall(4, fn)
#define subsys_initcall(fn)		__define_initcall(5, fn)
#define command_initcall(fn)	__define_initcall(6, fn)
#define server_initcall(fn)		__define_initcall(7, fn)
#define wboxtest_initcall(fn)	__define_initcall(8, fn)
#define late_initcall(fn)		__define_initcall(9, fn)

#define pure_exitcall(fn)		__define_exitcall(0, fn)
#define machine_exitcall(fn)	__define_exitcall(1, fn)
#define core_exitcall(fn)		__define_exitcall(2, fn)
#define postcore_exitcall(fn)	__define_exitcall(3, fn)
#define driver_exitcall(fn)		__define_exitcall(4, fn)
#define subsys_exitcall(fn)		__define_exitcall(5, fn)
#define command_exitcall(fn)	__define_exitcall(6, fn)
#define server_exitcall(fn)		__define_exitcall(7, fn)
#define wboxtest_exitcall(fn)	__define_exitcall(8, fn)
#define late_exitcall(fn)		__define_exitcall(9, fn)

void do_initcalls(void);
void do_exitcalls(void);

/*
 * final init calls
 */
#define final_initcall(fn)		__define_initcall(final, fn)
#define final_exitcall(fn)		__define_exitcall(final, fn)

void do_init_final(void);
void do_exit_final(void);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_LIBX_INITCALL_H__ */
