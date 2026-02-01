import {
    createConnection,
    TextDocuments,
    ProposedFeatures,
    InitializeParams,
    TextDocumentSyncKind,
    CompletionItem,
    CompletionItemKind,
    TextDocumentPositionParams,
    Diagnostic,
    DiagnosticSeverity,
} from 'vscode-languageserver/node';

import { TextDocument } from 'vscode-languageserver-textdocument';
import { CVNParser, CVNSymbol } from './parser';
import { CVNValidator } from './validator';

// Create a connection for the server
const connection = createConnection(ProposedFeatures.all);

// Create a text document manager
const documents: TextDocuments<TextDocument> = new TextDocuments(TextDocument);

// Parser and validator instances
const parser = new CVNParser();
const validator = new CVNValidator();

// Document symbol cache
const documentSymbols = new Map<string, CVNSymbol[]>();

connection.onInitialize((params: InitializeParams) => {
    connection.console.log('CVN Language Server initializing...');
    
    return {
        capabilities: {
            textDocumentSync: TextDocumentSyncKind.Incremental,
            completionProvider: {
                resolveProvider: true,
                triggerCharacters: [' ', '.', '(', '{']
            },
            diagnosticProvider: {
                interFileDependencies: false,
                workspaceDiagnostics: false
            }
        }
    };
});

connection.onInitialized(() => {
    connection.console.log('CVN Language Server initialized!');
});

// Document change handler
documents.onDidChangeContent(change => {
    const document = change.document;
    const text = document.getText();
    
    // Parse document and cache symbols
    const symbols = parser.parse(text);
    documentSymbols.set(document.uri, symbols);
    
    // Validate and send diagnostics
    const diagnostics = validator.validate(document, symbols);
    connection.sendDiagnostics({ uri: document.uri, diagnostics });
});

// Completion handler
connection.onCompletion(
    (params: TextDocumentPositionParams): CompletionItem[] => {
        const document = documents.get(params.textDocument.uri);
        if (!document) {
            return [];
        }
        
        const text = document.getText();
        const offset = document.offsetAt(params.position);
        const line = document.getText({
            start: { line: params.position.line, character: 0 },
            end: params.position
        });
        
        const symbols = documentSymbols.get(params.textDocument.uri) || [];
        
        return getCompletions(line, text, offset, symbols, params.position.line);
    }
);

// Completion resolve handler
connection.onCompletionResolve((item: CompletionItem): CompletionItem => {
    // Add documentation to completion items
    if (item.data === 'character') {
        item.detail = 'Character Declaration';
        item.documentation = 'Defines a character with display name, color, and voice tag';
    } else if (item.data === 'asset') {
        item.detail = 'Asset Declaration';
        item.documentation = 'Defines an asset reference (bg, sprite, music, sfx, ui, text)';
    } else if (item.data === 'style') {
        item.detail = 'Style Declaration';
        item.documentation = 'Defines a reusable style with positioning and visual properties';
    } else if (item.data === 'show') {
        item.detail = 'Show Command';
        item.documentation = 'Shows a character or element on screen with specified sprite and style';
    } else if (item.data === 'say') {
        item.detail = 'Say Command';
        item.documentation = 'Character dialogue: say(Character, "text")';
    }
    
    return item;
});

