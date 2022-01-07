using Renci.SshNet;
using System;
using Newtonsoft.Json;
using System.Text;
using RestSharp;

namespace port_forward
{
    class Program
    {
        static void Main(string[] args)
        {
            var ClusterName = args[0];
            var Token = args[1];

            uint.TryParse(args[2], out var WorkerPort);
            uint.TryParse(args[3], out var ClusterPort);

            var request = new RestRequest("https://host.thinkmay.net/Cluster/SSH/Key")
                .AddQueryParameter("ClusterName", ClusterName)
                .AddHeader("Authorization","Bearer "+Token);
            request.Method = Method.GET;

            var Content = ((new RestClient()).Execute(request));
            if(Content.StatusCode == System.Net.HttpStatusCode.BadRequest) { return; }

            var instance = JsonConvert.DeserializeObject<ClusterInstance>(Content.Content);
            MemoryStream keyStream = new MemoryStream(Encoding.UTF8.GetBytes(instance.keyPair.PrivateKey));
            var keyFiles = new[] { new PrivateKeyFile(keyStream) };

            var methods = new List<AuthenticationMethod>();
            methods.Add(new PrivateKeyAuthenticationMethod("ubuntu", keyFiles));

            var con = new ConnectionInfo(instance.IPAdress, 22, "ubuntu", methods.ToArray());
            var client = new SshClient(con);
            var port = new ForwardedPortLocal("localhost", WorkerPort, "localhost", ClusterPort);

            client.Connect();
            client.AddForwardedPort(port);
            port.Start();

            while (true)
            {
                System.Threading.Thread.Sleep(1000);        
            }

            port.Stop();
            client.Disconnect();            
        }
    }
}