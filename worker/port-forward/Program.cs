using Renci.SshNet;
using System;
using Newtonsoft.Json;
using System.Text;
using RestSharp;
using System.Collections.Generic;
using System.Net.Sockets;
using System.Net;
using System.Linq;
using System.Threading;
using Renci.SshNet.Common;

namespace port_forward
{
    class Program
    {
        static void Main(string[] args)
        {
            var port = Environment.GetEnvironmentVariable("port");
            var token = Environment.GetEnvironmentVariable("clustertoken");
            var domain = Environment.GetEnvironmentVariable("domain");

            if(port == null || token == null || domain == null)
            {
                Quit(ReturnCode.ERROR_GET_ENV); 
                return;
            }

            ReversePortForward(token, int.Parse(port), domain).Wait();
            Quit(ReturnCode.PORT_FORWARD_OK);
        }
        static async Task ReversePortForward(string cluster_token,
                                             int port,
                                             string domain)
        {
            SshClient client = null;
            ClusterInstance instance = null;
            ForwardedPortRemote reverse = null;

            try
            {
                var request = new RestRequest($"https://{domain}/Cluster/Instance", Method.GET)
                    .AddHeader("Authorization",cluster_token);
                var instanceResult = (await (new RestClient()).ExecuteAsync(request));

                if(instanceResult.StatusCode != HttpStatusCode.OK ||
                   instanceResult.Content == null ||
                   instanceResult.ContentLength == 0)
                {
                    Quit(ReturnCode.ERROR_FETCH_INSTANCE_INFOR);
                }

                instance =  JsonConvert.DeserializeObject<ClusterInstance>(instanceResult.Content);
            }
            catch (Exception ex) 
            { 
                Quit(ReturnCode.ERROR_GET_ENV);
            }


            try
            {
                MemoryStream keyStream = new MemoryStream(Encoding.UTF8.GetBytes(instance.keyPair.PrivateKey));
                var keyFiles = new[] { new PrivateKeyFile(keyStream) };

                var methods = new List<AuthenticationMethod>();
                methods.Add(new PrivateKeyAuthenticationMethod("ubuntu", keyFiles));

                var con = new ConnectionInfo(instance.IPAdress, 22, "ubuntu", methods.ToArray());
                client = new SshClient(con);
                reverse = new ForwardedPortRemote((uint)port, "localhost", (uint)port);

            }
            catch (Exception ex)
            {
                Quit(ReturnCode.ERROR_INIT_SSH_CLIENT);
            }

            if(client == null || reverse == null)
            {
                Quit(ReturnCode.ERROR_INIT_SSH_CLIENT);
            }


            try
            {
                client.Connect();
                if (!client.IsConnected)
                    Quit(ReturnCode.ERROR_CONNECT_TO_INSTANCE);
            }
            catch (Exception ex)
            {
                Quit(ReturnCode.ERROR_HANDLE_SSH_CONNECTION);
            }



            try
            {
                client.KeepAliveInterval = TimeSpan.FromMinutes(1);
                client.AddForwardedPort(reverse);
                reverse.Start();

                if(!reverse.IsStarted)
                    throw new Exception();

            }
            catch (Exception ex)
            {
                Quit(ReturnCode.ERROR_PORTFORWARD);
            }

            while (true)
            {
                if(!client.IsConnected)
                {
                    Quit(ReturnCode.ERROR_HANDLE_SSH_CONNECTION);
                    Thread.Sleep(TimeSpan.FromSeconds(1));
                }
            }
        }
        static void Quit(ReturnCode ret)
        {
            Environment.Exit((int)ret);
        }
    }
}