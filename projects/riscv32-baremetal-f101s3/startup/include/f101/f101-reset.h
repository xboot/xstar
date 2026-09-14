#ifndef __F101_RESET_H__
#define __F101_RESET_H__

#ifdef __cplusplus
extern "C" {
#endif

#define F101_RESET_MBUS			(32 *  0 + 30)
#define F101_RESET_DE			(32 *  1 + 16)
#define F101_RESET_DI			(32 *  2 + 16)
#define F101_RESET_G2D			(32 *  3 + 16)
#define F101_RESET_CE			(32 *  4 + 16)
#define F101_RESET_VE			(32 *  5 + 16)
#define F101_RESET_DMA			(32 *  6 + 16)
#define F101_RESET_HSTIMER		(32 *  7 + 16)
#define F101_RESET_DBGSYS		(32 *  8 + 16)
#define F101_RESET_PWM_BL		(32 *  9 + 16)
#define F101_RESET_PWM			(32 * 10 + 16)
#define F101_RESET_LPSRAM		(32 * 11 + 16)
#define F101_RESET_DRAM			(32 * 12 + 16)
#define F101_RESET_SMHC0		(32 * 13 + 16)
#define F101_RESET_SMHC2		(32 * 13 + 18)
#define F101_RESET_UART0		(32 * 14 + 16)
#define F101_RESET_UART1		(32 * 14 + 17)
#define F101_RESET_UART2		(32 * 14 + 18)
#define F101_RESET_UART3		(32 * 14 + 19)
#define F101_RESET_UART4		(32 * 14 + 20)
#define F101_RESET_UART5		(32 * 14 + 21)
#define F101_RESET_TWI0			(32 * 15 + 16)
#define F101_RESET_TWI1			(32 * 15 + 17)
#define F101_RESET_TWI2			(32 * 15 + 18)
#define F101_RESET_CAN0			(32 * 16 + 16)
#define F101_RESET_CAN1			(32 * 16 + 17)
#define F101_RESET_SPI0			(32 * 17 + 16)
#define F101_RESET_SPI1			(32 * 17 + 17)
#define F101_RESET_SPIF			(32 * 18 + 16)
#define F101_RESET_IRRX			(32 * 19 + 16)
#define F101_RESET_GPADC		(32 * 20 + 16)
#define F101_RESET_THS			(32 * 21 + 16)
#define F101_RESET_I2S0			(32 * 22 + 16)
#define F101_RESET_OWA			(32 * 23 + 16)
#define F101_RESET_CODEC		(32 * 24 + 16)
#define F101_RESET_USBOHCI0		(32 * 25 + 16)
#define F101_RESET_USBEHCI0		(32 * 25 + 20)
#define F101_RESET_USBOTG0		(32 * 25 + 24)
#define F101_RESET_DPSS_TOP		(32 * 26 + 16)
#define F101_RESET_DSI			(32 * 27 + 16)
#define F101_RESET_TCON_LCD		(32 * 28 + 16)
#define F101_RESET_LVDS0		(32 * 29 + 16)
#define F101_RESET_TPADC		(32 * 30 + 16)

#ifdef __cplusplus
}
#endif

#endif /* __F101_RESET_H__ */
