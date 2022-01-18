using Signalling.Models;
using System.Text;
using Signalling.Interfaces;
using System;
using System.Collections.Concurrent;
using System.Linq;
using System.Net.WebSockets;
using System.Threading;
using System.Threading.Tasks;
using System.IO;
using Newtonsoft.Json;

namespace Signalling.Services
{
    public class SessionQueue : ISessionQueue
    {
        private ConcurrentDictionary<SessionAccession, WebSocket> onlineList;

        public SessionQueue()
        {
            onlineList = new ConcurrentDictionary<SessionAccession, WebSocket>();
        }



        public async Task Handle(SessionAccession accession, WebSocket ws)
        {
            onlineList.AddOrUpdate(accession,ws, (b,c) => ws);
            var core = onlineList.Where(o => o.Key.ID == accession.ID);
            if(core.Count() == 2)
            {
                var sessionCore = core.Where(o => o.Key.Module == Module.CORE_MODULE).First();
                SendMessage(sessionCore.Value,JsonConvert.SerializeObject(new WebSocketMessage{RequestType = WebSocketMessageResult.REQUEST_STREAM, Content = " "}));
            }

            try
            {
                do
                {
                    using (var memoryStream = new MemoryStream())
                    {
                        var message = ReceiveMessage(ws, memoryStream).Result;
                        if (message.Count > 0)
                        {
                            var receivedMessage = Encoding.UTF8.GetString(memoryStream.ToArray());
                            var WebSocketMessage = JsonConvert.DeserializeObject<WebSocketMessage>(receivedMessage);

                            _handleSdpIceOffer(accession, WebSocketMessage,ws);
                        }
                    }
                } while (ws.State == WebSocketState.Open);
                await ws.CloseAsync(WebSocketCloseStatus.NormalClosure, string.Empty, CancellationToken.None);
            }
            catch (Exception ex)
            {
                Serilog.Log.Information($"{ex.Message} : {ex.StackTrace}");
            }

            Serilog.Log.Information("Connection closed");
            onlineList.TryRemove(accession,out var output);
        }

        private async Task<WebSocketReceiveResult> ReceiveMessage(WebSocket ws, Stream memoryStream)
        {
            var readBuffer = new ArraySegment<byte>(new byte[4 * 1024]);
            WebSocketReceiveResult result;
            do
            {
                result = await ws.ReceiveAsync(readBuffer, CancellationToken.None);
                await memoryStream.WriteAsync(readBuffer.Array, readBuffer.Offset, result.Count,
                    CancellationToken.None);
            } while (!result.EndOfMessage);
            return result;
        }

        public async Task SendMessage(WebSocket ws, string msg)
        {
            var bytes = Encoding.UTF8.GetBytes(msg);
            var buffer = new ArraySegment<byte>(bytes);
            await ws.SendAsync(buffer, WebSocketMessageType.Text, true, CancellationToken.None);
        }


        async Task _handleSdpIceOffer(SessionAccession accession, WebSocketMessage msg, WebSocket ws)
        {
            try
            {
                foreach(var item in onlineList)
                {
                    if(item.Key.ID == accession.ID)
                    {
                        if(accession.Module == Module.CLIENT_MODULE &&
                           item.Key.Module == Module.CORE_MODULE)
                        {
                            await SendMessage(item.Value, JsonConvert.SerializeObject(msg));
                            return;
                        }
                        if(accession.Module == Module.CORE_MODULE &&
                           item.Key.Module == Module.CLIENT_MODULE)
                        {
                            await SendMessage(item.Value, JsonConvert.SerializeObject(msg));
                            return;
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                Serilog.Log.Information("Fail to handle handshake");                
                Serilog.Log.Information(ex.Message);                
                Serilog.Log.Information(ex.StackTrace);                
            }
        }
    }
}
