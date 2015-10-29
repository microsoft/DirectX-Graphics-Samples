# nBody Async Sample
## Overview

This is a slightly modified version of the Microsoft ``D3D12nBodyGravity`` sample (which can be found here: https://github.com/Microsoft/DirectX-Graphics-Samples/tree/master/Samples/D3D12nBodyGravity).

The main differences are:

* The CPU side threading has been removed. The sample now leverages async compute as follows: Once the n-body simulation finishes, the results are copied and the next simulation step is immediately kicked off. This allows the simulation to overlap with the rendering. The original sample would synchronize such that either the graphics or the compute queue was in use. Now, the compute queue is kept busy 100% with the n-body simulation, and the rendering is running concurrently.
* The sample has slightly higher graphics load by using larger particles (increasing the load on the blend units) and using a noise function to modify the color of each particle (increasing the compute load)
* The sample is now queuing up to **4** frames, instead of **2**, making sure that the GPU is always filled with work.
* The sample has been instrumented to measure the frame timing.
* The number of particles and the block size has been slightly increased to increase the amount of computation. It uses now 16384 particles and work groups of 256 threads each, resulting in 64 fully filled invocations. The original sample uses 10000 particles and work groups of 128 each.

## System requirements

* Windows 10, 64-bit
* A D3D12 compatible GPU
* Screen resolution must be 1920x1080 or higher. The sample will run in 1600×1050 pixel resolution.

## Notes

* Multiple GPUs are not supported.
* The ``NoAsync`` version will schedule all work onto the graphics queue and use fewer synchronization primitives. The ``Async`` version will schedule all the simulation load onto the ``compute`` queue and all graphics onto the ``graphics`` queue, with synchronization between the queues.
* To enable/disable asynchronous compute, change `AsynchronousComputeEnabled` in `D3D12nBodyGravity.h`.

## Performance notes

This sample shows how to take advantage of multiple queues and is has not been tuned to maximize the overlap. In particular, the rendering is unchanged and rather cheap, which limits the amount of overlap where both the graphics queue and the compute queue can be busy. The expected speed-up from the asynchronous version is around 10-15%.

A simple way to increase the graphics load is to change the `g_fParticleRad` in the shader to something like 80.

## Changelog

### 1.0.0

* Initial release
