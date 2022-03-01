
using System;

namespace remote.Models
{
    public class RemoteViewModel
    {
        public string token {get;set;}


        public string icePolicy {get;set;}

        public SessionClient session {get;set;}
    }
}