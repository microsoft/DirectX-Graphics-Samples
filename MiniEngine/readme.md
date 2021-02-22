# MiniEngine by Team Minigraph at Microsoft

## Getting started:
* Open ModelViewer/ModelViewer_VS16.sln
* Select configuration: Debug (full validation), Profile (instrumented), Release
* Select platform
* Build and run

## Controls:
* forward/backward/strafe: left thumbstick or WASD (FPS controls)
* up/down: triggers or E/Q
* yaw/pitch: right thumbstick or mouse
* toggle slow movement: click left thumbstick or lshift
* open debug menu: back button or backspace
* navigate debug menu: dpad or arrow keys
* toggle debug menu item: A button or return
* adjust debug menu value: dpad left/right or left/right arrow keys

## glTF 2.0 Support:

Get sample assets from https://github.com/KhronosGroup/glTF-Sample-Models or make your own.

* Place asset folder underneath the ModelViewer folder
* Add to command line "-model [relative path to gltf or glb file]"
* Example:  ModelViewer.exe -model SciFiHelmet/glTF/SciFiHelmet.gltf

Notes:  Some IBL cube maps are provided for physically based rendering in the ModelViewer/Textures folder.  ModelViewer automatically detects cube maps in this folder so you can add your own.  They must follow the same naming convention for diffuse and specular maps.  You can change the active environment map in the tweak menu under the "ModelViewer" category.

DirectXMesh and DirectXTex are used for compiling content the first time it is loaded.  For example, image files will have mip maps generated, encoded to a block compressed format, and then saved as a DDS for subsequent loading.  This means that first-time loading takes a little longer.
