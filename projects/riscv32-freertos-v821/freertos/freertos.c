#include <freertos/freertos.h>

void freertos_risc_v_application_exception_handler(uint32_t cause)
{
	LOG("freertos_risc_v_application_exception_handler %d\r\n", cause);
}

void freertos_risc_v_application_interrupt_handler(uint32_t cause)
{
	LOG("freertos_risc_v_application_exception_handler %d\r\n", cause);
	interrupt_handle_exception(NULL);
}

void freertos_init(void)
{
	extern unsigned char __heap_start[];
	extern unsigned char __heap_end[];
	const struct HeapRegion xHeapRegions[] = {
		{ (uint8_t *)__heap_start, (size_t)(__heap_end - __heap_start) },
		{ (uint8_t *)0, (size_t)0 },
	};
	vPortDefineHeapRegions(xHeapRegions);

	extern void freertos_risc_v_trap_handler(void);
	__asm__ __volatile__("csrw mtvec, %0" : : "r" (freertos_risc_v_trap_handler));
}

void freertos_run(void (*func)(void *))
{
	if(func)
		xTaskCreate(func, "xstar", 65536 / sizeof(StackType_t), NULL, configMAX_PRIORITIES / 2, NULL);
	vTaskStartScheduler();
	while(1);
}

void freertos_exit(void)
{
}
