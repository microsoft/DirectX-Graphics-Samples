# Variable Rate Shading sample
![Variable Rate Shading GUI](src/Screenshot.PNG)

This sample illustrates how to use Variable Rate Shading (VRS) to improve application performance. VRS adds the concept of subsampling, where shading can be computed at a level coarser than a pixel. For example, a group of pixels can be shaded as a single unit and the result is then broadcast to all samples in the group. This is great for areas of the image where extra detail doesn't help - such as those behind HUD elements, transparencies, blurs (depth-of-field, motion, etc.), and optical distortions due to VR optics.

#### Recommended scenarios to try
Hit SPACE to stop the light animating and then use the [F1-F5] keys to toggle between presets for Shading Rates. Can you spot the visual difference between F1 and F2? Try experimenting with the various controls to find an acceptable balance between degraded-visuals and performance.

### Controls
* SPACE: Toggles light animation.
* ALT + ENTER: Toggles between windowed and fullscreen modes.
* [+/-]: Increments/decrements the glass refraction scale.
* CTRL + [+/-]: Increments/decrements the fog density.

* [F1-F5]: Selects a **preset** for multiple Shading Rates.
* [1-7]: Selects Shading Rate for the Refraction pass.
* SHIFT + [1-7]: Selects the Shading Rate for the Scene pass.
* CTRL + [1-7]: Selects the Shading Rate for the Postprocess pass.

## Requirements
* Windows 10 18312 or higher (available via [Windows Insider Program](https://insider.windows.com/en-us/))
* [Visual Studio 2017](https://www.visualstudio.com/) with the Windows 10 18312 SDK
* For drivers, please contact the hardware vendor you're working with. We'll update this readme with links to public drivers once they are available