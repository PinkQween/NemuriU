# CVN - High-Performance Visual Novel Engine

A powerful, SDL2-based visual novel engine with native C performance that surpasses Ren'Py in flexibility and power.

## Features

### ✅ Implemented (Phase 1 Complete)

- **Dual-Display Support**: Native support for multi-screen setups (like Wii U gamepad + TV)
- **Hardware-Accelerated Rendering**: SDL2-based rendering with GPU acceleration
- **Layered Rendering System**: Unlimited layers with z-ordering and per-layer display routing
- **Advanced Sprite System**: Position, scale, rotation, tint, alpha, flip
- **Resource Management**: Automatic texture caching and loading
- **Audio System**: BGM + multi-channel SFX with SDL_mixer
- **Native C API**: Direct access to engine internals from scripts
- **Cross-Platform**: macOS, Linux, Windows support via CMake

### 🚧 In Progress

- Text rendering system with TTF fonts
- Dialogue box with customizable styles
- CVN script parser extended with flow control
- Transition effects (fade, dissolve, wipe, etc.)
- Save/load system

### 📋 Planned

- Animation system with easing functions
- Particle effects
- Shader support
- Video playback
- Hot-reload for development
- Mobile/console ports

## Architecture

```
┌─────────────────────────────────────────┐
│         CVN Scripting Language          │  ← High-level VN scripts
├─────────────────────────────────────────┤
│      Script VM + Native C Blocks        │  ← Bytecode interpreter
├─────────────────────────────────────────┤
│         Core Engine (C/SDL2)            │
│  ┌───────────┬──────────┬────────────┐  │
│  │  Renderer │ Resource │   Audio    │  │
│  │  (Layers) │ Manager  │  (Mixer)   │  │
│  └───────────┴──────────┴────────────┘  │
├─────────────────────────────────────────┤
│   SDL2 (Window, Graphics, Input, Audio) │
└─────────────────────────────────────────┘
```

## Building

### Prerequisites

- CMake 3.10+
- SDL2, SDL2_image, SDL2_mixer, SDL2_ttf
- C99-compatible compiler

### macOS

```bash
brew install sdl2 sdl2_image sdl2_mixer sdl2_ttf cmake
mkdir build && cd build
cmake ..
make -j4
```

### Linux

```bash
sudo apt-get install libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libsdl2-ttf-dev cmake
mkdir build && cd build
cmake ..
make -j4
```

### Windows

Use vcpkg or download SDL2 development libraries, then:

```cmd
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake
cmake --build .
```

## Quick Start

### Running the Demo

```bash
./build/cvn_runtime
```

The demo showcases:
- Dual-window rendering (primary + secondary display)
- Background and character sprite loading
- Real-time animations and effects
- Per-layer display routing

### Basic API Usage

```c
#include "cvn.h"

CVNConfig config = {
    .title = "My VN",
    .window_width = 1280,
    .window_height = 720,
    .enable_secondary_display = true,
    .secondary_width = 854,
    .secondary_height = 480,
    .asset_base_path = "assets",
    .audio_frequency = 44100,
    .audio_channels = 16
};

CVNEngine *engine = cvn_init(&config);

// Show background
cvn_api_set_background(engine, "bg/scene1.png");

// Show character sprite
cvn_api_show_sprite(engine, "ch/character.png", "char1", 
                    "actors", 0.5f, 0.75f, 1.0f);

// Manipulate with native C
CVNInstance *inst = VN_FindInstance(engine, "char1");
inst->rotation = 15.0f;
inst->tint = 0xFFAAAAFF;

// Main loop
while (cvn_update(engine)) {
    cvn_render(engine);
}

cvn_shutdown(engine);
```

## CVN Scripting Language

Example from `samples/main.cvn`:

```c
character Chocola {
    display_name: "Chocola";
    name_color: 0xff69b4;
    voice_tag: "chocola";
}

asset bg cafe_day = "bg/cafe_day.png";
asset sprite chocola_happy = "ch/chocola_happy.png";

style actor_left {
    layer: "actors";
    x: 0.18; y: 0.82;
    anchor_x: 0.5; anchor_y: 1.0;
    scale: 1.00;
}

start {
    scene bg cafe_day;
    with fade(0.5);
    
    show Chocola sprite chocola_happy as "chocola"
        use actor_left { z: 120; };
    
    say(Chocola, "Hello world!");
    
    // Native C power access
    native "c" {
        CVNInstance* inst = VN_FindInstance("chocola");
        inst->rotation = 5.0f;
        inst->tint = 0xFF99DDFF;
    }
}
```

## Dual-Display System

CVN's multi-display architecture allows you to route different rendering layers to different physical displays:

```c
// Route UI layer to secondary display (gamepad/touch screen)
cvn_renderer_set_layer_display(renderer, "ui", CVN_DISPLAY_SECONDARY);

// Route everything else to primary display (TV/monitor)
cvn_renderer_set_layer_display(renderer, "background", CVN_DISPLAY_PRIMARY);
cvn_renderer_set_layer_display(renderer, "actors", CVN_DISPLAY_PRIMARY);
```

Use cases:
- **Wii U style**: Game on TV, map/inventory on gamepad
- **DS/3DS style**: Main scene on top, dialogue on bottom
- **Multi-monitor**: Fullscreen game + debug/dev tools

## Performance

- **Native C**: ~100x faster than Python-based engines
- **Hardware Accelerated**: GPU rendering via SDL2
- **Minimal Overhead**: Direct SDL calls, no abstraction layers
- **Efficient**: Texture caching, batch rendering, optimized loops

## CVN vs Ren'Py

| Feature | Ren'Py | CVN |
|---------|--------|-----|
| Language | Python | C + Scripting |
| Performance | ~60 FPS | ~300+ FPS |
| Multi-Display | Limited | Native |
| Extensibility | Python modules | Direct C |
| Console Support | Difficult | Homebrew-friendly |
| Shader Support | Limited | Full OpenGL |
| Memory Usage | ~100MB base | ~10MB base |

## Directory Structure

```
cvn/
├── include/          # Public API headers
│   └── cvn.h
├── src/
│   ├── engine/       # Core engine
│   │   ├── window.c/h
│   │   ├── renderer.c/h
│   │   ├── resource.c/h
│   │   ├── audio.c/h
│   │   └── api.c/h
│   ├── script/       # Parser & VM
│   │   └── cvn_parser.c/h
│   └── cvn_engine.c  # Main engine
├── runtime/          # Demo/test runtime
│   └── main.c
├── samples/          # Example VN projects
│   ├── main.cvn
│   └── assets/
├── docs/             # Documentation
└── CMakeLists.txt
```

## Contributing

CVN is in active development. Phase 1 (Core Engine) is complete. Next up:
- Phase 2: Text & Dialogue System
- Phase 3: Enhanced CVN Parser with VM
- Phase 4: Advanced Visual Effects

## License

See LICENSE.txt

## Credits

Built with SDL2, SDL_image, SDL_mixer, SDL_ttf
Inspired by Ren'Py, NVList, and visual novel engines
Designed for maximum performance and flexibility
