#ifndef __XSTAR_H__
#define __XSTAR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>

#include <kernel/audio/effect.h>
#include <kernel/audio/mixer.h>
#include <kernel/audio/sink.h>
#include <kernel/audio/source.h>
#include <kernel/command/command.h>
#include <kernel/core/cochannel.h>
#include <kernel/core/coroutine.h>
#include <kernel/core/logger.h>
#include <kernel/core/profiler.h>
#include <kernel/core/psub.h>
#include <kernel/core/setting.h>
#include <kernel/core/thchannel.h>
#include <kernel/core/thworker.h>
#include <kernel/font/font.h>
#include <kernel/font/truetype.h>
#include <kernel/graphic/blur.h>
#include <kernel/graphic/color.h>
#include <kernel/graphic/dirtylist.h>
#include <kernel/graphic/matrix2d.h>
#include <kernel/graphic/region.h>
#include <kernel/graphic/surface.h>
#include <kernel/shell/context.h>
#include <kernel/shell/ctrlc.h>
#include <kernel/shell/readline.h>
#include <kernel/shell/shell.h>
#include <kernel/time/delay.h>
#include <kernel/time/delaycall.h>
#include <kernel/time/timer.h>
#include <kernel/time/wallclock.h>
#include <kernel/vision/vision.h>
#include <kernel/vision/bitwise.h>
#include <kernel/vision/colormap.h>
#include <kernel/vision/dilate.h>
#include <kernel/vision/dither.h>
#include <kernel/vision/erode.h>
#include <kernel/vision/gamma.h>
#include <kernel/vision/gray.h>
#include <kernel/vision/inrange.h>
#include <kernel/vision/invert.h>
#include <kernel/vision/rectangle.h>
#include <kernel/vision/resize.h>
#include <kernel/vision/sepia.h>
#include <kernel/vision/text.h>
#include <kernel/vision/threshold.h>
#include <kernel/window/event.h>
#include <kernel/window/window.h>
#include <kernel/xfs/xfs.h>
#include <kernel/xfs/archiver.h>

#include <driver/device.h>
#include <driver/driver.h>
#include <driver/adc/adc.h>
#include <driver/atnet/atnet.h>
#include <driver/atomic/atomic.h>
#include <driver/audio/audio.h>
#include <driver/audio/capture.h>
#include <driver/audio/playback.h>
#include <driver/battery/battery.h>
#include <driver/block/block.h>
#include <driver/block/partition.h>
#include <driver/buzzer/buzzer.h>
#include <driver/camera/camera.h>
#include <driver/clk/clk.h>
#include <driver/clockevent/clockevent.h>
#include <driver/clocksource/clocksource.h>
#include <driver/compass/compass.h>
#include <driver/console/console.h>
#include <driver/dac/dac.h>
#include <driver/dma/dma.h>
#include <driver/framebuffer/framebuffer.h>
#include <driver/g2d/g2d.h>
#include <driver/gmeter/gmeter.h>
#include <driver/gnss/gnss.h>
#include <driver/gpio/gpio.h>
#include <driver/gyroscope/gyroscope.h>
#include <driver/hygrometer/hygrometer.h>
#include <driver/i2c/i2c.h>
#include <driver/input/input.h>
#include <driver/interrupt/interrupt.h>
#include <driver/led/led.h>
#include <driver/ledstrip/ledstrip.h>
#include <driver/ledtrigger/ledtrigger.h>
#include <driver/light/light.h>
#include <driver/limiter/limiter.h>
#include <driver/motor/motor.h>
#include <driver/net/net.h>
#include <driver/net/ntpclient.h>
#include <driver/nvmem/nvmem.h>
#include <driver/oximeter/oximeter.h>
#include <driver/pressure/pressure.h>
#include <driver/printer/printer.h>
#include <driver/proximity/proximity.h>
#include <driver/pwm/pwm.h>
#include <driver/regulator/regulator.h>
#include <driver/reset/reset.h>
#include <driver/rng/rng.h>
#include <driver/rtc/rtc.h>
#include <driver/sd/sdhci.h>
#include <driver/servo/servo.h>
#include <driver/spi/spi.h>
#include <driver/spinlock/spinlock.h>
#include <driver/stepper/stepper.h>
#include <driver/thermometer/thermometer.h>
#include <driver/uart/uart.h>
#include <driver/vibrator/vibrator.h>
#include <driver/watchdog/watchdog.h>

#define XSTAR_VERSION_MAJOR		1
#define XSTAR_VERSION_MINOR		0
#define XSTAR_VERSION_PATCH		0

void xstar_init(struct xos_environ_t * env, const char * dtree);
void xstar_exit(void);

int xstar_version(void);
const char * xstar_banner(void);
const char * xstar_clogo(void);

int xstar_feature_coroutine(void);
int xstar_feature_thread(void);

void xstar_sync(void);
void xstar_shutdown(void);
void xstar_reboot(void);
void xstar_standby(void);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_H__ */
