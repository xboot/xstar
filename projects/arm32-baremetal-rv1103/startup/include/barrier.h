#ifndef __ARM32_BARRIER_H__
#define __ARM32_BARRIER_H__

#ifdef __cplusplus
extern "C" {
#endif

#define isb()		__asm__ __volatile__ ("isb sy" : : : "memory")
#define dsb()		__asm__ __volatile__ ("dsb sy" : : : "memory")
#define dmb()		__asm__ __volatile__ ("dmb sy" : : : "memory")

/* Read and write memory barrier */
#define mb()		dsb()
/* Read memory barrier */
#define rmb()		dsb()
/* Write memory barrier */
#define wmb()		dsb()

/* SMP read and write memory barrier */
#define smp_mb()	dmb()
/* SMP read memory barrier */
#define smp_rmb()	dmb()
/* SMP write memory barrier */
#define smp_wmb()	dmb()

#ifdef __cplusplus
}
#endif

#endif /* __ARM32_BARRIER_H__ */
