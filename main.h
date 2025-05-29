#ifndef MAIN_H
#define MAIN_H

#include <xc.h>
#include "adc.h"
#include "clcd.h"
#include "digital_keypad.h"
#include "ds1307.h"
#include "i2c.h"
#include "EEprom.h"
#include "car_black_box_def.h"
#include <string.h>
#include "timers.h"
#include "uart.h"

#define DASHBOARD_SCREEN     0x00
#define LOGIN_SCREEN         0x01
#define MAIN_MENU_SCREEN     0x02
#define VIEW_LOG_SCREEN      0x03
#define CLEAR_LOG_SCREEN     0x04
#define DOWNLOAD_LOG_SCREEN  0x05
#define SET_TIME_SCREEN      0x06
#define CHANG_PASS_SCREEN    0x07

#define RESET_PASSWORD       0x11
#define RESET_NOTHING        0x22
#define LOGIN_SUCCESS        0x33
#define RESET_MENU           0x44
#define RESET_VIEW           0x55
#define RESET_CLEAR          0x66
#define RESET_TIME           0x77
#define RETURN_BACK_MENU     0x88
#define RETURN_BACK_DASH     0x99

#define LONG_PRESS           0x10
#define SHORT_PRESS          0x20
#define NO_PRESS             0x30

#endif