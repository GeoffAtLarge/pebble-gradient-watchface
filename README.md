# pebble-gradient-watchface
An analog watchface for the Pebble Time 2 that tells time with two overlapping gradients, inspired by gradient-dial designer watches.

## Telling time
- Each hand is a translucent disc with a conic gradient: 100% opaque at the hand, fading logarithmically to 0% over 225 degrees
- The hour disc fades clockwise from the hour hand; the minute disc fades counter-clockwise from the minute hand and is layered over the hour disc
- The sharp edges where a disc meets its own hand, and the way the two layers combine, change continuously through the day
- The hour disc always stays inside an inner area two thirds the size of the screen; the minute disc extends past it to the screen edge

## Settings
- Hour and minute colors, picked from a grid of swatches (including black and other dark tones)
- Gradient: logarithmic (stays dark longer) or linear
- Hour area shape: circle or rectangle, with an optional outline
- Rectangular watches only: contain the minute gradient in a circle as wide as the screen (the hour area then becomes a circle or a square), with an optional outline

## Notes
The gradients are drawn per pixel into the framebuffer with ordered dithering, so the Pebble's 4-levels-per-channel palette still produces smooth fades. The face redraws once a minute.
