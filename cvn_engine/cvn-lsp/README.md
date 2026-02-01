# CVN Language Support for VSCode

Language support extension for CVN (C-style Visual Novel) scripting language.

## Features

- **Syntax Highlighting**: Full syntax highlighting for CVN language constructs
  - Character, asset, and style declarations
  - Commands: scene, show, hide, say, pause, music, sfx
  - Native C code blocks
  - Color literals (hex format)
  - String interpolation

- **IntelliSense Autocompletion**:
  - Declaration keywords (character, asset, style, start)
  - Asset types (bg, sprite, music, sfx, ui, text)
  - Commands and transitions
  - Character and asset name completion from document
  - Style property completion
  - Context-aware suggestions

- **Snippets**: Quick templates for common CVN patterns
  - `char` - Character declaration
  - `assetbg`, `assetsprite`, `assetmusic`, `assetsfx` - Asset declarations
  - `style` - Style declaration
  - `show`, `hide` - Show/hide commands
  - `say` - Dialogue
  - And more...

- **Editor Features**:
  - Auto-closing brackets and quotes
  - Comment toggling (// and /* */)
  - Code folding
  - Proper indentation

## Installation

### From Source

1. Clone this repository
2. Navigate to the extension directory:
   ```bash
   cd cvn/lsp
   ```
3. Install dependencies:
   ```bash
   npm install
   ```
4. Compile the extension:
   ```bash
   npm run compile
   ```
5. Press F5 in VSCode to launch the Extension Development Host

### From VSIX (once packaged)

1. Build the package:
   ```bash
   npm run package
   ```
2. Install the .vsix file:
   ```bash
   code --install-extension cvn-0.0.1.vsix
   ```

## Usage

1. Create a file with `.cvn` extension
2. Start typing CVN code - syntax highlighting activates automatically
3. Use IntelliSense (Ctrl+Space) for autocompletion suggestions
4. Type snippet prefixes and press Tab to expand

### Example

```cvn
character Chocola {
    display_name: "Chocola";
    name_color: 0xff69b4;
    voice_tag: "chocola";
}

asset bg cafe_day = "bg/cafe_day.png";
asset sprite chocola_happy = "ch/chocola_happy.png";

style actor_center {
    layer: "actors";
    x: 0.5; y: 0.82;
    anchor_x: 0.5; anchor_y: 1.0;
    scale: 1.0;
    alpha: 1.0;
    z: 100;
}

start {
    scene bg cafe_day;
    with fade(0.5);
    
    show Chocola sprite chocola_happy as "chocola" use actor_center;
    
    say(Chocola, "Welcome to the café!");
}
```

## Language Reference

### Declarations

- `character Name { ... }` - Define a character
- `asset type name = "path"` - Define an asset
- `style name { ... }` - Define a reusable style
- `start { ... }` - Entry point

### Commands

- `scene bg name` - Set background
- `show Character sprite name as "id" use style { ... }` - Show character/sprite
- `hide "id"` - Hide element
- `say(Character, "text")` - Dialogue
- `music play name [loop] [fade_in duration]` - Play music
- `sfx play name` - Play sound effect
- `pause duration` - Pause
- `with transition(duration)` - Apply transition
- `native "c" { ... }` - Embed C code

## Development

```bash
# Install dependencies
npm install

# Compile TypeScript
npm run compile

# Watch mode
npm run watch

# Lint
npm run lint

# Package
npm run package
```

## License

MIT

## Repository

https://github.com/pinkqween/CVN
