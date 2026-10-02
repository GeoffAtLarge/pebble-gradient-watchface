# pebble-gradient-watchface
An analog watchface for the Pebble Time 2 that tells time with two overlapping gradients, inspired by gradient-dial designer watches.

## Telling time
- The hour gradient sweeps clockwise from 12 o'clock to the hour hand position, fading light to dark at its leading edge
- The minute gradient does the same for the minute position, one revolution per hour
- Where the two overlap, the colors blend
- The hour gradient always stays inside an inner area two thirds the size of the screen; the minute gradient extends past it to the screen edge

## Settings
- Hour and minute colors
- Hour area shape: circle or rectangle, with an optional outline
- Rectangular watches only: contain the minute gradient in a circle as wide as the screen (the hour area then becomes a circle or a square), with an optional outline

## Notes
The gradients are drawn per pixel into the framebuffer with ordered dithering, so the Pebble's 4-levels-per-channel palette still produces smooth fades. The face redraws once a minute.
