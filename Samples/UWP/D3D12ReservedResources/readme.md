---
page_type: sample
languages:
- cpp
products:
- windows
- windows-uwp
name: Direct3D 12 reserved resources sample
urlFragment: d3d12-reserved-resources-sample-uwp
description: This sample demonstrates the use of reserved resources in DirectX 12.
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

# Direct3D 12 reserved resources sample
![ReservedResources GUI](src/D3D12ReservedResources.png)

This sample demonstrates the use of reserved resources in DirectX 12. In this sample, a quad is textured with a reserved (aka: tiled) resource containing a full mip chain. The currently visible mip is mapped and unmapped to the reserved resource on demand. By pressing the arrow keys, you can change which mip is visible. The sample also demonstrates that all the tiles in a reserved resource are not required to reside in the same heap. This functionality allows apps to persist heaps containing tiles that are likely to be used again and discard heaps that are no longer needed.

### Optional features
This sample has been updated to build against the Windows 10 Anniversary Update SDK. In this SDK a new revision of Root Signatures is available for Direct3D 12 apps to use. Root Signature 1.1 allows for apps to declare when descriptors in a descriptor heap won't change or the data descriptors point to won't change.  This allows the option for drivers to make optimizations that might be possible knowing that something (like a descriptor or the memory it points to) is static for some period of time.