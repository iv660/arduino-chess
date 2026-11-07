# Hardware Setup Guide

## XC Slim Pad - Arduino Chess Hardware Configuration

This document describes the hardware setup for the Arduino Chess project using the XC Slim Pad configuration.

## Board

**Arduino Nano (ATmega328P)**
- Microcontroller: ATmega328P
- Operating Voltage: 5V
- Clock Speed: 16 MHz
- Flash Memory: 32 KB (2 KB used by bootloader)
- SRAM: 2 KB
- EEPROM: 1 KB

## Components

### 1.8" TFT Display with ST7735 Controller

The display uses SPI communication with the following connections:

| Display Pin | Arduino Nano Pin | Description |
|-------------|------------------|-------------|
| VCC         | 5V               | Power supply |
| GND         | GND              | Ground |
| CS          | D10              | Chip Select (SPI) |
| RESET       | RST              | Reset (connected to Arduino Reset pin) |
| A0/DC       | D8               | Data/Command select |
| SDA/MOSI    | D11              | Master Out Slave In (SPI) |
| SCK         | D13              | Serial Clock (SPI) |
| LED         | 3.3V or 5V       | Backlight power |

**Note:** The ST7735 display typically works with 3.3V logic, but many modules include level shifters for 5V compatibility. Verify your specific module's voltage requirements.

### Analog Joystick with Button

The joystick provides two analog axes (X and Y) and one momentary switch:

| Joystick Pin | Arduino Nano Pin | Description |
|--------------|------------------|-------------|
| VCC          | 5V               | Power supply |
| GND          | GND              | Ground |
| VRx          | A0               | X-axis analog output (0-1023) |
| VRy          | A1               | Y-axis analog output (0-1023) |
| SW           | D2               | Momentary switch (active LOW) |

**Center position:** Approximately 512 (neutral position for both axes)
- Moving right: VRx increases (towards 1023)
- Moving left: VRx decreases (towards 0)
- Moving up: VRy increases (towards 1023)
- Moving down: VRy decreases (towards 0)

The switch is normally HIGH (internal pull-up enabled in software) and goes LOW when pressed.

### Pause Button

| Button Pin   | Arduino Nano Pin | Description |
|--------------|------------------|-------------|
| One side     | D3               | Input pin |
| Other side   | GND              | Ground |

- Normally HIGH (internal pull-up enabled in software)
- Goes LOW when pressed
- No external pull-up resistor needed

### Start Button

| Button Pin   | Arduino Nano Pin | Description |
|--------------|------------------|-------------|
| One side     | D4               | Input pin |
| Other side   | GND              | Ground |

- Normally HIGH (internal pull-up enabled in software)
- Goes LOW when pressed
- No external pull-up resistor needed

## Pin Summary

### Digital Pins
- **D2:** Joystick button (INPUT_PULLUP)
- **D3:** Pause button (INPUT_PULLUP)
- **D4:** Start button (INPUT_PULLUP)
- **D8:** TFT DC (Data/Command)
- **D10:** TFT CS (Chip Select)
- **D11:** TFT MOSI (SPI)
- **D12:** TFT MISO (SPI, not used by ST7735)
- **D13:** TFT SCK (SPI) / Built-in LED
- **RST:** TFT Reset (shared with Arduino Reset)

### Analog Pins
- **A0:** Joystick X-axis
- **A1:** Joystick Y-axis

### Power
- **5V:** Power for display, joystick
- **GND:** Common ground for all components

## Assembly Notes

1. **Display Reset:** The ST7735 display Reset pin is connected to the Arduino Nano Reset pin. This means both devices reset together when the Arduino is reset. No separate reset control is available in software.

2. **SPI Conflict:** The built-in LED on pin D13 shares the SPI clock line. This may cause the LED to flicker during display updates. This is normal and does not affect functionality.

3. **Pull-up Resistors:** All buttons (Joystick, Pause, and Start) are configured with internal pull-up resistors in software using `INPUT_PULLUP` mode. No external pull-up resistors are present in the hardware design. External pull-up resistors can be added (10kΩ recommended) if needed for additional noise immunity.

4. **Display Orientation:** The ST7735 display can be used in different orientations. The software can rotate the display 0°, 90°, 180°, or 270° as needed.

5. **Power Consumption:** 
   - Arduino Nano: ~20 mA
   - ST7735 Display: ~20-40 mA (depending on brightness and content)
   - Total estimated: ~50-70 mA
   - Can be powered via USB or external 5V supply

## Testing

### LED Blink Test

The basic sketch (`src/main.cpp`) tests the Arduino Nano by blinking the built-in LED at 1 Hz. Upload the sketch and verify:

1. The LED on pin D13 blinks on/off every second
2. Serial output at 115200 baud shows status messages
3. No compilation or upload errors

### Component Testing (Future)

Individual component tests can be added to verify:
- Display initialization and graphics
- Joystick analog readings and button
- Pause and Start button inputs

## Schematic Reference

See `reference/3rd-party/xc-slim-pad/xc-slim-pad-schematic.png` for the complete schematic diagram.

## Troubleshooting

### Display Issues
- **No display:** Check power and SPI connections (CS, DC, RST, MOSI, SCK)
- **White screen:** Reset pin might be incorrect or display needs initialization delay
- **Corrupted display:** Check SPI speed in software (may need to be reduced)

### Button Issues
- **Buttons always pressed:** Check pull-up configuration and wiring
- **Buttons not responding:** Verify GND connection and pin numbers

### Joystick Issues
- **Incorrect readings:** Check A0/A1 connections
- **Inverted axes:** Swap X/Y in software or invert values
- **Drifting center:** Calibrate neutral position in software

## Safety

- Do not exceed 5V on Arduino Nano pins
- Avoid connecting/disconnecting components while powered
- Use appropriate current-limited power supply
- Ensure proper grounding for all components
