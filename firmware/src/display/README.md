# Display stack

Layered driver for the SSD1306 128x64 OLED over I2C. The API and per-call contracts live in
each layer's header (`inc/display/`); implementations are in `src/display/`. Bottom to top:

- **`i2c_bus`** (`i2c_bus.h`): raw I2C0 transfers (blocking + async), busy flag, IRQ step
- **`SSD1306`** (`SSD1306.h`): command set, packed framebuffer, double-buffered async flush
- **`display`** (`display.h`): memory-safe pixel/image drawing into a persistent packed canvas
- **`gfx`**: *(TODO)* graphics engine. shapes, text, bars, menus on top of `display`

The application must route `I2C0_IRQHandler()` to `i2c_irq_handler()`.
