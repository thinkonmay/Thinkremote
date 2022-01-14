/**
 * @file agent-win32-server.c
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2022-01-13
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <glib.h>
#include <agent-type.h>
#include <agent-win32-server.h>
#include <global-var.h>

#ifdef G_OS_WIN32

#ifndef UNICODE
#define UNICODE
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif


#include <windows.h>
#include <http.h>
#include <stdio.h>

#pragma comment(lib, "httpapi.lib")




//
// Macros.
//
#define INITIALIZE_HTTP_RESPONSE( resp, status, reason )    \
    do                                                      \
    {                                                       \
        RtlZeroMemory( (resp), sizeof(*(resp)) );           \
        (resp)->StatusCode = (status);                      \
        (resp)->pReason = (reason);                         \
        (resp)->ReasonLength = (USHORT) strlen(reason);     \
    } while (FALSE)

#define ADD_KNOWN_HEADER(Response, HeaderId, RawValue)               \
    do                                                               \
    {                                                                \
        (Response).Headers.KnownHeaders[(HeaderId)].pRawValue =      \
                                                          (RawValue);\
        (Response).Headers.KnownHeaders[(HeaderId)].RawValueLength = \
            (USHORT) strlen(RawValue);                               \
    } while(FALSE)
#define ALLOC_MEM(cb) HeapAlloc(GetProcessHeap(), 0, (cb))
#define FREE_MEM(ptr) HeapFree(GetProcessHeap(), 0, (ptr))

gpointer        DoReceiveRequests               (gpointer data);


DWORD           HandleHttpRequestWithBody         ( Win32Server* server,
                                                PHTTP_REQUEST pRequest);

DWORD           HandleHttpRequestWithoutBody     ( Win32Server* server,
                                                PHTTP_REQUEST pRequest);

struct _Win32Server
{
    AgentServer* server;

    HANDLE hReqQueue;

    ServerMessageHandle handle;
};



Win32Server*
init_window_server(ServerMessageHandle handle,
                   AgentServer* agent)
{
    Win32Server* server = ALLOC_MEM(sizeof(Win32Server));
    server->server = agent;
    server->handle = handle;

    HTTPAPI_VERSION api_version = HTTPAPI_VERSION_1;

    GString* string = g_string_new("http://localhost:");
    g_string_append(string,AGENT_PORT);
    g_string_append(string,"/");
    gchar* utf8 = g_string_free(string,FALSE);
    PCWSTR url = g_utf8_to_utf16(utf8,strlen(utf8),NULL,NULL,NULL);

    if (HttpInitialize( api_version, HTTP_INITIALIZE_SERVER, NULL)              != NO_ERROR) { goto Clean; }
    if (HttpCreateHttpHandle( &server->hReqQueue,0)                             != NO_ERROR) { goto Clean; }
    if (HttpAddUrl( server->hReqQueue, (url), NULL) != NO_ERROR) { goto Clean; }
    
    g_thread_new("agent-server",DoReceiveRequests,server);
    return server;

Clean:
    if(server->hReqQueue) { CloseHandle(server->hReqQueue); }
    HttpRemoveUrl( server->hReqQueue, (url));
    HttpTerminate(HTTP_INITIALIZE_SERVER, NULL);
    return NULL;
}








gpointer
DoReceiveRequests(gpointer data)
{
    Win32Server* server = (Win32Server*) data;
    ULONG              result;
    HTTP_REQUEST_ID    requestId;
    DWORD              bytesRead;
    PHTTP_REQUEST      pRequest;
    PCHAR              pRequestBuffer;
    ULONG              RequestBufferLength;

    //
    // Allocate a 2 KB buffer. This size should work for most 
    // requests. The buffer size can be increased if required. Space
    // is also required for an HTTP_REQUEST structure.
    //
    RequestBufferLength = sizeof(HTTP_REQUEST) + 2048;
    pRequestBuffer      = (PCHAR) ALLOC_MEM( RequestBufferLength );
    
    pRequest = (PHTTP_REQUEST)pRequestBuffer;

    // Wait for a new request. This is indicated by a NULL 
    // request ID.
    HTTP_SET_NULL_ID( &requestId );


    for(;;)
    {
        memset(pRequest,0,RequestBufferLength);
        result = HttpReceiveHttpRequest(
                    server->hReqQueue,  // Req Queue
                    requestId,          // Req ID
                    0,                  // Flags
                    pRequest,           // HTTP request buffer
                    RequestBufferLength,// req buffer length
                    &bytesRead,         // bytes received
                    NULL                // LPOVERLAPPED
                    );

        if(result == NO_ERROR)
        {
            if((pRequest->Flags & HTTP_REQUEST_FLAG_MORE_ENTITY_BODY_EXISTS)) 
            {
                HandleHttpRequestWithBody(server,pRequest);
            }
            else
            {
                HandleHttpRequestWithoutBody(server,pRequest);
            }
            
            HTTP_SET_NULL_ID( &requestId );
        }
        else if(result == ERROR_MORE_DATA)
        {
            //
            // The input buffer was too small to hold the request
            // headers. Increase the buffer size and call the 
            // API again. 
            //
            // When calling the API again, handle the request
            // that failed by passing a RequestID.
            //
            // This RequestID is read from the old buffer.
            //
            requestId = pRequest->RequestId;

            //
            // Free the old buffer and allocate a new buffer.
            //
            RequestBufferLength = bytesRead;
            FREE_MEM( pRequestBuffer );
            pRequestBuffer = (PCHAR) ALLOC_MEM( RequestBufferLength );

            if (pRequestBuffer == NULL)
            {
                result = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            pRequest = (PHTTP_REQUEST)pRequestBuffer;
        }
        else if(ERROR_CONNECTION_INVALID == result && !HTTP_IS_NULL_ID(&requestId))
        {
            // The TCP connection was corrupted by the peer when
            // attempting to handle a request with more buffer. 
            // Continue to the next request.
            HTTP_SET_NULL_ID( &requestId );
        }
    }

    if(pRequestBuffer)
    {
        FREE_MEM( pRequestBuffer );
    }
}











#define MAX_ULONG_STR ((ULONG) sizeof("4294967295"))

DWORD 
HandleHttpRequestWithBody( Win32Server* server,
                            PHTTP_REQUEST pRequest)
{
    HTTP_RESPONSE   response;
    DWORD           result;

    gchar*          pEntityBuffer[2048];

    ULONG           BytesRead;
    ULONG           TotalBytesRead = 0;

    gchar*          buffer_pointer[8192];
    gchar*          buffer = buffer_pointer;

    memset(pEntityBuffer,0,2048);
    memset(buffer,0,8192);

    do
    {
        BytesRead = 0; 
        result = HttpReceiveRequestEntityBody(
                    server->hReqQueue,
                    pRequest->RequestId,
                    0,
                    pEntityBuffer,
                    2048,
                    &BytesRead,
                    NULL );
        switch(result)
        {
            case NO_ERROR:
                if(BytesRead != 0)
                {
                    TotalBytesRead += BytesRead;
                    buffer += BytesRead;
                    memcpy(buffer,pEntityBuffer,BytesRead);
                }
                break;
            case ERROR_HANDLE_EOF:
                //
                // The last request entity body has been read.
                // Send back a response. 
                //
                // To illustrate entity sends via 
                // HttpSendResponseEntityBody, the response will 
                // be sent over multiple calls. To do this,
                // pass the HTTP_SEND_RESPONSE_FLAG_MORE_DATA
                // flag.
                
                if(BytesRead != 0)
                {
                    TotalBytesRead += BytesRead;
                    buffer += BytesRead;
                    memcpy(buffer,pEntityBuffer,BytesRead);
                }

                gchar response_buffer[8192] = {0};
                memset(response_buffer,0,8192);
                gchar* request_token = pRequest->Headers.KnownHeaders[HttpHeaderAuthorization].pRawValue;
                GBytes* byte = g_bytes_new(buffer,TotalBytesRead);
                if((*(server->handle))(pRequest->pRawUrl, request_token,byte,response_buffer,server->server))
                {
                    INITIALIZE_HTTP_RESPONSE(&response, 200, "OK");
                }
                else
                {
                    INITIALIZE_HTTP_RESPONSE(&response, 400, "BAD REQUEST");
                }

                SendHttpResponse(server->hReqQueue,pRequest,response.StatusCode,response.pReason,response_buffer);
                return;
            default:
                return;
        }
    } while(TRUE);
}
















DWORD SendHttpResponse(
    IN HANDLE        hReqQueue,
    IN PHTTP_REQUEST pRequest,
    IN USHORT        StatusCode,
    IN PSTR          pReason,
    IN PSTR          pEntityString
    )
{
    HTTP_RESPONSE   response;
    HTTP_DATA_CHUNK dataChunk;
    DWORD           result;
    DWORD           bytesSent;

    //
    // Initialize the HTTP response structure.
    //
    INITIALIZE_HTTP_RESPONSE(&response, StatusCode, pReason);

    //
    // Add a known header.
    //
    ADD_KNOWN_HEADER(response, HttpHeaderContentType, "application/json");
   
    if(strlen(pEntityString) > 0)
    {
        dataChunk.DataChunkType           = HttpDataChunkFromMemory;
        dataChunk.FromMemory.pBuffer      = pEntityString;
        dataChunk.FromMemory.BufferLength = (ULONG) strlen(pEntityString);
        response.EntityChunkCount         = 1;
        response.pEntityChunks            = &dataChunk;
    }
    else
    {
        dataChunk.DataChunkType           = HttpDataChunkFromMemory;
        dataChunk.FromMemory.pBuffer      = "hello";
        dataChunk.FromMemory.BufferLength = (ULONG) strlen("hello");
        response.EntityChunkCount         = 1;
        response.pEntityChunks            = &dataChunk;

    }


    // 
    // Because the entity body is sent in one call, it is not
    // required to specify the Content-Length.
    //
    
    result = HttpSendHttpResponse(
                    hReqQueue,           // ReqQueueHandle
                    pRequest->RequestId, // Request ID
                    0,                   // Flags
                    &response,           // HTTP response
                    NULL,                // pReserved1
                    &bytesSent,          // bytes sent  (OPTIONAL)
                    NULL,                // pReserved2  (must be NULL)
                    0,                   // Reserved3   (must be 0)
                    NULL,                // LPOVERLAPPED(OPTIONAL)
                    NULL                 // pReserved4  (must be NULL)
                    ); 

    if(result != NO_ERROR)
    {
        wprintf(L"HttpSendHttpResponse failed with %lu \n", result);
    }

    return result;
}





DWORD 
HandleHttpRequestWithoutBody(Win32Server* server,
                            PHTTP_REQUEST pRequest)
{
    HTTP_RESPONSE   response;
    DWORD           result;

    gchar response_buffer[8192] = {0};
    memset(response_buffer,0,8192);
    gchar* request_token = pRequest->Headers.KnownHeaders[HttpHeaderAuthorization].pRawValue;
    if((*(server->handle))(pRequest->pRawUrl, request_token,NULL,response_buffer,server->server))
    {
        INITIALIZE_HTTP_RESPONSE(&response, 200, "OK");
    }
    else
    {
        INITIALIZE_HTTP_RESPONSE(&response, 400, "BAD REQUEST");
    }

    SendHttpResponse(server->hReqQueue,pRequest,response.StatusCode,response.pReason,response_buffer);
    return;
}

#endif