# The D3D12 Residency Starter Library

## What is this library?
This library is intended to be a low-integration-cost turnkey solution to managing your D3D12 heaps/committed resources to reduce the chance that you will get into an overcommitted video memory situation as well as ensure your performance remains as good as it can be when you do end up in that situation.

It implements essentially the same memory management behavior that a D3D11 app would get from the layers below the API.  In D3D12, the layers below the API cannot do this for you on account of the more flexible API, more direct access to memory, and bindless architecture.  That's where this residency library comes in.

The library should be seen as a stepping stone for D3D12 apps to get good performance when under memory pressure while leaving the door very open for apps to make optimizations that beat D3D11-style memory management algorithms.

## What happens if I don't manage memory (using this library or otherwise)?
Without this library or any other sort of memory management, there is a real and significant GPU performance hit when your app uses more memory than is available.  This is most noticeable on low memory (1GB/2GB) boards but can easily happen on 4GB cards as well (e.g. 4k, ultra settings, etc).  The OS's Video Memory Manager (VidMM) will do its absolute best to ensure your application can always make forward progress but any contingencies that VidMM has to use to keep your app running often come with a performance hit.  It is critical that applications manage their memory instead of relying on VidMM to make general case decisions which will invariably be non-optimal.

With D3D12, you have the responsibility of managing your memory but also the power to do even better than D3D11.  We built this library so that you can maintain D3D11 perf parity first leaving you with time to focus on the customizations that will make your D3D12 game better than it could have been in D3D11.

## Exactly what problem does the library try to solve?
There are two main memory related problems both of which contribute to GPU perf regressions relative to D3D11:

1. Using too much memory overall
2. Not managing the D3D12 heaps you've created (e.g. not getting rid of heaps you aren't using either by destruction or eviction).

The residency library is explicitly designed to address the second problem.

It is important to note that apps must account for all of the memory they are using and ensure that they aren't using more memory than D3D11 overall.  No amount of management can overcome a fundamental over-usage of memory.

## How does it solve the management part of the problem?
The primary piece of information that the layers below the API would have in D3D11 apps is when a texture/buffer/etc is needed for rendering.  More direct control over memory and a bindless architecture in D3D12 mean that the same information is not directly available through the API.

With this information, anything that hasn't been used in a while could be evicted (D3D11 had a concept of eviction as well, only it was hidden from the application).  Anything that was needed to render would be made resident (also hidden from the application).

From this, you can infer that in order for a D3D12 application to achieve the same behavior as a D3D11 app, the application's memory manager component would need to satisfy these responsibilities (at minimum):

* Know when D3D12 heaps need to be resident (e.g. when they're about to be used)
* Synchronize correctly so that rendering cannot happen until these heaps are made resident
* Track when heaps are in use on the GPU so that they can be evicted only when the GPU is done with them

The library provides interfaces to indicate when a resource is going to be used for rendering and handles all of the ```MakeResident```/```Evict``` calls as well as all the required synchronization.

## Alright, how do I use it?
Here are the high level steps for using the library:

1. Create a ```D3DX12Residency::ResidencyManager```
2. Create a ```D3DX12Residency::ManagedObject``` for every heap/committed resource you want the library to manage
  1. The library works by tracking/evicting/making resident the ```ManagedObject's``` underlying heap/committed resource
3. Use ```ResidencyManager::BeginTrackingObject``` to start managing each ```ManagedObject```
4. Use a ```D3DX12Residency::ResidencySet``` to track which heaps will be used in a particular command list (it will be a 1:1 relationship)
  1. It's essentially a list of ```ManagedObjects``` which will need to be resident when a command list executes
5. Use ```ResidencyManager::ExecuteCommandLists``` to execute the workload which takes a command queue, array of command lists, an array of residency sets, and a count
  1. This will execute the command lists and ensure all of the heaps/committed resources that you need to execute are resident at the right times

### FAQs

#### What exactly is Residency?
Residency correlates to whether a heap is accessible by the GPU or not.  If it is not resident, you should not be touching that resource (neither read nor write).  A resident resource can be used by the GPU.  The ```MakeResident``` and ```Evict``` calls are what controls whether a heap is resident or not.

#### What does calling ```Evict``` do?
```Evict``` is essentially a promise that you will not touch the heap until you call ```MakeResident``` again at which point you also promise to wait until ```MakeResident``` returns to start using that heap.  It means that VidMM (Video Memory Manager) can repurpose that memory (eg. for another app) if the system is under memory pressure and it needs it.

Note that when you call ```Evict```, the heap is only marked for eviction.  VidMM will try really hard not to evict it if it doesn't need to thereby making the subsequent ```MakeResident``` call (when you call it) a no-op (ie. nothing needs to happen and your heap is ready for use almost immediately).

#### What does calling ```MakeResident``` do?
```MakeResident``` is a blocking call which will bring your heap data and page table mappings back as they were before you called Evict.  You don't need to copy the data back in.

#### Why would I use ```MakeResident``` and ```Evict```?
In return for using ```MakeResident``` and ```Evict```, VidMM promises to return the data and page table mappings back to their original state before you called ```Evict``` as well as try its hardest to not actually evict anything if it nothing else needs that memory.  In many cases, this is better than destroying the heap/committed resource in which case you would need to recreate the heap/committed resource, re-copy the data into an upload heap, and execute a GPU copy before making it available again to the renderer.  The chance for ```MakeResident``` to be a no-op also improves the average case performance when compared to a destruction/recreation/repopulating technique.

### Other Library FAQs

#### What is the ```MaxLatency``` parameter in the ResidencyManager's ```Initialize``` method?
When rendering very quickly, it is possible for the renderer to get too far ahead of the library's worker thread.  The ```MaxLatency``` parameter helps to limit how far ahead it can get.  The value should essentially be the average ```NumberOfBufferedFrames * NumberOfCommandListSubmissionsPerFrame``` throughout the execution of your app.

#### The Visual Studio Graphics Debugging (VSGD) tools crash when capturing an app that uses this library
You can work around this bug by using the library's single threaded mode using the line:
```
#define RESIDENCY_SINGLE_THREADED 0
```
0 is the default; change it to 1 to force single threaded behavior to work around the issue.
