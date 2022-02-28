using System.Threading.Tasks;
using Microsoft.AspNetCore.Mvc;
using Signalling.Models;
using Signalling.Interfaces;
using RestSharp;

namespace Signalling.Controllers
{
    [ApiController]
    [Produces("application/json")]
    public class SessionsController : ControllerBase
    {
        private readonly ISessionQueue Queue;

        private readonly RestClient Authenticator;

        public SessionsController(ISessionQueue queue)
        {
            Authenticator = new RestClient();
            Queue = queue;
        }


        [HttpGet("Handshake")]
        public async Task Get(string token)
        {
            var context = ControllerContext.HttpContext;
            if (context.WebSockets.IsWebSocketRequest)
            {
                Module module;
                if(token == "TestingCoreModuleToken")
                {
                    module = Module.CORE_MODULE;
                }
                else if(token == "TestingClientModuleToken")
                {
                    module = Module.CLIENT_MODULE;
                }
                else
                {
                    return; 
                }

                var accession = new SessionAccession
                {
                    Module = module,
                    ID = 0,
                    ClientID = 1,
                    WorkerID = 2,
                };
                var webSocket = await context.WebSockets.AcceptWebSocketAsync();
                await Queue.Handle(accession, webSocket);
            }
        }
    }
}
