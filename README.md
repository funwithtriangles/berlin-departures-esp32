# Berlin Departure Board (esp32)

Arduino code for a simple home departures board, designed for a 64x32 LED panel.

## Setup

### WiFi Configuration

1. Copy the example credentials file:

   ```bash
   cp credentials.example.h credentials.h
   ```

2. Edit `credentials.h` with your WiFi network details:
   ```cpp
   const char* ssid = "YourNetworkName";
   const char* password = "YourPassword";
   ```

> [!NOTE]
> The `credentials.h` file is gitignored to keep your WiFi credentials private.

## LED Panel

It's a 64x32 RGB LED panel, designed to have chaining capabilities for large wall displays, in this project we're just using one though. I ordered mine from Ali Express, they all seem to be somewhat similar.
The data connection is known as [HUB75/HUB75E](http://www.moonbaseotago.com/hub75/).

All the guides I have read say that getting the right power for the panel is important. I don't doubt that, but for this project I've managed to power both the ESP32 and the LED panel from the USBC coming out of my Macbook air 🤷. Maybe because it's not using all the LEDs and at medium brightness.

## Arduino Libraries

To drive the board, this project is using [ESP32-HUB75-MatrixPanel-DMA](https://github.com/mrcodetastic/ESP32-HUB75-MatrixPanel-DMA). I chose it because it plays well with Adafruit's graphics library while being very configurable (e.g. choose your pins).

We're also using [U8g2_for_Adafruit_GFX](https://github.com/olikraus/U8g2_for_Adafruit_GFX) for a custom font.

## Custom font

I wanted the font to be 6px high, not the usual 8px. This lead me down a bit of a rabbithole and I ended up designing my own custom font. Currently the font only has the following glyphs:

```
0123456789,UM
```

I designed the font using [BitsNPicas](https://github.com/kreativekorp/bitsnpicas). OSX users [beware of the gatekeeper](https://disable-gatekeeper.github.io/).

The font needs to be saved as a `bdf` file, and then converted to an array that can be placed directly in the code, to be used by the [u8g2](https://github.com/olikraus/u8g2) library. There is a conversion tool to do this (`bdfconv`).

- [Windows](https://github.com/olikraus/u8g2/tree/master/tools/font/bdfconv)
- [OSX](https://github.com/olikraus/u8glib/issues/419#issuecomment-2039043536) (also included in this repo)
- [u8g2 FAQ](https://github.com/olikraus/u8g2/blob/master/doc/faq.txt)

**Conversion instructions for OSX users**

1. Save your new `font.bdf` into this directory
2. Navigate to this directory using the terminal
3. Run the below command

```bash
./bdfconv -f 1 -m '0-255' -n font -o font.c font.bdf
```

4. Copy the contents of `font.c` into `berlin-departures-esp32.ino`, replacing the previous font part of the code.
5. Delete the `font.c` file (otherwise you might get compilation errors when trying to upload to your device)

> [!NOTE]  
> Each font glyph has 1px of padding on the right-hand side. I couldn't work out how to implement this in code, using `u8g2`.
