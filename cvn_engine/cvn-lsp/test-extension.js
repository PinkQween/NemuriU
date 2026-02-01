// Simple test to verify the extension loads
const vscode = require('vscode');
const path = require('path');

async function test() {
    try {
        // Get the extension
        const ext = vscode.extensions.getExtension('pinkqween.cvn');
        
        if (!ext) {
            console.error('Extension not found!');
            return;
        }
        
        console.log('✓ Extension found');
        console.log('  Display Name:', ext.packageJSON.displayName);
        console.log('  Version:', ext.packageJSON.version);
        
        // Activate the extension
        if (!ext.isActive) {
            await ext.activate();
            console.log('✓ Extension activated');
        } else {
            console.log('✓ Extension already active');
        }
        
        console.log('\n✓ All tests passed!');
    } catch (error) {
        console.error('Error:', error.message);
    }
}

module.exports = { test };
