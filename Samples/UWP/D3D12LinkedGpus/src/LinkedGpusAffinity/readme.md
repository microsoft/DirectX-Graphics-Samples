# Linked GPUs using the Affinity Layer Sample
Demonstrate how to use the D3DX12AffinityLayer library to render a scene on multiple GPUs using alternate frame rendering (AFR). Each GPU node available to the application will render a frame in round-robin fashion.

The rendering consists of two passes:
  * The scene pass draws a number of triangles of different colors. The triangles animate across the screen as the frames progress.
  * The post-processing pass takes the render targets of the previous 6 scene passes and blends them together to produce a motion blur effect.

When this sample runs on a system with linked GPUs, each node will take turns rendering the scene pass and share that frame's resulting render target with the other linked nodes.

Diff this project with the SingleGpu project to get an idea of the changes required to integrate the affinity layer library into your engine.

### Controls
SPACE bar - toggles between fullscreen and windowed modes.
LEFT/RIGHT arrow keys - toggles the sync interval for Present between 0 and 1.
UP/DOWN arrow keys - increases/decreases a simulated workload on the GPU.