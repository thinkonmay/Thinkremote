const { app, BrowserWindow } = require('electron');

// Handle creating/removing shortcuts on Windows when installing/uninstalling.
if (require('electron-squirrel-startup')) { // eslint-disable-line global-require
  app.quit();
}

const createWindow = () => {
  // Create the browser window.
  const mainWindow = new BrowserWindow({
    show: false,
    icon: 'assets/logo.ico',
  });
  mainWindow.setTitle(require('./package.json').name);
  mainWindow.maximize();
  mainWindow.show();
  mainWindow.loadURL("https://service.thinkmay.net/")
};

// This method will be called when Electron has finished
// initialization and is ready to create browser windows.
// Some APIs can only be used after this event occurs.
app.on('ready', createWindow);

// Quit when all windows are closed, except on macOS. There, it's common
// for applications and their menu bar to stay active until the user quits
// explicitly with Cmd + Q.
app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    app.quit();
  }
});

app.on('activate', () => {
  // On OS X it's common to re-create a window in the app when the
  // dock icon is clicked and there are no other windows open.
  if (BrowserWindow.getAllWindows().length === 0) {
    createWindow();
  }
});

// In this file you can include the rest of your app's specific main process
// code. You can also put them in separate files and import them here.  



const ProtocolRegistry = require('protocol-registry');

console.log('Registering...');
// Registers the Protocol
ProtocolRegistry.register({
    protocol: 'thinkmay', // sets protocol for your command , testproto://**
    command: `powershell ./remote-app.exe $_URL_`, // $_URL_ will the replaces by the url used to initiate it
    override: true, // Use this with caution as it will destroy all previous Registrations on this protocol
    terminal: true, // Use this to run your command inside a terminal
    script: false
}).then(async () => {
    console.log('Successfully registered');
});