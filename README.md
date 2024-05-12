# Mjoelnir

## Vulkan Game Engine

### Building

`cmake . -Wdeprecated -B build -G "Ninja Multi-Config" && cmake --build build --config Release|Debug`  

### Debugging

When building using the debug configuration, NDEBUG will not be set, so check for that.  

```c
#ifndef NDEBUG
// code
#endif
```
