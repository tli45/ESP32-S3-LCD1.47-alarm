// ==== 仅需这个显示库（Arduino 会自动包含 <Arduino.h>）====
#include <LovyanGFX.hpp>

// ==== Waveshare ESP32-C6-LCD-1.47 的 LCD 引脚映射 ====
#define PIN_LCD_MOSI  6     // LCD_MOSI -> GPIO6
#define PIN_LCD_SCLK  7     // LCD_SCLK -> GPIO7
#define PIN_LCD_CS    14    // LCD_CS   -> GPIO14
#define PIN_LCD_DC    15    // LCD_DC   -> GPIO15
#define PIN_LCD_RST   21    // LCD_RST  -> GPIO21
#define PIN_LCD_BL    22    // LCD_BL   -> GPIO22（高电平点亮）

// ==== 屏幕参数（ST7789，常见 172x320；若是 240x320 就改成 240/320 并调整 invert）====
static const int  PANEL_W = 172;
static const int  PANEL_H = 320;
static const bool PANEL_INVERT = true;
static const bool PANEL_RGB_ORDER = false;

// ==== 倒计时设定（你改这里）====
#define COUNT_MIN  2   // 分钟
#define COUNT_SEC  0   // 秒
#define ALARM_FLASH_SECONDS 30   // 到点后闪烁持续时长（秒）
#define FLASH_INTERVAL_MS  300   // 闪烁翻转间隔（毫秒）

// ==== LovyanGFX 面板封装 ====
class LGFX_ST7789 : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 _panel;
  lgfx::Bus_SPI _bus;
public:
  LGFX_ST7789() {
    auto b = _bus.config();
    b.spi_host   = SPI2_HOST;   // ESP32-C6 SPI2
    b.freq_write = 40000000;    // 40 MHz
    b.pin_sclk   = PIN_LCD_SCLK;
    b.pin_mosi   = PIN_LCD_MOSI;
    b.pin_miso   = -1;
    b.pin_dc     = PIN_LCD_DC;
    _bus.config(b);
    _panel.setBus(&_bus);

    auto p = _panel.config();
    p.pin_cs   = PIN_LCD_CS;
    p.pin_rst  = PIN_LCD_RST;
    p.pin_busy = -1;
    p.panel_width  = PANEL_W;
    p.panel_height = PANEL_H;
    p.offset_x = 0;
    p.offset_y = 0;
    p.invert   = PANEL_INVERT;
    p.readable = false;
    p.rgb_order= PANEL_RGB_ORDER;
    p.memory_width  = p.panel_width;
    p.memory_height = p.panel_height;
    _panel.config(p);

    setPanel(&_panel);
  }
} display;

// ==== 计时器内部变量 ====
static uint32_t countdown_total_ms = (uint32_t)(COUNT_MIN * 60UL + COUNT_SEC) * 1000UL;
static uint32_t start_ms = 0;
static bool     alarm_phase = false;    // 是否处于闪烁阶段
static uint32_t alarm_start_ms = 0;
static bool     flash_red = false;
static uint32_t last_flash_toggle = 0;

// ==== 绘制倒计时界面 ====
void drawCountdown(uint32_t remain_ms) {
  uint32_t remain_s = (remain_ms + 999) / 1000; // 四舍五入到秒
  uint32_t m = remain_s / 60;
  uint32_t s = remain_s % 60;

  display.startWrite();
  display.fillScreen(TFT_BLACK);
  display.setTextDatum(textdatum_t::middle_center);

  // 大号 mm:ss
  display.setFont(&fonts::Font7);
  display.setTextColor(TFT_WHITE, TFT_BLACK);
  char buf[16];
  snprintf(buf, sizeof(buf), "%02lu:%02lu", (unsigned long)m, (unsigned long)s);
  display.drawString(buf, display.width()/2, display.height()/2 - 20);

  // 提示字样
  display.setFont(&fonts::Font2);
  display.drawString("TIMER", display.width()/2, display.height()/2 + 20);
  display.endWrite();
}

// ==== 绘制闪烁界面 ====
void drawAlarmFlash(bool redFrame) {
  display.startWrite();
  display.fillScreen(redFrame ? TFT_RED : TFT_BLACK);
  display.setTextDatum(textdatum_t::middle_center);
  display.setFont(&fonts::Font7);
  display.setTextColor(TFT_WHITE, redFrame ? TFT_RED : TFT_BLACK);
  display.drawString("TIME UP!", display.width()/2, display.height()/2);
  display.endWrite();
}

void setup() {
  // 背光
  pinMode(PIN_LCD_BL, OUTPUT);
  digitalWrite(PIN_LCD_BL, HIGH);

  display.init();
  display.setRotation(1); // 0~3 视实际安装方向
  display.fillScreen(TFT_BLACK);

  // 启动计时
  start_ms = millis();
  alarm_phase = false;
  drawCountdown(countdown_total_ms);
}

void loop() {
  uint32_t now = millis();

  if (!alarm_phase) {
    // 计时中
    uint32_t elapsed = now - start_ms; // millis 溢出自动处理
    if (elapsed >= countdown_total_ms) {
      // 进入闪烁阶段
      alarm_phase = true;
      alarm_start_ms = now;
      last_flash_toggle = 0;
      flash_red = false;
      drawAlarmFlash(flash_red);
      return;
    }

    uint32_t remain = countdown_total_ms - elapsed;
    static uint32_t last_draw = 0;
    if (now - last_draw > 200) { // 5FPS 更新
      drawCountdown(remain);
      last_draw = now;
    }
  } else {
    // 闪烁阶段
    uint32_t alarm_elapsed = now - alarm_start_ms;
    if (alarm_elapsed < (uint32_t)ALARM_FLASH_SECONDS * 1000UL) {
      if (now - last_flash_toggle >= FLASH_INTERVAL_MS) {
        flash_red = !flash_red;
        drawAlarmFlash(flash_red);
        last_flash_toggle = now;
      }
    } else {
      // 闪烁结束后，停在 00:00 画面
      drawCountdown(0);
      // 如果想自动重新开始一轮计时，解除下面注释：
      // start_ms = now;
      // alarm_phase = false;
      // drawCountdown(countdown_total_ms);
      while (true) delay(1000); // 本次完成后不再循环
    }
  }

  delay(10);
}
