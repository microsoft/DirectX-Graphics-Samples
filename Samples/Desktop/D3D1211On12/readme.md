---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: 11 on 12 Sample
urlFragment: 11-on-12-sample-win32
description: Demonstrates how to use Direct3D 11-based rendering in combination with Direct3D 12.
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

# 11 on 12 Sample

![11On12 GUI](src/D3D1211On12.png)

This sample demonstrates how to use Direct3D 11 based rendering in combination with Direct3D 12 by sharing devices, command queues and resources between the 2 API versions. The sample illustrates this by using Direct2D to render a UI over Direct3D 12 content. This ability to share resources between runtimes makes porting apps/engines to 12 simpler as entire 11-based components, such as a UI system, can remain virtually untouched.
