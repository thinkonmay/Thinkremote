using System;
using System.Collections.Generic;
using System.Text;
using System.ComponentModel.DataAnnotations;

namespace port_forward
{
    public class ClusterInstance
    {
        public int ID { get; set; }

        public string TurnUser { get; set; }

        public string TurnPassword { get; set; }

        public DateTime? Registered { get; set; }
        public string IPAdress { set; get; }

        public string InstanceID { get; set; }

        public string InstanceName { set; get; }

        public string PrivateIP { get; set; }

        public EC2KeyPair keyPair { get; set; }
    }

    public class EC2KeyPair
    {
        public int ID { get; set; }
        public string Name { set; get; }
        public string PrivateKey { get; set; }
    }
}
