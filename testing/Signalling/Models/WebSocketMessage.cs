namespace Signalling.Models
{
    public class WebSocketMessage
    {
        public string RequestType { get; set; }
        public string Content { get; set; }
    }

    public class WebSocketMessageResult
    {
        public const string OFFER_SDP =  "OFFER_SDP";
        public const string OFFER_ICE =  "OFFER_ICE";
        public const string REQUEST_STREAM = "REQUEST_STREAM";
    }
}
