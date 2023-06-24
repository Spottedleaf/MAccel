# MAccel
Mouse acceleration program based off povohat's program 'interaccel', found here: http://mouseaccel.blogspot.com/p/blog-table-of-contents.html

This program requires the following driver to be installed:
https://github.com/oblitum/Interception

Any file in the interception folder is a part of the Interception project and not owned by me.

# Deprecation notice
This program has been deprecated in favor of [RawAccel](https://github.com/a1xd/rawaccel/blob/master/doc/Guide.md#installation).
Anticheats seem to hate the Interception driver, so it is recommended to use RawAccel instead.

To convert settings to RawAccel (from RawAccel defaults):
This conversion only works for `assume-constant-rate=1`. There is no correct conversion for `assume-constant-rate=0`.
For converting `assume-constant-rate=0`, simply use `updates-per-second` as the actual mouse polling rate.

- Check "By Component" in the "Anistropy" tab.
- Set "Sens Multiplier" to `x-post-scale` * `x-multiplier`
- Set "Y/X Ratio" to (`y-post-scale` * `y-multiplier`) / (`x-post-scale` * `x-multiplier`)
- Set X and Y mode to "Classic"
- Uncheck Gain for both X and Y
- Set for X and Y: Input Offset = 0.0, Cap Type = Output, Cap. Output = 0.0
- Set Acceleration for X and Y separately to be = `pre-scale` * `multiplier` / (1000 / `updates-per-second`).
- Set Power for X and Y separately to be `power` + 1.

In the settings.json, set Polling rate to 1000Hz and DPI to 0. This will completely disable the additional scaling
done by RawAccel for these values. 
