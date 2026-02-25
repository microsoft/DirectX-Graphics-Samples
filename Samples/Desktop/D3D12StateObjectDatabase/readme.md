---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: Direct3D 12 State Object Database Sample
urlFragment: d3d12-state-object-database-sample-win32
description: This sample demonstrates explicitly writing a State Object Database (SODB) with Direct3D 12.
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

# Direct3D 12 State Object Database Sample

## Sample to Write a State Object Database

This sample demonstrates explicitly writing a State Object Database (SODB) with Direct3D 12.  State Object Databases contain the API arguments for creating Pipeline State Objects and State Objects.  They are used to precompile shaders for a target adapter and application.

The explicit method for recording an SODB can be useful when translating a custom PSO storage mechanism into the standard SODB, or in other circumstances where the set of objects is known and playthrough can be avoided.

## State Object Database Capture

Alternatively, a capture method exists that will capture any Pipeline State Object or State Object encountered during a playthrough session into an SODB.  No code changes to the target application are needed to record the objects encountered in the playthrough.

### 1. Add the application to d3dconfig to apply settings

D3DConfig settings only apply to applications in this list of apps.  

```txt
d3dconfig.exe apps --add <exefilename>
```

Notes

- Full path may also be used, but be aware that applications launched through hard links or other references that may not match this path.
- Settings are applied to all instances of the matching exe.
- The add command has two dashes. “–-add”

### 2. Set the path for the SODB file output

This settings specifies the output State Object Database file.

```txt
d3dconfig.exe device pso-db-path=<filepath>.sodb
```

Notes

- All matching d3dconfig apps write to this db. Use exe apps to see the currently configured applications.

### 3. Enable SODB collection

Enable SODB collection.

```txt
d3dconfig.exe device enable-pso-db=true
```

Notes

- d3dconfig settings are only read at device creation.  Exit and restart the game to ensure settings apply.
- This setting may be set to false to temporarily stop collection.

### 4. Play through the game and exit

Pipeline State Objects and State Objects created by the D3D12 API are recorded to the database specified in step 2.

### 5. Clear D3DConfig Settings

Clear all D3Dconfig settings to stop SODB capture

```txt
d3dconfig.exe --reset
```