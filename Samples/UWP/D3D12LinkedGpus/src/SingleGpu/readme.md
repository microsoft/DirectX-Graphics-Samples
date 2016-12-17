# Single GPU Sample
This project is provided as a baseline for the two linked GPU projects.

The rendering consists of two passes:
  * The scene pass draws a number of triangles of different colors. The triangles animate across the screen as the frames progress.
  * The post-processing pass takes the render targets of the previous 6 scene passes and blends them together to produce a motion blur effect.

Diff this project with the LinkedGpusAffinity project to get an idea of the changes required to integrate the affinity layer library into your engine.

### Controls
SPACE bar - toggles between fullscreen and windowed modes.
LEFT/RIGHT arrow keys - toggles the sync interval for Present between 0 and 1.
UP/DOWN arrow keys - increases/decreases a simulated workload on the GPU.