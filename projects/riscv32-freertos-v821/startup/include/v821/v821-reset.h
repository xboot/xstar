#ifndef __V821_RESET_H__
#define __V821_RESET_H__

#ifdef __cplusplus
extern "C" {
#endif

#define V821_RESET_DPSS_TOP		(32 * 0 + 31)
#define V821_RESET_WKUP_TMR		(32 * 0 + 29)
#define V821_RESET_MCSI			(32 * 0 + 28)
#define V821_RESET_HMCSI		(32 * 0 + 27)
#define V821_RESET_G2D			(32 * 0 + 26)
#define V821_RESET_DE			(32 * 0 + 25)
#define V821_RESET_GMAC			(32 * 0 + 24)
#define V821_RESET_USB_PHY		(32 * 0 + 23)
#define V821_RESET_USB_OHCI		(32 * 0 + 22)
#define V821_RESET_USB_EHCI		(32 * 0 + 21)
#define V821_RESET_USB_OTG		(32 * 0 + 20)
#define V821_RESET_USB			(32 * 0 + 19)
#define V821_RESET_UART3		(32 * 0 + 18)
#define V821_RESET_UART2		(32 * 0 + 17)
#define V821_RESET_UART1		(32 * 0 + 16)
#define V821_RESET_UART0		(32 * 0 + 15)
#define V821_RESET_TWI0			(32 * 0 + 14)
#define V821_RESET_PWM			(32 * 0 + 13)
#define V821_RESET_WIEGAND		(32 * 0 + 12)
#define V821_RESET_TRNG			(32 * 0 + 11)
#define V821_RESET_TIMER		(32 * 0 + 10)
#define V821_RESET_SGDMA		(32 * 0 +  9)
#define V821_RESET_DMA			(32 * 0 +  8)
#define V821_RESET_SYSCTRL		(32 * 0 +  7)
#define V821_RESET_CE			(32 * 0 +  6)
#define V821_RESET_HSTIMER		(32 * 0 +  5)
#define V821_RESET_SPLOCK		(32 * 0 +  4)
#define V821_RESET_DRAM			(32 * 0 +  3)
#define V821_RESET_RV_MSGBOX	(32 * 0 +  2)
#define V821_RESET_RV_SYS_APB	(32 * 0 +  1)
#define V821_RESET_RV_CFG		(32 * 0 +  0)

#define V821_RESET_A27_CFG		(32 * 1 + 28)
#define V821_RESET_A27_MSGBOX	(32 * 1 + 27)
#define V821_RESET_A27			(32 * 1 + 26)
#define V821_RESET_TWI2			(32 * 1 + 25)
#define V821_RESET_TWI1			(32 * 1 + 24)
#define V821_RESET_SPI2			(32 * 1 + 23)
#define V821_RESET_SMHC1		(32 * 1 + 21)
#define V821_RESET_SMHC0		(32 * 1 + 20)
#define V821_RESET_SPI1			(32 * 1 + 19)
#define V821_RESET_DBGSYS		(32 * 1 + 18)
#define V821_RESET_MBUS			(32 * 1 + 12)
#define V821_RESET_TCON			(32 * 1 + 11)
#define V821_RESET_VO0			(32 * 1 + 10)
#define V821_RESET_I2S1			(32 * 1 +  9)
#define V821_RESET_I2S0			(32 * 1 +  8)
#define V821_RESET_DMIC			(32 * 1 +  7)
#define V821_RESET_AUDIO		(32 * 1 +  6)
#define V821_RESET_SPIF			(32 * 1 +  5)
#define V821_RESET_SPI			(32 * 1 +  4)
#define V821_RESET_VE			(32 * 1 +  3)
#define V821_RESET_THS			(32 * 1 +  1)
#define V821_RESET_GPA			(32 * 1 +  0)

#define V821_RESET_A27_WFG		(32 * 2 +  2)
#define V821_RESET_GPIO_WDG		(32 * 2 +  1)
#define V821_RESET_RV_WDG		(32 * 2 +  0)

#define V821_RESET_E907			(32 * 3 +  0)

#ifdef __cplusplus
}
#endif

#endif /* __V821_RESET_H__ */
