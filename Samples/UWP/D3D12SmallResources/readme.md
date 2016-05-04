# Small Resources Sample
This sample demonstrates the use of small placed resources in Direct3D 12. The sample allocates a number of small textures using 4K resource alignment and shows the potential memory savings gained by using placed resources over committed and reserved resources which use 64K resource alignments. The resource type and current GPU memory usage are displayed in the window's title bar.
### Controls
SPACE bar - toggles between using placed and committed resources.