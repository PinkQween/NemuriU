export interface CVNSymbol {
    name: string;
    type: 'character' | 'asset' | 'style' | 'start';
    assetType?: string;
    line: number;
    position: number;
}

export class CVNParser {
    parse(text: string): CVNSymbol[] {
        const symbols: CVNSymbol[] = [];
        
        // Parse characters
        const charRegex = /\bcharacter\s+(\w+)/g;
        let match;
        
        while ((match = charRegex.exec(text)) !== null) {
            symbols.push({
                name: match[1],
                type: 'character',
                line: this.getLineNumber(text, match.index),
                position: match.index
            });
        }
        
        // Parse assets
        const assetRegex = /\basset\s+(bg|sprite|music|sfx|ui|text)\s+(\w+)/g;
        while ((match = assetRegex.exec(text)) !== null) {
            symbols.push({
                name: match[2],
                type: 'asset',
                assetType: match[1],
                line: this.getLineNumber(text, match.index),
                position: match.index
            });
        }
        
        // Parse styles
        const styleRegex = /\bstyle\s+(\w+)/g;
        while ((match = styleRegex.exec(text)) !== null) {
            symbols.push({
                name: match[1],
                type: 'style',
                line: this.getLineNumber(text, match.index),
                position: match.index
            });
        }
        
        // Parse start block
        const startRegex = /\bstart\s*\{/g;
        while ((match = startRegex.exec(text)) !== null) {
            symbols.push({
                name: 'start',
                type: 'start',
                line: this.getLineNumber(text, match.index),
                position: match.index
            });
        }
        
        return symbols;
    }
    
    private getLineNumber(text: string, position: number): number {
        return text.substring(0, position).split('\n').length - 1;
    }
    
    /**
     * Find all references to a character/asset/style
     */
    findReferences(text: string, symbolName: string): number[] {
        const positions: number[] = [];
        const regex = new RegExp(`\\b${symbolName}\\b`, 'g');
        let match;
        
        while ((match = regex.exec(text)) !== null) {
            positions.push(match.index);
        }
        
        return positions;
    }
    
    /**
     * Check if a position is inside a block
     */
    isInBlock(text: string, position: number): boolean {
        let braceCount = 0;
        for (let i = 0; i < position; i++) {
            if (text[i] === '{') braceCount++;
            if (text[i] === '}') braceCount--;
        }
        return braceCount > 0;
    }
    
    /**
     * Get the containing block at a position
     */
    getContainingBlock(text: string, position: number): { type: string; start: number; end: number } | null {
        let braceStack: number[] = [];
        
        for (let i = 0; i < position; i++) {
            if (text[i] === '{') {
                braceStack.push(i);
            } else if (text[i] === '}') {
                braceStack.pop();
            }
        }
        
        if (braceStack.length === 0) return null;
        
        const blockStart = braceStack[braceStack.length - 1];
        
        // Find the end of this block
        let braceCount = 1;
        let blockEnd = blockStart + 1;
        for (let i = blockStart + 1; i < text.length; i++) {
            if (text[i] === '{') braceCount++;
            if (text[i] === '}') braceCount--;
            if (braceCount === 0) {
                blockEnd = i;
                break;
            }
        }
        
        // Determine block type by looking backwards from the opening brace
        const beforeBlock = text.substring(Math.max(0, blockStart - 100), blockStart);
        let blockType = 'unknown';
        
        if (beforeBlock.match(/\bcharacter\s+\w+\s*$/)) blockType = 'character';
        else if (beforeBlock.match(/\bstyle\s+\w+\s*$/)) blockType = 'style';
        else if (beforeBlock.match(/\bstart\s*$/)) blockType = 'start';
        else if (beforeBlock.match(/\bshow\s+.*$/)) blockType = 'show';
        
        return { type: blockType, start: blockStart, end: blockEnd };
    }
}
