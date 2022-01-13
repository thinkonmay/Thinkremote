using Renci.SshNet;
using System;
using Newtonsoft.Json;
using System.Text;
using RestSharp;
using System.Collections.Generic;
using System.Net.Sockets;
using System.Net;
using System.Linq;

namespace port_forward
{
    class Program
    {
        static void Main(string[] args)
        {
            try
            {
                var request = new RestRequest("http://localhost:2220/PortDescribe", Method.POST)
                    .AddJsonBody("asdfbsdaklfbjsdafb");
                var result = JsonConvert.DeserializeObject<Dictionary<string,string>>((new RestClient()).Execute(request).Content);

                result.TryGetValue("agent_port", out var agent_port);
                result.TryGetValue("token", out var token);

                MainAsync(token, 2220, int.Parse(agent_port)).Wait();
            }
            catch (Exception ex)
            {
            }
        }
        static async Task MainAsync(string cluster_token,
                                    int agent_port, 
                                    int agent_instance_port)
        {
            ClusterInstance instance = null;
            try
            {
                var request = new RestRequest("https://host.thinkmay.net/Cluster/Infor", Method.GET)
                    .AddHeader("Authorization",cluster_token);

                var instanceResult = (await (new RestClient()).ExecuteAsync(request));
                instance =  JsonConvert.DeserializeObject<GlobalCluster>(instanceResult.Content).instance;
            }
            catch (Exception ex) { return; }


            MemoryStream keyStream = new MemoryStream(Encoding.UTF8.GetBytes(instance.keyPair.PrivateKey));
            var keyFiles = new[] { new PrivateKeyFile(keyStream) };

            var methods = new List<AuthenticationMethod>();
            methods.Add(new PrivateKeyAuthenticationMethod("ubuntu", keyFiles));

            var con = new ConnectionInfo(instance.IPAdress, 22, "ubuntu", methods.ToArray());
            var client = new SshClient(con);
            var agent = new ForwardedPortRemote((uint)agent_instance_port, "localhost", (uint)agent_port);


            try
            {
                client.Connect();
                client.AddForwardedPort(agent);
                agent.Start();

                { while (true) { Thread.Sleep(10000); } }

                agent.Stop();
                client.Disconnect();            
            }
            catch (Exception ex)
            {
            }
        }
    }
}