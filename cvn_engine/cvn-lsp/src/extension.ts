import * as path from 'path';
import { workspace, ExtensionContext } from 'vscode';

import {
    LanguageClient,
    LanguageClientOptions,
    ServerOptions,
    TransportKind
} from 'vscode-languageclient/node';

let client: LanguageClient;

export function activate(context: ExtensionContext) {
    console.log('CVN Language Client activating...');

    // The server is implemented in node
    const serverModule = context.asAbsolutePath(
        path.join('out', 'server', 'server.js')
    );

    // The debug options for the server
    const debugOptions = { execArgv: ['--nolazy', '--inspect=6009'] };

    // If the extension is launched in debug mode then the debug server options are used
    // Otherwise the run options are used
    const serverOptions: ServerOptions = {
        run: { module: serverModule, transport: TransportKind.ipc },
        debug: {
            module: serverModule,
            transport: TransportKind.ipc,
            options: debugOptions
        }
    };

    // Options to control the language client
    const clientOptions: LanguageClientOptions = {
        // Register the server for CVN and Efficient Interrupted VN C documents
        documentSelector: [
            { scheme: 'file', language: 'cvn' },
            { scheme: 'file', language: 'effvn-c' }
        ],
        synchronize: {
            fileEvents: [
                workspace.createFileSystemWatcher('**/*.cvn'),
                workspace.createFileSystemWatcher('**/*.c')
            ]
        }
    };

    // Create the language client and start the client.
    client = new LanguageClient(
        'cvnLanguageServer',
        'CVN Language Server',
        serverOptions,
        clientOptions
    );

    // Start the client. This will also launch the server
    client.start();
    
    console.log('CVN Language Client activated!');
}

export function deactivate(): Thenable<void> | undefined {
    if (!client) {
        return undefined;
    }
    return client.stop();
}
