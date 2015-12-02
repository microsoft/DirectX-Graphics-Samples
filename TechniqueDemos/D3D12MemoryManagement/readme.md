# Memory Management Sample
This is an advanced sample designed to demonstrate new memory management techniques in DirectX 12 and the new WDDMv2 driver model. DirectX 12 requires developers to explicitly manage their memory residency to ensure they do not use more than their fair share of memory on the system, in order to give other applications the ability to run in parallel.

By using tiled resources and DirectX 12's new resource model, the sample demonstrates how the user can map individual mip levels to different heaps, allowing them to manage residency at a sub-resource granularity. Applications can use this to stay within the kernel provided video memory budget, while also streaming mipmaps asynchronously with rendering operations.

## Controls and Features
### (M)ipmap overlays
Press the 'm' key to toggle mipmap overlays. The overlays on the images show which mipmaps are loaded and displayed. Each image is given an overlay and an outline. The overlay color displays the current mip level needed to render the image at full quality for the current zoom level. The outline, by contrast, shows the mip level which is currently resident by the background streaming thread.

The colors table used for mipmaps are in "rainbow order" - that is, mip 0=Red, 1=Orange, 2=Yellow, etc. 

### (S)tatistics overlays
Press the 's' key to toggle statistics overlays. The statistics overlays show some useful information for visualizing the state of the application, including a memory graph, CPU timing numbers, framerate, and glitch count. 

The memory graph shows both the application's current usage (yellow) as well as the current budget for that process (red line). A well-behaved application is defined as one whose current usage always remains under the budget (or tries its best to do so).

Under memory pressure, some applications will simply be unable to stay under the budget due to a combination of a highly constrained budget and a minimum footprint required by the application. Consuming more memory than your process budget allows can subject your process to throttling by the graphics kernel. The exact behavior when going over budget depends on a number of factors, such as whether or not a non-local video memory budget exists, and the priority of your application (e.g. DWM and foreground applications are prioritized over background applications). 

### Budget overrides (+/-)
Press the '+' and '-' keys to adjust the budget in 128 MB chunks. Zero, which is the default value, means to use the kernel provided budget value.

Although the memory graph in the statistics overlay is useful for visualizing the current state of the application, it can be hard to determine if the application is behaving correctly if the budget never changes. The graphics kernel will periodically issue new budgets to the process over time based on total system-wide memory pressure, but there is currently no way to control the budget assigned to the process from user mode. This sample simulates this behavior by clamping the value returned by the graphics kernel to a maximum value, allowing the application to behave under the impression that its memory budget is more restricted than it really is. The application can use this override to stress trimming and prefetching behaviors.

### Camera Management (d/c)
The sample uses the user's camera as a means to determine what should be trimmed or prefetched. For example, images closer to the camera are prioritized higher than images further away. Under normal conditions, this is quite difficult to visualize - after all, how can you see the paging engine prefetching textures nearby (but outside of) the camera view, if you cannot actually see outside of the camera?

To solve this issue, the sample provided two cameras which can be controlled independently. These two cameras decouple the memory management algorithms from the rendering.

Press the 'd' key to detach the cameras, and the 'c' key to toggle which camera is controlled by the mouse. The primary camera will always control rendering, but if detached, the secondary camera will control the residency management.

The detached camera is shown as hollow red rectangle. The hollow center is the camera view - i.e. what the user would see, if looking through that camera. The red region outside of that is the camera's prefetching region. This region is used to determine which nearby images need to be loaded. The nearby mipmaps are prefetched in this fashion to prevent texture popping caused by casual camera movement. The sample also attempts to load one mipmap higher than the current visible mipmap in the center view.

The exact decision on how to prefetch and trim is up to the application, but the sample uses the following priority scheme:

1. Page in all visible mipmaps  (i.e. mipmaps needed for rendering at the current camera zoom)
2. Page in one mipmap higher for visible mipmaps, and the currently visible mipmap levels for nearby images (e.g. those in the red region)
3. Round-robin all images and load one more mipmap until all images load mipmap 0 or we are at our budget.

Priorities 1 and 2 are designed to ensure the highest quality rendering for images the user is expected to see, while priority 3 is designed purely to prefetch as much as possible. In our sample, it was determined that mipmaps loaded during priority 3 did not have a strict ordering requirement, and the chosen mipmap may correspond to seemingly random images from the perspective of the debug camera, due to the round-robin approach.

### Toggle (v)-sync
Press the 'v' key to toggle v-sync on and off.

### Toggle (f)ullscreen mode
Press the 'f' key to toggle between fullscreen and windowed modes.
