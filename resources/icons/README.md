# Toolbar and control icons

The toolbar and the output buttons use Font Awesome Free **6.7.2** by
Fonticons, Inc., Copyright 2024, under **CC BY 4.0**. The original SVGs and the
license are in `fontawesome/`. Source:
https://github.com/FortAwesome/Font-Awesome/tree/6.7.2/svgs and
https://fontawesome.com/license/free.

`tools/make-tool-icons.py` (the pipeline of airShot's) converts the SVGs into
Haiku Vector Icon Format data in `src/ui/IconData.h`, centred on a 64-unit
canvas. AirPins renders them at the font's size and tints them with the
control text colour, so normal builds need neither Python nor a Font Awesome
installation.

| Use | Font Awesome icon |
| --- | --- |
| Open configuration | solid folder-open |
| Save configuration | solid floppy-disk |
| Board pin layout | solid table-columns |
| BCM pin layout | solid list-ol |
| Compact layout | solid compress |
| Set all pins to Unused | solid rotate-left |
| Device details | solid microchip |
| Output high / low | solid toggle-on / toggle-off |
| (spare) | solid circle-info |

The application icon is not Font Awesome: it is `resources/airpins-icon.hvif`,
drawn by `tools/make-icon.py` after `resources/branding/AirPins.png`.