function getCompletions(
    linePrefix: string,
    fullText: string,
    offset: number,
    symbols: CVNSymbol[],
    lineNumber: number
): CompletionItem[] {
    const completions: CompletionItem[] = [];
    
    // Keywords at statement level
    if (isAtStatementStart(linePrefix)) {
        const keywords = [
            { label: 'character', kind: CompletionItemKind.Keyword, insertText: 'character ${1:Name} {\n\tdisplay_name: "${2:Display Name}";\n\tname_color: 0x${3:ffffff};\n\tvoice_tag: "${4:voice}";\n}', data: 'character' },
            { label: 'asset', kind: CompletionItemKind.Keyword, insertText: 'asset ${1|bg,sprite,music,sfx,ui,text|} ${2:name} = "${3:path}";', data: 'asset' },
            { label: 'style', kind: CompletionItemKind.Keyword, insertText: 'style ${1:name} {\n\t$0\n}', data: 'style' },
            { label: 'start', kind: CompletionItemKind.Keyword, insertText: 'start {\n\t$0\n}' },
            { label: 'native', kind: CompletionItemKind.Keyword, insertText: 'native "c" {\n\t$0\n}' },
            { label: 'scene', kind: CompletionItemKind.Function, insertText: 'scene bg ${1:name};' },
            { label: 'show', kind: CompletionItemKind.Function, insertText: 'show ${1:Character} sprite ${2:sprite} as "${3:id}" use ${4:style};', data: 'show' },
            { label: 'hide', kind: CompletionItemKind.Function, insertText: 'hide "${1:id}";' },
            { label: 'say', kind: CompletionItemKind.Function, insertText: 'say(${1:Character}, "${2:text}");', data: 'say' },
            { label: 'pause', kind: CompletionItemKind.Function, insertText: 'pause ${1:1.0};' },
            { label: 'music', kind: CompletionItemKind.Function, insertText: 'music ${1|play,stop,pause|} ${2:name};' },
            { label: 'sfx', kind: CompletionItemKind.Function, insertText: 'sfx play ${1:name};' },
        ];
        
        completions.push(...keywords);
    }
    
    // Asset types after 'asset'
    if (linePrefix.match(/\basset\s+$/)) {
        ['bg', 'sprite', 'music', 'sfx', 'ui', 'text'].forEach(type => {
            completions.push({
                label: type,
                kind: CompletionItemKind.EnumMember,
                detail: `Asset type: ${type}`
            });
        });
    }
    
    // Characters after 'say(' or 'show'
    if (linePrefix.match(/\bsay\s*\(\s*$/) || linePrefix.match(/\bshow\s+$/)) {
        symbols.filter(s => s.type === 'character').forEach(char => {
            completions.push({
                label: char.name,
                kind: CompletionItemKind.Class,
                detail: `Character: ${char.name}`
            });
        });
    }
    
    // Assets after asset type keywords
    const assetMatch = linePrefix.match(/\b(bg|sprite|music|sfx|ui|text)\s+$/);
    if (assetMatch) {
        const assetType = assetMatch[1];
        symbols.filter(s => s.type === 'asset' && s.assetType === assetType).forEach(asset => {
            completions.push({
                label: asset.name,
                kind: CompletionItemKind.Value,
                detail: `${assetType}: ${asset.name}`
            });
        });
    }
    
    // Styles after 'use'
    if (linePrefix.match(/\buse\s+$/)) {
        symbols.filter(s => s.type === 'style').forEach(style => {
            completions.push({
                label: style.name,
                kind: CompletionItemKind.Struct,
                detail: `Style: ${style.name}`
            });
        });
    }
    
    // Transitions after 'with'
    if (linePrefix.match(/\bwith\s+$/)) {
        ['fade', 'dissolve', 'move', 'wipe'].forEach(trans => {
            completions.push({
                label: trans,
                kind: CompletionItemKind.Function,
                insertText: `${trans}(\${1:0.5})`
            });
        });
    }
    
    // Properties inside blocks
    if (isInsideBlock(fullText, offset)) {
        const blockType = getBlockType(fullText, offset);
        
        if (blockType === 'character') {
            ['display_name', 'name_color', 'voice_tag'].forEach(prop => {
                completions.push({
                    label: prop,
                    kind: CompletionItemKind.Property,
                    insertText: `${prop}: \${1};`
                });
            });
        } else if (blockType === 'style' || blockType === 'show') {
            ['layer', 'x', 'y', 'anchor_x', 'anchor_y', 'scale', 'alpha', 'rotation', 'tint', 'z', 'flip_x', 'flip_y'].forEach(prop => {
                completions.push({
                    label: prop,
                    kind: CompletionItemKind.Property,
                    insertText: `${prop}: \${1};`
                });
            });
        }
    }
    
    return completions;
}

function isAtStatementStart(linePrefix: string): boolean {
    const trimmed = linePrefix.trim();
    return trimmed === '' || trimmed.endsWith('{') || trimmed.endsWith(';');
}

function isInsideBlock(text: string, offset: number): boolean {
    let braceCount = 0;
    for (let i = 0; i < offset; i++) {
        if (text[i] === '{') braceCount++;
        if (text[i] === '}') braceCount--;
    }
    return braceCount > 0;
}

function getBlockType(text: string, offset: number): string | null {
    // Find the nearest opening brace before offset
    let bracePos = -1;
    let braceCount = 0;
    
    for (let i = offset - 1; i >= 0; i--) {
        if (text[i] === '}') braceCount++;
        if (text[i] === '{') {
            if (braceCount === 0) {
                bracePos = i;
                break;
            }
            braceCount--;
        }
    }
    
    if (bracePos === -1) return null;
    
    // Look backwards from the brace to find the declaration keyword
    const beforeBrace = text.substring(Math.max(0, bracePos - 100), bracePos);
    
    if (beforeBrace.match(/\bcharacter\s+\w+\s*$/)) return 'character';
    if (beforeBrace.match(/\bstyle\s+\w+\s*$/)) return 'style';
    if (beforeBrace.match(/\bshow\s+.*$/)) return 'show';
    if (beforeBrace.match(/\bstart\s*$/)) return 'start';
    
    return null;
}

// Listen on the connection
documents.listen(connection);
connection.listen();
