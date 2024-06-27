#
# Makefile for zte touchscreen_v2 driver.
#

zte_tpd-y := ztp_core.o ztp_report_algo.o ztp_state_change.o
zte_tpd-$(CONFIG_TOUCHSCREEN_UFP_MAC) +=  ztp_ufp.o

######################################################
ifneq ($(filter y, $(CONFIG_TOUCHSCREEN_GOODIX_BRL_9916R)),)
	ifneq ($(filter y, $(CONFIG_TOUCHSCREEN_GOODIX_BRL_I2C)),)
		zte_tpd-y += goodix_berlin_driver_9916r/goodix_brl_i2c.o
	else
		zte_tpd-y += goodix_berlin_driver_9916r/goodix_brl_spi.o
	endif
endif
zte_tpd-$(CONFIG_TOUCHSCREEN_GOODIX_BRL_9916R) += \
	goodix_berlin_driver_9916r/goodix_brl_fwupdate.o \
	goodix_berlin_driver_9916r/goodix_brl_hw.o \
	goodix_berlin_driver_9916r/goodix_cfg_bin.o \
	goodix_berlin_driver_9916r/goodix_ts_core.o \
	goodix_berlin_driver_9916r/goodix_ts_gesture.o \
	goodix_berlin_driver_9916r/goodix_ts_inspect.o \
	goodix_berlin_driver_9916r/goodix_ts_tools.o \
	goodix_berlin_driver_9916r/goodix_ts_utils.o \
	goodix_berlin_driver_9916r/goodix_common_interface.o		

######################################################

obj-m += zte_tpd.o
