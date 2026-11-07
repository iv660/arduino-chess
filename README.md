# Arduino Chess - XC Slim Pad

Chess game for Arduino Nano with ST7735 TFT display and joystick controls.

## Hardware

**Board:** Arduino Nano (ATmega328P)

**Components:**
- 1.8" TFT Display with ST7735 controller
- Analog Joystick with momentary switch
- Pause button (normally high)
- Start button (normally high)

See [HARDWARE.md](reference/3rd-party/xc-slim-pad/HARDWARE.md) for detailed hardware specifications.

## Development

This project uses PlatformIO for building and uploading.

### Build

```bash
pio run
```

### Upload to Arduino Nano

```bash
pio run --target upload
```

### Serial Monitor

```bash
pio device monitor
```

## Pin Configuration

### TFT Display (ST7735)
- **CS:** D10
- **RST:** RST (connected to Arduino Reset pin)
- **DC:** D8
- **MOSI:** D11 (hardware SPI)
- **MISO:** D12 (hardware SPI)
- **SCK:** D13 (hardware SPI)

### Joystick
- **X-axis:** A0 (analog)
- **Y-axis:** A1 (analog)
- **Button:** D2 (digital input, normally high)

### Control Buttons
- **Pause:** D3 (digital input, normally high)
- **Start:** D4 (digital input, normally high)

### Built-in LED
- **LED:** D13

## Getting Started

The current version includes a basic LED blink test to verify board programming.

After uploading, the built-in LED (pin 13) should blink at 1 Hz, and status messages will be printed to the serial monitor at 115200 baud.

## License

To be determined.
