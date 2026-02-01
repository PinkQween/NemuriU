import { Diagnostic, DiagnosticSeverity } from 'vscode-languageserver/node';
import { TextDocument } from 'vscode-languageserver-textdocument';
import { CVNSymbol } from './parser';

export class CVNValidator {
    validate(document: TextDocument, symbols: CVNSymbol[]): Diagnostic[] {
        const diagnostics: Diagnostic[] = [];
        const text = document.getText();
        
        // Check for duplicate definitions
        this.checkDuplicates(symbols, diagnostics, document);
        
        // Check for undefined references
        this.checkUndefinedReferences(text, symbols, diagnostics, document);
        
        // Check syntax errors
        this.checkSyntaxErrors(text, diagnostics, document);
        
        // Check for missing required properties
        this.checkRequiredProperties(text, symbols, diagnostics, document);
        
        return diagnostics;
    }
    
    private checkDuplicates(symbols: CVNSymbol[], diagnostics: Diagnostic[], document: TextDocument) {
        const seen = new Map<string, CVNSymbol>();
        
        for (const symbol of symbols) {
            const key = `${symbol.type}:${symbol.name}`;
            const existing = seen.get(key);
            
            if (existing) {
                const range = {
                    start: document.positionAt(symbol.position),
                    end: document.positionAt(symbol.position + symbol.name.length)
                };
                
                diagnostics.push({
                    severity: DiagnosticSeverity.Error,
                    range: range,
                    message: `Duplicate ${symbol.type} '${symbol.name}' (first defined on line ${existing.line + 1})`,
                    source: 'cvn'
                });
            } else {
                seen.set(key, symbol);
            }
        }
    }
    
    private checkUndefinedReferences(text: string, symbols: CVNSymbol[], diagnostics: Diagnostic[], document: TextDocument) {
        // Check character references in say()
        const sayRegex = /\bsay\s*\(\s*(\w+)\s*,/g;
        let match;
        
        const characterNames = new Set(symbols.filter(s => s.type === 'character').map(s => s.name));
        
        while ((match = sayRegex.exec(text)) !== null) {
            const charName = match[1];
            if (!characterNames.has(charName)) {
                const range = {
                    start: document.positionAt(match.index + match[0].indexOf(charName)),
                    end: document.positionAt(match.index + match[0].indexOf(charName) + charName.length)
                };
                
                diagnostics.push({
                    severity: DiagnosticSeverity.Error,
                    range: range,
                    message: `Undefined character '${charName}'`,
                    source: 'cvn'
                });
            }
        }
        
        // Check asset references in scene, show, music, sfx
        const bgRegex = /\bscene\s+bg\s+(\w+)/g;
        const bgAssets = new Set(symbols.filter(s => s.type === 'asset' && s.assetType === 'bg').map(s => s.name));
        
        while ((match = bgRegex.exec(text)) !== null) {
            const assetName = match[1];
            if (!bgAssets.has(assetName)) {
                const range = {
                    start: document.positionAt(match.index + match[0].indexOf(assetName)),
                    end: document.positionAt(match.index + match[0].indexOf(assetName) + assetName.length)
                };
                
                diagnostics.push({
                    severity: DiagnosticSeverity.Error,
                    range: range,
                    message: `Undefined background asset '${assetName}'`,
                    source: 'cvn'
                });
            }
        }
        
        // Check style references
        const useRegex = /\buse\s+(\w+)/g;
        const styleNames = new Set(symbols.filter(s => s.type === 'style').map(s => s.name));
        
        while ((match = useRegex.exec(text)) !== null) {
            const styleName = match[1];
            if (!styleNames.has(styleName)) {
                const range = {
                    start: document.positionAt(match.index + match[0].indexOf(styleName)),
                    end: document.positionAt(match.index + match[0].indexOf(styleName) + styleName.length)
                };
                
                diagnostics.push({
                    severity: DiagnosticSeverity.Error,
                    range: range,
                    message: `Undefined style '${styleName}'`,
                    source: 'cvn'
                });
            }
        }
    }
    
    private checkSyntaxErrors(text: string, diagnostics: Diagnostic[], document: TextDocument) {
        // Check for unmatched braces
        let braceCount = 0;
        const braceStack: number[] = [];
        
        for (let i = 0; i < text.length; i++) {
            if (text[i] === '{') {
                braceCount++;
                braceStack.push(i);
            } else if (text[i] === '}') {
                braceCount--;
                if (braceCount < 0) {
                    const range = {
                        start: document.positionAt(i),
                        end: document.positionAt(i + 1)
                    };
                    
                    diagnostics.push({
                        severity: DiagnosticSeverity.Error,
                        range: range,
                        message: 'Unmatched closing brace',
                        source: 'cvn'
                    });
                    braceCount = 0; // Reset to avoid cascading errors
                } else {
                    braceStack.pop();
                }
            }
        }
        
        // Check for unclosed braces
        if (braceCount > 0 && braceStack.length > 0) {
            const lastBrace = braceStack[braceStack.length - 1];
            const range = {
                start: document.positionAt(lastBrace),
                end: document.positionAt(lastBrace + 1)
            };
            
            diagnostics.push({
                severity: DiagnosticSeverity.Error,
                range: range,
                message: 'Unclosed brace',
                source: 'cvn'
            });
        }
        
        // Check for invalid hex colors
        const hexRegex = /\b0x([0-9a-fA-F]+)\b/g;
        let match;
        
        while ((match = hexRegex.exec(text)) !== null) {
            const hexValue = match[1];
            if (hexValue.length !== 6 && hexValue.length !== 8) {
                const range = {
                    start: document.positionAt(match.index),
                    end: document.positionAt(match.index + match[0].length)
                };
                
                diagnostics.push({
                    severity: DiagnosticSeverity.Warning,
                    range: range,
                    message: `Color literal should be 6 hex digits (0xRRGGBB) or 8 for alpha (0xRRGGBBAA), got ${hexValue.length}`,
                    source: 'cvn'
                });
            }
        }
    }
    
    private checkRequiredProperties(text: string, symbols: CVNSymbol[], diagnostics: Diagnostic[], document: TextDocument) {
        // Check character blocks have required properties
        for (const symbol of symbols) {
            if (symbol.type === 'character') {
                const blockRegex = new RegExp(`character\\s+${symbol.name}\\s*\\{([^}]*)\\}`, 's');
                const match = blockRegex.exec(text);
                
                if (match) {
                    const blockContent = match[1];
                    
                    if (!blockContent.includes('display_name:')) {
                        const range = {
                            start: document.positionAt(symbol.position),
                            end: document.positionAt(symbol.position + symbol.name.length)
                        };
                        
                        diagnostics.push({
                            severity: DiagnosticSeverity.Warning,
                            range: range,
                            message: `Character '${symbol.name}' missing recommended property 'display_name'`,
                            source: 'cvn'
                        });
                    }
                }
            }
        }
    }
}
