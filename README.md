# ESP32 Countdown Timer (ST7789 LCD)

This project implements a **simple countdown timer using an ESP32-C6 and a 1.47" ST7789 LCD display**.

When the countdown reaches zero, the display enters a **flashing alarm mode** to notify the user.

The project uses the **LovyanGFX graphics library** to drive the LCD.

---

## Features

- Countdown timer display (MM:SS)
- Large centered display for easy readability
- Flashing red alarm screen when time is up
- Configurable countdown time
- Adjustable flashing duration and interval
- Compatible with **Waveshare ESP32-C6-LCD-1.47**

---

## Hardware

- ESP32-C6 development board
- Waveshare ESP32-C6-LCD-1.47 display
- ST7789 LCD controller

---

## Pin Mapping

| LCD Pin | ESP32 GPIO |
|-------|-----------|
| MOSI | GPIO6 |
| SCLK | GPIO7 |
| CS | GPIO14 |
| DC | GPIO15 |
| RST | GPIO21 |
| BL | GPIO22 |

Backlight is enabled when **GPIO22 is HIGH**.

---

## Required Library

Install the following library in Arduino IDE:

LovyanGFX

Library repository:

https://github.com/lovyan03/LovyanGFX

---

## Configuration

Countdown time can be modified in the code:

#define COUNT_MIN  2
#define COUNT_SEC  0

Example: 5 minutes

#define COUNT_MIN  5
#define COUNT_SEC  0

---

## Alarm Settings

#define ALARM_FLASH_SECONDS 30
#define FLASH_INTERVAL_MS 300

- `ALARM_FLASH_SECONDS` → total flashing duration
- `FLASH_INTERVAL_MS` → flashing speed

---

## Display Behavior

During countdown:

02:00  
TIMER

After time expires:

TIME UP!

The screen flashes between **red and black**.

---

## Possible Improvements

- Add buttons to set time
- Add buzzer alarm
- Add rotary encoder for adjustment
- Add WiFi timer control
- Add multiple timer presets
