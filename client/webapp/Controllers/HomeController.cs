using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Threading.Tasks;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Extensions.Logging;
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
        public IActionResult Remote(string token)
        {
            return View(new RemoteViewModel
            { 
                token = token,
                InforURL = Environment.GetEnvironmentVariable("URL")
            });
        }

        [Route("/Development")]
        public IActionResult Development(string ip, string port)
        {
            return View(new DevelopmentViewModel{});
        }
    }
}
