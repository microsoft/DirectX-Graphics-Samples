# The D3D12 MultiGPU Starter Library

This library is intended to help apps kick start their MultiGPU implementation.  The library itself is called the affinity layer in that execution and resources can be 'affinitized' to one or more GPUs in the system.  This library should help developers quickly bring up DirectX 12 MultiGPU in their engines. 

## What do you mean by 'MultiGPU'?
MultiGPU can take on a few meanings.  The name indicates that we are referring to the use of multiple GPUs in a user's systems but does not imply how or why.  At the core, the MultiGPU features in DirectX 12 are designed to expose multiple GPUs to the app.  The power is then in the app's hands to use those GPUs to their maximum potential. 

All in all, the idea of MultiGPU in DirectX 12 is to give the app full access to the graphics hardware in the system whether it is execution resources, graphics memory, etc. 

## Exactly what problem does the library try to solve right now?
Currently, this library tries to help developers implement one of the most commonly used MultiGPU techniques, Alternate Frame Rendering (AFR) using linked GPUs.  There are a large number of other possibilities with DirectX 12's explicit MultiGPU features but AFR is one of the most widely used techniques.  The library may be expanded in the future to support other scenarios. 

There are many other possible techniques (other than AFR) for MultiGPU; some that are suitable candidates to be abstracted out into a library like the affinity layer, others that are not.  We are looking at other scenarios where the library can fit in as a (almost) drop in solution.

## What is Alternate Frame Rendering (AFR)?
AFR does exactly as the name suggests.  Frames are rendered in an alternating (i.e. round-robin) manner on each of the GPUs in the system.  If there are two GPUs, the first GPU will render every even frame and the second GPU will render every odd frame.  Since both GPUs can render in parallel, the theoretical maximum rendering frequency is twice that of a single GPU system.  This is of course assuming both GPUs are identical in power, which, in linked GPU systems, they almost always are. 

## How does the library help apps implement AFR?
One of the most common methods of achieving dual GPU AFR is to: 
  * Duplicate all non-system memory resources on both GPUs (i.e. textures, buffers, render targets, etc.) 
  * Execute the same work on both GPUs
    * The ideal case being GPU1 starts rendering when GPU0 is 50% through it's rendering pass theoretically doubling the frame rate 
  * Copy any inter-frame dependencies across the GPUs so that they are available when the next frame (ie. the other GPU) starts rendering 
    * An example of this might be a temporal AA dependency from frame N-1 needed for frame N 

Conceptually, it might look something like this:
```
   GPU0   |---Frame 0---|---Frame 2---|---Frame 4---|--- etc
   GPU1          |---Frame 1---|---Frame 3---|---Frame 5---|--- etc
```

The library provides the app a simple abstracted way of executing commands, uploading data, and synchronizing on one or many GPUs simultaneously.

Another thing to note is that this library does allow for apps to switch between single GPU and multi GPU scenarios.  You don't need two code paths.  The library does naturally come with some CPU overhead.  Please profile your games and if the extra CPU usage seems to be abnormal, feel free to let us know.

# How do I use it?
Here are the high level steps for using the library:
  1. The library is a thin wrapper around the entire D3D12 API. The expectation is to find and replace ```ID3D12``` with ```CD3DX12Affinity``` (and similar for structure names) through an entire codebase and have the code compile and work correctly on single-GPU.
    1. The exception is device and swap chain creation, where the normal API should be invoked first, followed by construction of the affinity wrapper.
    2. Note that it might be desirable to use typedefs wrapped in #ifs (e.g. GPUD3D12*) which can be toggled back and forth between the core D3D12 types and the affinity layer types.
  2. For AFR, use the ```CD3DX12Device::SwitchToNextNode()``` method on the affinity device after each successful ```Present()```.
  3. Your resource tracker will need to detect and track the following cases:
    1. Static resources which are initialized via GPU writes, e.g. textures/buffers/etc. These copies need to be issued on both GPUs. Create command lists (and allocators) with an explicit node mask indicating all GPUs.  Command lists created this way do not inherit the active node on reset.
    2. Cross-frame dependencies. At this point, you should be able to run on two GPUs with flickering (hopefully not between correct and black content). This is because contents on GPU N is intending to read contents from the previous frame (which are on GPU N-1), but the resource it's reading from has contents from N frames ago on GPU N. The engine needs to either break these dependencies, or marshal contents from frame N-1 to N using correct synchronization. Note: You will also want to ensure synchronization to cause your frames to be serialized.
    3. Resource states are independent for each GPU since the library instantiates N resources under the hood. Your tracker should transition the resource using the active state of the resource on the current node you are recording the command list on.

# Other considerations

## What if I'm using a 3rd party library?
All resources in Direct3D 12 need to specify a Node mask to run properly targeting a specific GPU. If the 3rd party library doesn't support MultiGPU and is creating resources on your behalf, it will need to be updated to take a NodeMask parameter to be set on Direct3D 12 object creation. After that you can instantiate the library for N number of GPUs.  The affinity objects have a GetChildObject method to access underlying affinitized D3D12 resources to pass to the active instance of the library.

## I've heard terms like 'Linked' and 'Unlinked' MultiGPU; what do they mean?
Simplifying a bit, linked GPUs usually refer to multiple GPUs connected in a way which satisfies a specific OS/API contract enabling the NodeMask feature.  In conforming to this special contract, it's possible for application simultaneously use the linked GPUs more efficiently than if they were unlinked.  Though this is not necessarily always the case, linked GPUs are often found in pairs as identical cards from the same vendor sometimes even physically connected by a special cable. 

Unlinked GPUs on the other hand can be completely different in power and even vendor.  The DirectX 12 API also allows communication between unlinked GPUs though it may be slower/less efficient than what linked GPUs can manage.  As a tradeoff, unlinked GPUs open a huge number of possibilities essentially removing restrictions on video card capability, vendor, etc.  Any card of any capability should be able to work with any other card. 
