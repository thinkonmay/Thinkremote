using Renci.SshNet;
using System;
using Newtonsoft.Json;
using System.Text;
using RestSharp;
using System.Collections.Generic;

namespace port_forward
{
    class Program
    {
        static void Main(string[] args)
        {
            MainAsync( Int32.Parse(args[1]), args[2], args[3]).Wait();
        }

        static async Task MainAsync(string token,
                                    int agent_port, 
                                    int core_port)
        {
            ClusterInstance instance = null;
            PortForward agentPort = null, corePort = null;
            try
            {
                var request = new RestRequest("https://development.thinkmay.net/Cluster/Infor", Method.GET)
                    .AddHeader("Authorization",token);
                var agentPortRequest = new RestRequest("https://development.thinkmay.net/Port/Request", Method.GET)
                    .AddQueryParameter("WorkerID",WorkerID.ToString())
                    .AddQueryParameter("LocalPort",agent_port)
                    .AddHeader("Authorization",token);
                var corePortRequest = new RestRequest("https://development.thinkmay.net/Port/Request", Method.GET)
                    .AddQueryParameter("WorkerID",WorkerID.ToString())
                    .AddQueryParameter("LocalPort",core_port)
                    .AddHeader("Authorization",token);

                var instanceResult = (await (new RestClient()).ExecuteAsync(request));
                var agentResult   =  (await (new RestClient()).ExecuteAsync(agentPortRequest));
                var coreResult    =  (await (new RestClient()).ExecuteAsync(corePortRequest));

                instance =  JsonConvert.DeserializeObject<GlobalCluster>(instanceResult.Content).instance;
                agentPort = JsonConvert.DeserializeObject<PortForward>(agentResult.Content);
                corePort  = JsonConvert.DeserializeObject<PortForward>(coreResult.Content);
            }
            catch (Exception ex)
            {

            }


            MemoryStream keyStream = new MemoryStream(Encoding.UTF8.GetBytes(instance.keyPair.PrivateKey));
            var keyFiles = new[] { new PrivateKeyFile(keyStream) };

            var methods = new List<AuthenticationMethod>();
            methods.Add(new PrivateKeyAuthenticationMethod("ubuntu", keyFiles));

            var con = new ConnectionInfo(instance.IPAdress, 22, "ubuntu", methods.ToArray());
            var client = new SshClient(con);
            var agent = new ForwardedPortLocal("localhost", agent_port, "localhost", (uint)agentPort.InstancePort);
            var core  = new ForwardedPortLocal("localhost", core_port, "localhost", (uint)corePort.InstancePort);


            try
            {

                client.Connect();
                client.AddForwardedPort(agent);
                client.AddForwardedPort(core);
                agent.Start();
                core.Start();

                var portDescription = new Dictionary<string,int>();
                portDescription.Add("agent",agentPort.InstancePort);
                portDescription.Add("core", corePort.InstancePort);
                var PortDescribe = new RestRequest($"https://localhost:{agent_port}/Port/Describe", Method.POST)
                    .AddJsonBody(portDescription);
                await (new RestClient()).ExecuteAsync(PortDescribe);

                while (true) { System.Threading.Thread.Sleep(100000); }

                agent.Stop();
                core.Stop();
                client.Disconnect();            
            }
            catch (Exception ex)
            {
                Console.WriteLine($"got exception {ex.Message} while connecting with cluster");
                agent.Stop();
                core.Stop();
            }

            var agentPortRelease = new RestRequest($"https://development.thinkmay.net/Port/Release", Method.GET)
                .AddQueryParameter("InstancePort",agent_port.ToString())
                .AddHeader("Authorization",token);
            var corePortRelease = new RestRequest($"https://development.thinkmay.net/Port/Release", Method.GET)
                .AddQueryParameter("InstancePort",core_port.ToString())
                .AddHeader("Authorization",token);

            await (new RestClient()).ExecuteAsync(agentPortRelease);
            await (new RestClient()).ExecuteAsync(corePortRelease);

        }
    }
}