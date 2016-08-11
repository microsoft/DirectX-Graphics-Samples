# Fullscreen sample
This sample illustrates how to handle fullscreen <-> windowed transitions and window resizing in DirectX 12.

### Controls
SPACE bar - toggles between windowed and fullscreen modes.
LEFT/RIGHT arrow keys - changes the resolution of the scene.

### Optional Features
This sample has been updated to build against the Windows 10 Anniversary Update SDK. In this SDK a new revision of Root Signatures is available for Direct3D 12 apps to use. Root Signature 1.1 allows for apps to declare when descriptors in a descriptor heap won't change or the data descriptors point to won't change.  This allows the option for drivers to make optimizations that might be possible knowing that something (like a descriptor or the memory it points to) is static for some period of time.