using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Threading.Tasks;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Extensions.Logging;
using RestSharp;
using Newtonsoft.Json;
using remote.Models;

namespace remote.Controllers
{
    public class HomeController : Controller
    {
        private readonly ILogger<HomeController> _logger;

        public HomeController(ILogger<HomeController> logger)
        {
            _logger = logger;
        }


        [Route("/Remote")]
        public async Task<IActionResult> Remote(string token, string ice)
        {
            var domain = Environment.GetEnvironmentVariable("URL");

            var result = JsonConvert.DeserializeObject<SessionClient>((
                await (new RestClient().ExecuteAsync(
                    new RestRequest($"https://{domain}/Session/Setting",Method.Get)
                    .AddQueryParameter("token",token)))).Content);

            return View(new RemoteViewModel
            { 
                token = token,
                InforURL = $"https://{domain}/Session/Setting",
                icePolicy = ice,
                session = result
            });
        }

        [Route("/Development")]
        public IActionResult Development(string ip, string port, string ice)
        {
            var session = new SessionClient
            {
                signallingurl = $"http://{ip}:{port}/Handshake",
                turnip =         "turn:13.214.177.108:3478",
                turnuser =       "359549596",
                turnpassword =   "2000860796",
                audiocodec = Codec.CODEC_H265,
                videocodec = Codec.OPUS_ENC,
            };
            return View(new DevelopmentViewModel
            {
                icePolicy = ice,
                session = session
            });
        }
    }
}
