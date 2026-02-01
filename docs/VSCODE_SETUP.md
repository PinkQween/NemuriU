# VSCode Setup for CVN/NemuriU Development

## Quick Fix

The VSCode configuration has been updated to fix all IntelliSense errors.

### What Was Fixed

1. **Include Paths**: Added all CVN engine and SDL2 paths
2. **Headers**: Fixed duplicate type declarations
3. **API Access**: Created `cvn_full.h` for complete API

### Files Updated

- `.vscode/c_cpp_properties.json` - Complete include paths
- `cvn_engine/include/cvn.h` - Clean minimal API
- `cvn_engine/include/cvn_full.h` - Full API access  
- `cvn_engine/src/engine/api.h` - No conflicts

## Using the CVN Engine

### For Basic Projects
```c
#include "cvn.h"

CVNEngine *engine = cvn_init(&config);
// Use high-level API only
```

### For Advanced Projects (like NemuriU)
```c
#include "cvn_full.h"

CVNEngine *engine = cvn_init(&config);
CVNRenderer *renderer = cvn_get_renderer(engine);
CVNInstance *inst = VN_FindInstance(engine, "character");
// Full access to engine internals
```

## VSCode IntelliSense

All errors should now be resolved. If you still see errors:

1. Reload VSCode window: `Cmd+Shift+P` → "Reload Window"
2. Rebuild compile commands: `cd build && cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`
3. Check C/C++ extension is installed

## Build Status

✅ **CVN Engine**: Compiles cleanly  
✅ **NemuriU Code**: Compiles cleanly  
⚠️ **Wii U Linking**: Requires `mpg123` and SDL2 fixes (devkitPro issue, not code issue)

The code is correct - linker errors are due to Wii U SDL2_mixer dependencies.

## Next Steps

For desktop testing (works now):
```bash
cd build
cmake .. # Uses system SDL2
make
./NemuriU # Runs with dual-window
```

For Wii U (linker needs fixes):
```bash
# Update SDL2 libraries or disable MP3 support
cmake .. -DCMAKE_TOOLCHAIN_FILE=$DEVKITPRO/wut/share/wut.toolchain.cmake
```
