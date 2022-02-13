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

namespace port_forward
{
    class Program
    {
        static void Main(string[] args)
        {
            var port = Environment.GetEnvironmentVariable("port");
            var token = Environment.GetEnvironmentVariable("clustertoken");
            var infor_url = Environment.GetEnvironmentVariable("clusterinfor");

            if(port == null || token == null || infor_url == null)
            {
                Quit(ReturnCode.ERROR_GET_ENV); 
                return;
            }

            ReversePortForward(token, int.Parse(port), infor_url).Wait();
            Quit(ReturnCode.PORT_FORWARD_OK);
        }
        static async Task ReversePortForward(string cluster_token,
                                             int port,
                                             string infor_url)
        {
            SshClient client = null;
            ClusterInstance instance = null;
            ForwardedPortRemote reverse = null;

            try
            {
                var request = new RestRequest(infor_url, Method.GET)
                    .AddHeader("Authorization",cluster_token);
                var instanceResult = (await (new RestClient()).ExecuteAsync(request));

                if(instanceResult.StatusCode != HttpStatusCode.OK ||
                   instanceResult.Content == null ||
                   instanceResult.ContentLength == 0)
                {
                    Quit(ReturnCode.ERROR_FETCH_INSTANCE_INFOR);
                }

                instance =  JsonConvert.DeserializeObject<GlobalCluster>(instanceResult.Content).instance;
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
                client.AddForwardedPort(reverse);
                reverse.Start();

                if(!reverse.IsStarted)
                    throw new Exception();

            }
            catch (Exception ex)
            {
                Quit(ReturnCode.ERROR_PORTFORWARD);
            }

            while (true) { Thread.Sleep(TimeSpan.FromDays(1)); }
            return;
        }
        static void Quit(ReturnCode ret)
        {
            Environment.Exit((int)ret);
        }
    }
}