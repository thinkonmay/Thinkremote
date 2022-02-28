export const signalling_hub_get_connection_string = () => {
    return `${app.SignallingUrl}?token=${app.remoteToken}`;
}

/**
 * 
 * @param {string} message 
 */
export const setDebug = (message) => 
{
    console.log(message);
    this.debugEntries.push(applyTimestamp(message));
}