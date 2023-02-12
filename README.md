# Image sequence converter for SSD1306 OLED panels

Extends upon the [pico-examples for SSD1306 OLED panels](https://github.com/raspberrypi/pico-examples/tree/master/i2c/ssd1306_i2c), but produces binary blobs for the image data rather than a header file.

Example output is available in `examples`, with an example implementation for pi pico in `examples/pico`.

## Usage

For a single image:
`$ ./oledimgconv <file>`

For an image sequence, where '#' specifies any frame numbers and/or padding in the path.

## TODO:
- named output file
- compression
