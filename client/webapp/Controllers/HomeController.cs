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
        public async Task<IActionResult> Remote(string token)
        {
            var signallingURL = Environment.GetEnvironmentVariable("SIGNALLING");

            if (signallingURL != null)
            {
                var session = new SessionClient
                {
                    signallingurl =  signallingURL,
                    turnip =         "turn:13.229.142.5:3478",
                    turnuser =       "516352215",
                    turnpassword =   "810139898",
                };

                return View(new RemoteViewModel 
                {
                    token = "TestingClientModuleToken",
                    icePolicy = "all",
                    session = session
                });
            }

            try
            {
                var domain = Environment.GetEnvironmentVariable("URL");

                var result = JsonConvert.DeserializeObject<SessionClient>((
                    await (new RestClient().ExecuteAsync(
                        new RestRequest($"https://{domain}/Session/Setting",Method.Get)
                        .AddQueryParameter("token",token)))).Content);

                return View(new RemoteViewModel
                { 
                    token = token,
                    icePolicy = "all",
                    session = result
                });
            }
            catch (Exception)
            {
                return BadRequest();
            }
        }

        [Route("/Development")]
        public IActionResult Development(string ip, string port)
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
                icePolicy = "all",
                session = session
            });
        }
    }
}
