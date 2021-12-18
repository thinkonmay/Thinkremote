using System.Net.WebSockets;
using System.Threading.Tasks;
using Signalling.Models;

namespace Signalling.Interfaces
{
    public interface ISessionQueue
    {
        Task Handle(SessionAccession accession, WebSocket ws);
    }
}
