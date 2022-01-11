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
            string ip = GetIPAddress();
            MainAsync(args[0],$"http://{ip}:{args[1]}", int.Parse(args[1]), int.Parse(args[2])).Wait();
        }

        static string GetIPAddress()
        {
            string strHostName = System.Net.Dns.GetHostName();
            IPHostEntry ipHostInfo = Dns.Resolve(Dns.GetHostName());
            IPAddress ipAddress = ipHostInfo.AddressList
                .Where(x => x.AddressFamily == AddressFamily.InterNetwork).First();

            return ipAddress.ToString();
        }
        static async Task MainAsync(string cluster_token,
                                    string agent_url,
                                    int agent_port, 
                                    int core_port)
        {
            ClusterInstance instance = null;
            PortForward agentPort = null, corePort = null;
            try
            {
                var request = new RestRequest("https://development.thinkmay.net/Cluster/Infor", Method.GET)
                    .AddHeader("Authorization",cluster_token);
                var agentPortRequest = new RestRequest("https://development.thinkmay.net/Port/Request", Method.GET)
                    .AddQueryParameter("LocalPort",agent_port.ToString())
                    .AddHeader("Authorization",cluster_token);
                var corePortRequest = new RestRequest("https://development.thinkmay.net/Port/Request", Method.GET)
                    .AddQueryParameter("LocalPort",core_port.ToString())
                    .AddHeader("Authorization",cluster_token);

                var instanceResult = (await (new RestClient()).ExecuteAsync(request));
                var agentResult   =  (await (new RestClient()).ExecuteAsync(agentPortRequest));
                var coreResult    =  (await (new RestClient()).ExecuteAsync(corePortRequest));

                instance =  JsonConvert.DeserializeObject<GlobalCluster>(instanceResult.Content).instance;
                agentPort = JsonConvert.DeserializeObject<PortForward>(agentResult.Content);
                corePort  = JsonConvert.DeserializeObject<PortForward>(coreResult.Content);
            }
            catch (Exception ex) { return; }


            MemoryStream keyStream = new MemoryStream(Encoding.UTF8.GetBytes(instance.keyPair.PrivateKey));
            var keyFiles = new[] { new PrivateKeyFile(keyStream) };

            var methods = new List<AuthenticationMethod>();
            methods.Add(new PrivateKeyAuthenticationMethod("ubuntu", keyFiles));

            var con = new ConnectionInfo(instance.IPAdress, 22, "ubuntu", methods.ToArray());
            var client = new SshClient(con);
            var agent = new ForwardedPortRemote((uint)agentPort.InstancePort, "localhost", (uint)agent_port);
            var core  = new ForwardedPortRemote((uint) corePort.InstancePort, "localhost", (uint)core_port);


            try
            {
                client.Connect();
                client.AddForwardedPort(agent);
                client.AddForwardedPort(core);
                agent.Start();
                core.Start();

                if(await ReportPortForward(agentPort,corePort,agent_url,cluster_token)) 
                { while (true) { Thread.Sleep(10000); } }

                agent.Stop();
                core.Stop();
                client.Disconnect();            
            }
            catch (Exception ex)
            {
                Console.WriteLine($"got exception {ex.Message} while connecting with cluster");
            }
        }

        static async Task<bool> ReportPortForward(PortForward agentPort, 
                                      PortForward corePort,
                                      string agent_url,
                                      string cluster_token)
        {
            var portDescription = new Dictionary<string,string>();
            portDescription.Add("agent",agentPort.InstancePort.ToString());
            portDescription.Add("core", corePort.InstancePort.ToString());

            var PortDescribe = new RestRequest($"{agent_url}/Port/Describe", Method.POST)
                .AddJsonBody(portDescription)
                .AddHeader("Authorization",cluster_token);
            var agentRes = await (new RestClient()).ExecuteAsync(PortDescribe);
            return (agentRes.StatusCode == HttpStatusCode.OK);
        }
    }
}