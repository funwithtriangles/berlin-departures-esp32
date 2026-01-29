#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <U8g2_for_Adafruit_GFX.h>

const uint8_t font[281] U8G2_FONT_SECTION("font") =
    "/\0\2\2\3\3\1\1\4\5\6\0\0\6\0\6\0\0\0\0\0\0\374\0\4\300\10\1\4\300\10\2"
    "\4\300\10\3\4\300\10\4\4\300\10\5\4\300\10\6\4\300\10\7\4\300\10\10\4\300\10\11\4\300\10\12"
    "\4\300\10\13\4\300\10\14\4\300\10\15\4\300\10\16\4\300\10\17\4\300\10\20\4\300\10\21\4\300\10\22"
    "\4\300\10\23\4\300\10\24\4\300\10\25\4\300\10\26\4\300\10\27\4\300\10\30\4\300\10\31\4\300\10\32"
    "\4\300\10\33\4\300\10\34\4\300\10\35\4\300\10\36\4\300\10\37\4\300\10 \4\300\11,\5\322\213\12"
    "\60\10\365\336\312\356\264\0\61\6\362\333\352\0\62\12\365\336\312\31\245\225\321\20\63\12\365\316\320(j\250"
    "\323\2\64\12\365~\226)\15\235Q\2\65\12\365\316\215F\206:-\0\66\12\365\336\312h\244\254\323\2"
    "\67\12\365\316\320(\327(\243\10\70\12\365\336\312ie\235\26\0\71\12\365\336\312:\215\214\322\2M\10"
    "\365N^+i;U\7\365N\366N\13\177\4\300\10\0\0\0\4\377\377\0";

U8G2_FOR_ADAFRUIT_GFX u8g2;

#define R1_PIN 2
#define G1_PIN 13
#define B1_PIN 4
#define R2_PIN 16
#define G2_PIN 12
#define B2_PIN 17
#define A_PIN 5
#define B_PIN 27
#define C_PIN 18
#define D_PIN 26
#define E_PIN -1 // required for 1/32 scan panels, like 64x64px. Any available pin would do, i.e. IO32
#define LAT_PIN 25
#define OE_PIN 21
#define CLK_PIN 19

#define PANEL_RES_X 64 // Number of pixels wide of each INDIVIDUAL panel module.
#define PANEL_RES_Y 32 // Number of pixels tall of each INDIVIDUAL panel module.
#define PANEL_CHAIN 1  // Total number of panels chained one to another

#define ROW_HEIGHT 8
#define BOX_WIDTH 21
#define MINUTES_X BOX_WIDTH + 1

// MatrixPanel_I2S_DMA dma_display;
MatrixPanel_I2S_DMA *dma_display = nullptr;

uint16_t u6Purple, u9Orange, tramYellow, white, black, red;

void setup()
{

    HUB75_I2S_CFG::i2s_pins _pins = {R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN, A_PIN, B_PIN, C_PIN, D_PIN, E_PIN, LAT_PIN, OE_PIN, CLK_PIN};
    // Module configuration
    HUB75_I2S_CFG mxconfig(
        PANEL_RES_X, // module width
        PANEL_RES_Y, // module height
        PANEL_CHAIN, // Chain length
        _pins        // pin mapping
    );

    // Display Setup
    dma_display = new MatrixPanel_I2S_DMA(mxconfig);
    dma_display->begin();
    dma_display->setBrightness8(150); // 0-255
    dma_display->clearScreen();

    white = dma_display->color565(255, 255, 255);
    black = dma_display->color565(0, 0, 0);
    red = dma_display->color565(255, 0, 0);
    u6Purple = dma_display->color565(140, 109, 171);
    u9Orange = dma_display->color565(238, 118, 30);
    tramYellow = dma_display->color565(240, 215, 33);

    u8g2.begin(*dma_display);

    int16_t x = 5;
    int16_t y = 7;

    dma_display->fillRect(0, 0, BOX_WIDTH, 8, u6Purple);
    dma_display->fillRect(5, 1, 11, 6, black);
    dma_display->fillRect(0, ROW_HEIGHT, BOX_WIDTH, 8, u9Orange);
    dma_display->fillRect(5, 1 + ROW_HEIGHT, 11, 6, black);

    // Stations (left side)
    u8g2.setFont(font);
    u8g2.setForegroundColor(white);
    u8g2.setCursor(x, y);
    u8g2.print(F("U6"));
    u8g2.setCursor(x, y + ROW_HEIGHT);
    u8g2.print(F("U9"));
    u8g2.setForegroundColor(tramYellow);
    u8g2.setCursor(x - 1, y + ROW_HEIGHT * 2);
    u8g2.print(F("M13"));
    u8g2.setCursor(x, y + ROW_HEIGHT * 3);
    u8g2.print(F("50"));

    // Minute displays (right side)
    u8g2.setForegroundColor(white);
    // U6
    u8g2.setCursor(MINUTES_X, y + ROW_HEIGHT * 0);
    u8g2.print(F("1,15,23"));
    u8g2.setCursor(MINUTES_X, y + ROW_HEIGHT * 1);
    u8g2.print(F("55,55,55"));
    u8g2.setCursor(MINUTES_X, y + ROW_HEIGHT * 2);
    u8g2.setForegroundColor(red);
    u8g2.print(F("3"));
    u8g2.setForegroundColor(white);
    u8g2.print(F(",13,25"));
}

void loop()
{
}
