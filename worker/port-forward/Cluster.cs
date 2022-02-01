using System;
using System.Collections.Generic;
using System.Text;
using System.ComponentModel.DataAnnotations;

namespace port_forward
{
    public class GlobalCluster
    {
        public ClusterInstance instance {get;set;}
    }

    public class ClusterInstance
    {
        public string IPAdress { set; get; }

        public EC2KeyPair keyPair { get; set; }
    }

    public class EC2KeyPair
    {
        public int ID { get; set; }
        public string Name { set; get; }
        public string PrivateKey { get; set; }
    }
    public class PortForward
    {
        public int  LocalPort{get;set;}

        public int  InstancePort {get;set;}
    }

    public enum ReturnCode 
    {
        PORT_FORWARD_OK = 100,
        ERROR_GET_ENV,
        ERROR_FETCH_INSTANCE_INFOR,
        ERROR_INIT_SSH_CLIENT,
        ERROR_CONNECT_TO_INSTANCE,
        ERROR_PORTFORWARD,
        ERROR_HANDLE_SSH_CONNECTION,

        ERROR_DOTNET_ENVIRONMENT = 131,
    }
}
