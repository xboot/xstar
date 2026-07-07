/*
 * Copyright(c) Jianjun Jiang <8192542@qq.com>
 * Mobile phone: +86-18665388956
 * QQ: 8192542
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <libx/initcall.h>

void do_initcalls(void)
{
	initcall_t * call;

	extern initcall_t __start_xstar_initcall_0[];
	extern initcall_t __stop_xstar_initcall_0[];
	call =  &(*__start_xstar_initcall_0);
	while(call < &(*__stop_xstar_initcall_0))
	{
		(*call)();
		call++;
	}

	extern initcall_t __start_xstar_initcall_1[];
	extern initcall_t __stop_xstar_initcall_1[];
	call =  &(*__start_xstar_initcall_1);
	while(call < &(*__stop_xstar_initcall_1))
	{
		(*call)();
		call++;
	}

	extern initcall_t __start_xstar_initcall_2[];
	extern initcall_t __stop_xstar_initcall_2[];
	call =  &(*__start_xstar_initcall_2);
	while(call < &(*__stop_xstar_initcall_2))
	{
		(*call)();
		call++;
	}

	extern initcall_t __start_xstar_initcall_3[];
	extern initcall_t __stop_xstar_initcall_3[];
	call =  &(*__start_xstar_initcall_3);
	while(call < &(*__stop_xstar_initcall_3))
	{
		(*call)();
		call++;
	}

	extern initcall_t __start_xstar_initcall_4[];
	extern initcall_t __stop_xstar_initcall_4[];
	call =  &(*__start_xstar_initcall_4);
	while(call < &(*__stop_xstar_initcall_4))
	{
		(*call)();
		call++;
	}

	extern initcall_t __start_xstar_initcall_5[];
	extern initcall_t __stop_xstar_initcall_5[];
	call =  &(*__start_xstar_initcall_5);
	while(call < &(*__stop_xstar_initcall_5))
	{
		(*call)();
		call++;
	}

	extern initcall_t __start_xstar_initcall_6[];
	extern initcall_t __stop_xstar_initcall_6[];
	call =  &(*__start_xstar_initcall_6);
	while(call < &(*__stop_xstar_initcall_6))
	{
		(*call)();
		call++;
	}

	extern initcall_t __start_xstar_initcall_7[];
	extern initcall_t __stop_xstar_initcall_7[];
	call =  &(*__start_xstar_initcall_7);
	while(call < &(*__stop_xstar_initcall_7))
	{
		(*call)();
		call++;
	}

	extern initcall_t __start_xstar_initcall_8[];
	extern initcall_t __stop_xstar_initcall_8[];
	call =  &(*__start_xstar_initcall_8);
	while(call < &(*__stop_xstar_initcall_8))
	{
		(*call)();
		call++;
	}

	extern initcall_t __start_xstar_initcall_9[];
	extern initcall_t __stop_xstar_initcall_9[];
	call =  &(*__start_xstar_initcall_9);
	while(call < &(*__stop_xstar_initcall_9))
	{
		(*call)();
		call++;
	}
}

void do_exitcalls(void)
{
	exitcall_t * call;

	extern exitcall_t __start_xstar_exitcall_9[];
	extern exitcall_t __stop_xstar_exitcall_9[];
	call =  &(*__start_xstar_exitcall_9);
	while(call < &(*__stop_xstar_exitcall_9))
	{
		(*call)();
		call++;
	}

	extern exitcall_t __start_xstar_exitcall_8[];
	extern exitcall_t __stop_xstar_exitcall_8[];
	call =  &(*__start_xstar_exitcall_8);
	while(call < &(*__stop_xstar_exitcall_8))
	{
		(*call)();
		call++;
	}

	extern exitcall_t __start_xstar_exitcall_7[];
	extern exitcall_t __stop_xstar_exitcall_7[];
	call =  &(*__start_xstar_exitcall_7);
	while(call < &(*__stop_xstar_exitcall_7))
	{
		(*call)();
		call++;
	}

	extern exitcall_t __start_xstar_exitcall_6[];
	extern exitcall_t __stop_xstar_exitcall_6[];
	call =  &(*__start_xstar_exitcall_6);
	while(call < &(*__stop_xstar_exitcall_6))
	{
		(*call)();
		call++;
	}

	extern exitcall_t __start_xstar_exitcall_5[];
	extern exitcall_t __stop_xstar_exitcall_5[];
	call =  &(*__start_xstar_exitcall_5);
	while(call < &(*__stop_xstar_exitcall_5))
	{
		(*call)();
		call++;
	}

	extern exitcall_t __start_xstar_exitcall_4[];
	extern exitcall_t __stop_xstar_exitcall_4[];
	call =  &(*__start_xstar_exitcall_4);
	while(call < &(*__stop_xstar_exitcall_4))
	{
		(*call)();
		call++;
	}

	extern exitcall_t __start_xstar_exitcall_3[];
	extern exitcall_t __stop_xstar_exitcall_3[];
	call =  &(*__start_xstar_exitcall_3);
	while(call < &(*__stop_xstar_exitcall_3))
	{
		(*call)();
		call++;
	}

	extern exitcall_t __start_xstar_exitcall_2[];
	extern exitcall_t __stop_xstar_exitcall_2[];
	call =  &(*__start_xstar_exitcall_2);
	while(call < &(*__stop_xstar_exitcall_2))
	{
		(*call)();
		call++;
	}

	extern exitcall_t __start_xstar_exitcall_1[];
	extern exitcall_t __stop_xstar_exitcall_1[];
	call =  &(*__start_xstar_exitcall_1);
	while(call < &(*__stop_xstar_exitcall_1))
	{
		(*call)();
		call++;
	}

	extern exitcall_t __start_xstar_exitcall_0[];
	extern exitcall_t __stop_xstar_exitcall_0[];
	call =  &(*__start_xstar_exitcall_0);
	while(call < &(*__stop_xstar_exitcall_0))
	{
		(*call)();
		call++;
	}
}

static void __pure_init(void)
{
}
static void __pure_exit(void)
{
}
pure_initcall(__pure_init);
pure_exitcall(__pure_exit);

static void __machine_init(void)
{
}
static void __machine_exit(void)
{
}
machine_initcall(__machine_init);
machine_exitcall(__machine_exit);

static void __core_init(void)
{
}
static void __core_exit(void)
{
}
core_initcall(__core_init);
core_exitcall(__core_exit);

static void __postcore_init(void)
{
}
static void __postcore_exit(void)
{
}
postcore_initcall(__postcore_init);
postcore_exitcall(__postcore_exit);

static void __driver_init(void)
{
}
static void __driver_exit(void)
{
}
driver_initcall(__driver_init);
driver_exitcall(__driver_exit);

static void __subsys_init(void)
{
}
static void __subsys_exit(void)
{
}
subsys_initcall(__subsys_init);
subsys_exitcall(__subsys_exit);

static void __command_init(void)
{
}
static void __command_exit(void)
{
}
command_initcall(__command_init);
command_exitcall(__command_exit);

static void __server_init(void)
{
}
static void __server_exit(void)
{
}
server_initcall(__server_init);
server_exitcall(__server_exit);

static void __wboxtest_init(void)
{
}
static void __wboxtest_exit(void)
{
}
wboxtest_initcall(__wboxtest_init);
wboxtest_exitcall(__wboxtest_exit);

static void __late_init(void)
{
}
static void __late_exit(void)
{
}
late_initcall(__late_init);
late_exitcall(__late_exit);

/*
 * final init calls
 */
void do_init_final(void)
{
	initcall_t * call;

	extern initcall_t __start_xstar_initcall_final[];
	extern initcall_t __stop_xstar_initcall_final[];
	call =  &(*__start_xstar_initcall_final);
	while(call < &(*__stop_xstar_initcall_final))
	{
		(*call)();
		call++;
	}
}

void do_exit_final(void)
{
	exitcall_t * call;

	extern exitcall_t __start_xstar_exitcall_final[];
	extern exitcall_t __stop_xstar_exitcall_final[];
	call =  &(*__start_xstar_exitcall_final);
	while(call < &(*__stop_xstar_exitcall_final))
	{
		(*call)();
		call++;
	}
}

static void __final_init(void)
{
}
static void __final_exit(void)
{
}
final_initcall(__final_init);
final_exitcall(__final_exit);
