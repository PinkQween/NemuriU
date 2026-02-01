# CVN Language Support for Neovim

This guide explains how to set up CVN language support in Neovim.

## Prerequisites

- Neovim >= 0.8.0
- [nvim-lspconfig](https://github.com/neovim/nvim-lspconfig)
- [mason.nvim](https://github.com/williamboman/mason.nvim) (optional, for LSP management)

## Installation

### 1. Install the VSCode Extension (for syntax files)

The VSCode extension includes the TextMate grammar that can be used by Neovim with tree-sitter or syntax highlighting plugins.

### 2. Set up File Type Detection

Add to your Neovim config (`~/.config/nvim/init.lua` or `~/.config/nvim/lua/filetype.lua`):

```lua
vim.filetype.add({
  extension = {
    cvn = 'cvn',
  },
})
```

### 3. Basic Syntax Highlighting (without LSP)

Using vim syntax highlighting:

Create `~/.config/nvim/after/syntax/cvn.vim`:

```vim
" CVN syntax highlighting
if exists("b:current_syntax")
  finish
endif

" Keywords
syn keyword cvnDeclaration character asset style start native
syn keyword cvnAssetType bg sprite music sfx ui text
syn keyword cvnCommand scene show hide say pause music sfx with use as
syn keyword cvnKeyword loop fade_in fade_out
syn keyword cvnBoolean true false

" Comments
syn region cvnCommentBlock start="/\*" end="\*/"
syn match cvnCommentLine "//.*$"

" Strings
syn region cvnString start='"' end='"' contains=cvnInterpolation
syn region cvnInterpolation start='{' end='}' contained

" Numbers
syn match cvnNumber "\<\d\+\>"
syn match cvnFloat "\<\d\+\.\d\+\>"
syn match cvnHexColor "\<0x[0-9a-fA-F]\{6\}\>"

" Highlight groups
hi def link cvnDeclaration Keyword
hi def link cvnAssetType Type
hi def link cvnCommand Function
hi def link cvnKeyword Keyword
hi def link cvnBoolean Boolean
hi def link cvnCommentBlock Comment
hi def link cvnCommentLine Comment
hi def link cvnString String
hi def link cvnInterpolation Special
hi def link cvnNumber Number
hi def link cvnFloat Float
hi def link cvnHexColor Constant

let b:current_syntax = "cvn"
```

### 4. Enhanced Highlighting with Tree-sitter (Recommended)

If you use [nvim-treesitter](https://github.com/nvim-treesitter/nvim-treesitter), you can create a custom parser for CVN.

For now, you can use the generic highlighting with the basic syntax file above.

### 5. LSP Setup (Future)

When a dedicated LSP server is available, add to your LSP config:

```lua
local lspconfig = require('lspconfig')
local configs = require('lspconfig.configs')

-- Define CVN LSP if not already defined
if not configs.cvn_lsp then
  configs.cvn_lsp = {
    default_config = {
      cmd = {'cvn-lsp', '--stdio'},  -- Replace with actual LSP server command
      filetypes = {'cvn'},
      root_dir = lspconfig.util.root_pattern('.git', 'main.cvn'),
      settings = {},
    },
  }
end

-- Setup CVN LSP
lspconfig.cvn_lsp.setup({
  on_attach = function(client, bufnr)
    -- Your on_attach function
  end,
  capabilities = require('cmp_nvim_lsp').default_capabilities(),
})
```

### 6. Completion with nvim-cmp

For basic completion without LSP, you can use snippet completion:

```lua
local cmp = require('cmp')

cmp.setup.filetype('cvn', {
  sources = {
    { name = 'buffer' },
    { name = 'path' },
  }
})
```

## Quick Snippets for LuaSnip

If you use [LuaSnip](https://github.com/L3MON4D3/LuaSnip):

Create `~/.config/nvim/snippets/cvn.lua`:

```lua
local ls = require("luasnip")
local s = ls.snippet
local t = ls.text_node
local i = ls.insert_node

return {
  s("char", {
    t({"character "}), i(1, "CharacterName"), t({" {", "\tdisplay_name: \""}),
    i(2, "Display Name"), t({"\";", "\tname_color: 0x"}), i(3, "ffffff"),
    t({";", "\tvoice_tag: \""}), i(4, "voice"), t({"\";", "}"}),
  }),
  
  s("assetbg", {
    t("asset bg "), i(1, "name"), t(" = \""), i(2, "bg/asset.png"), t("\";"),
  }),
  
  s("say", {
    t("say("), i(1, "Character"), t(", \""), i(2, "dialogue"), t("\");"),
  }),
  
  s("show", {
    t({"show "}), i(1, "Character"), t(" sprite "), i(2, "sprite_name"),
    t(" as \""), i(3, "id"), t("\" use "), i(4, "style"), t(";"),
  }),
}
```

Then load it in your LuaSnip config:

```lua
require("luasnip.loaders.from_lua").load({paths = "~/.config/nvim/snippets"})
```

## Testing

1. Open a `.cvn` file
2. Check that syntax highlighting works
3. Test autocompletion (if configured)

## Future Enhancements

- Dedicated Tree-sitter parser for CVN
- Full LSP server with diagnostics
- Advanced completion and go-to-definition
- Inline documentation

## Resources

- [CVN Repository](https://github.com/pinkqween/CVN)
- [Sample CVN File](../samples/main.cvn)
