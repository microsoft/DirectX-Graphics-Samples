# Linked GPU Sample
This sample demonstrates how to use two linked homogeneous (i.e. CrossFire/SLI) GPUs to achieve greater performance than possible with only one GPU.  In particular, this sample shows the usage of a technique called Alternate Frame Rendering (AFR) which essentially allows you to use each GPU to render every other frame allowing a theoretical maximum performance of 2 times that of only one GPU.  This is of course only theoretical and in practicality, there are things like inter-frame dependencies (things rendered on one GPU required by the other GPU for rendering the next frame) that add some overhead that doesn't otherwise exist in a single GPU case.  Things like this limit the maximum benefit two GPUs can provide you over one.

Linked GPUs is what most people currently think of when someone mentions 'MultiGPU' and this sample shows how to utilize both GPUs using explicit MultiGPU.  Most importantly, it shows how the app has full explicit control over the GPU hardware through the API (eg. work submission, synchronization, memory management, etc. can be controlled explicitly for each GPU independently).

## Solution structure
There are three projects in this sample's Visual Studio solution:
  * **SingleGpu** - a reference project written with one GPU in mind
  * **LinkedGpusAffinity** - an upgrade of the SingleGpu project incorporating the D3DX12AffinityLayer library
  * **LinkedGpus** - an upgrade of the SingleGpu project showing raw usage of the NodeMask API

We included the SingleGpu project in the solution so that you can diff it against the LinkedGpusAffinity project and get an idea of what it's like to integrate MultiGPU into your game using the affinity layer.  For more information on the steps to integrate the affinity layer into your project, take a look at the library's [readme.md](https://github.com/Microsoft/DirectX-Graphics-Samples/tree/master/Libraries/D3DX12AffinityLayer)

Beginners that want to enable MultiGPU in their apps should start by understanding the single GPU version and the affinity layer version as it shows simplified MultiGPU management of resource uploading, cross-GPU synchronization, etc.  The affinity layer exists independent of the sample and can also be copied into your app for usage under the provided license.

# The Affinity Layer Library

One important thing to note about this sample is that it also demonstrates the usage of a helper library called the affinity layer.  When working with two GPUs, the CPU will need to submit work to each GPU.  Certain objects (ie. command lists, command queues, etc) can be 'affinitized' allowing work to be submitted to a specific GPU.  The affinity layer also provides a simple way for the app to submit work to multiple GPUs at the same time.  The affinity layer adds a thin layer of abstraction to help you enable MultiGPU in your app.

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

The library and sample demonstrate exactly this.  The library provides the app a simple abstracted way of executing commands, uploading data, and synchronizing on one or many GPUs simultaneously. Frame pacing is something that generally needs to be tuned for each application and is currently omitted from the samples.

Another thing to note is that this library does allow for apps to switch between single GPU and multi GPU scenarios.  You don't need two code paths.  The library does naturally come with some CPU overhead.  Please profile your games and if the extra CPU usage seems to be abnormal, feel free to let us know.

# Other considerations

## What if I'm using a 3rd party library?
All resources in Direct3D 12 need to specify a Node mask to run properly targeting a specific GPU. If the 3rd party library doesn't support MultiGPU and is creating resources on your behalf, it will need to be updated to take a NodeMask parameter to be set on Direct3D 12 object creation. After that you can instantiate the library for N number of GPUs.  The affinity objects have a GetChildObject method to access underlying affinitized D3D12 resources to pass to the active instance of the library.

## I've heard terms like 'Linked' and 'Unlinked' MultiGPU; what do they mean?
Simplifying a bit, linked GPUs usually refer to multiple GPUs connected in a way which satisfies a specific OS/API contract enabling the NodeMask feature.  In conforming to this special contract, it's possible for application simultaneously use the linked GPUs more efficiently than if they were unlinked.  Though this is not necessarily always the case, linked GPUs are often found in pairs as identical cards from the same vendor sometimes even physically connected by a special cable. 

Unlinked GPUs on the other hand can be completely different in power and even vendor.  The DirectX 12 API also allows communication between unlinked GPUs though it may be slower/less efficient than what linked GPUs can manage.  As a tradeoff, unlinked GPUs open a huge number of possibilities essentially removing restrictions on video card capability, vendor, etc.  Any card of any capability should be able to work with any other card. 

This sample and library currently focus on the linked GPU case.  If you want an example of unlinked GPU usage, please refer to this sample:

https://github.com/Microsoft/DirectX-Graphics-Samples/tree/master/Samples/UWP/D3D12HeterogeneousMultiadapter

# Requirements
The samples are designed to run on a system with more than one GPU and the GPUs must be linked. The samples particularly target dual linked GPUs (typically identical cards). The samples will also run on single GPU and heterogeneous GPUs, but won't be able to take advantage of the features being highlighted.

### Optional Features
These samples have been updated to build against the Windows 10 Anniversary Update SDK. In this SDK a new revision of Root Signatures is available for Direct3D 12 apps to use. Root Signature 1.1 allows for apps to declare when descriptors in a descriptor heap won't change or the data descriptors point to won't change.  This allows the option for drivers to make optimizations that might be possible knowing that something (like a descriptor or the memory it points to) is static for some period of time.