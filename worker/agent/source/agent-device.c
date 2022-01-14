/**
 * @file agent-device.c
 * @author {Do Huy Hoang} ({huyhoangdo0205@gmail.com})
 * @brief 
 * @version 1.0
 * @date 2021-12-01
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <agent-device.h>

#include <logging.h>
#include <message-form.h>
#include <json-glib/json-glib.h>
#include <global-var.h>

/**
 * @brief 
 * 
 */
typedef struct _DeviceInformation
{
	gchar cpu[100];
	gchar gpu[512];
	gint ram_capacity;
	gchar OS[100];
	gchar IP[100];
}DeviceInformation;

#ifdef G_OS_WIN32
#include <winsock2.h>
#include <Windows.h>	
#include <stdio.h>
#include <sysinfoapi.h>
#include <d3d9.h>
#include <intrin.h>
#include <stdlib.h>
#include <iphlpapi.h>
#include <global-var.h>

#define DIV 1048576
#define ID  0
#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "d3d9.lib")

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))



gchar* 
get_local_ip()
{
	gchar* ip_address = malloc(20);
    PIP_ADAPTER_INFO pAdapterInfo;
    PIP_ADAPTER_INFO current_adapter = NULL;
    DWORD dwRetVal = 0;
    UINT i;

/* variables used to print DHCP time info */
    struct tm newtime;
    char buffer[32];
    errno_t error;

    ULONG ulOutBufLen = sizeof (IP_ADAPTER_INFO);
    pAdapterInfo = (IP_ADAPTER_INFO *) MALLOC(sizeof (IP_ADAPTER_INFO));
    if (pAdapterInfo == NULL) {
        printf("Error allocating memory needed to call GetAdaptersinfo\n");
        return 1;
    }
// Make an initial call to GetAdaptersInfo to get
// the necessary size into the ulOutBufLen variable
    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
        FREE(pAdapterInfo);
        pAdapterInfo = (IP_ADAPTER_INFO *) MALLOC(ulOutBufLen);
        if (pAdapterInfo == NULL) {
            printf("Error allocating memory needed to call GetAdaptersinfo\n");
            return 1;
        }
    }

    if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
        current_adapter = pAdapterInfo;
		do {
			memset(ip_address,0,20);
			memcpy(ip_address, current_adapter->IpAddressList.IpAddress.String,
				strlen(current_adapter->IpAddressList.IpAddress.String));
			current_adapter = current_adapter->Next;
		} while(!g_strcmp0(ip_address,"0.0.0.0")); 
    } else {
        printf("GetAdaptersInfo failed with error: %d\n", dwRetVal);

    }
    if (pAdapterInfo)
        FREE(pAdapterInfo);

    return ip_address;
}
















DeviceInformation*
get_device_information() 
{
	DeviceInformation* device_info = malloc(sizeof(DeviceInformation));
	memset(device_info,0, sizeof(DeviceInformation));

	int CPUInfo[4] = { -1 };
	unsigned nExIds, i = 0;
	char CPUBrandString[0x40];
	__cpuid(CPUInfo, 0x80000000);
	nExIds = CPUInfo[0];
	


	for (i = 0x80000000; i <= nExIds; i++) {
		__cpuid(CPUInfo, i);
		if (i == 0x80000002)
			memcpy(CPUBrandString, CPUInfo, sizeof(CPUInfo));
		else if (i == 0x80000003)
			memcpy(CPUBrandString + 16, CPUInfo, sizeof(CPUInfo));
		else if (i == 0x80000004)
			memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
	}
	memcpy(device_info->cpu , CPUBrandString,64);

	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof(statex);
	GlobalMemoryStatusEx(&statex);
	guint64 ram_cap = (statex.ullTotalPhys / 1024) / 1024;

	device_info->ram_capacity = ram_cap;

	IDirect3D9* d3Object = Direct3DCreate9(D3D_SDK_VERSION);
	UINT adaptercount = d3Object->lpVtbl->GetAdapterCount(d3Object);
	D3DADAPTER_IDENTIFIER9* adapters = (D3DADAPTER_IDENTIFIER9*)malloc(sizeof(D3DADAPTER_IDENTIFIER9) * adaptercount);

	for (int i = 0; i < adaptercount; i++)
	{
		d3Object->lpVtbl->GetAdapterIdentifier(d3Object, i, 0, &(adapters[i]));
	}

	memcpy(device_info->gpu , &adapters->Description,512);



	gchar OS[100] ;
	memset(&OS,0, sizeof(OS));
	DWORD minor_version = (HIBYTE(LOWORD(GetVersion())));
	DWORD major_version = (LOBYTE(LOWORD(GetVersion())));

	gchar major[5];
	gchar minor[5];


	itoa(major_version, major, 10);
	itoa(minor_version, minor, 10);

	strcat(OS, "Window10 Version ");
	strcat(OS, major);
	strcat(OS, ".");
	strcat(OS, minor);



	gchar* ip = get_local_ip();

	memcpy(device_info->OS , &OS,strlen(OS));
	memcpy(device_info->IP,ip,strlen(ip));

	return device_info;
}
#else
#include <stdio.h>
#include <unistd.h>
#include <string.h> /* for strncpy */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>

gchar*
get_local_ip()
{
	int fd;
	struct ifreq ifr;

	fd = socket(AF_INET, SOCK_DGRAM, 0);

	/* I want to get an IPv4 IP address */
	ifr.ifr_addr.sa_family = AF_INET;

	/* I want IP address attached to "eth0" */
	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);
	ioctl(fd, SIOCGIFADDR, &ifr);
	close(fd);

	return inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
}

DeviceInformation*
get_device_information() 
{

}

#endif 


gchar*
get_registration_message(gboolean port_forward, 
						 gchar* agent_instance_port, 
						 gchar* core_instance_port)
{
	DeviceInformation* infor = get_device_information();
	JsonObject* information = json_object_new();

	json_object_set_string_member(information,	"CPU", infor->cpu);
	json_object_set_string_member(information,	"GPU", infor->gpu);
	json_object_set_string_member(information,	"OS", infor->OS);
	json_object_set_int_member(information,		"RAMcapacity", infor->ram_capacity);

	GString* agent_url_string = g_string_new("http://");

	if(!port_forward)
	{
		g_string_append(agent_url_string, infor->IP );

		g_string_append(agent_url_string, ":");

		g_string_append(agent_url_string, AGENT_PORT 		);
	}
	else
	{
		g_string_append(agent_url_string,"localhost" );

		g_string_append(agent_url_string, ":");

		g_string_append(agent_url_string, agent_instance_port);
	}
	
	json_object_set_string_member(information,	"AgentUrl", g_string_free(agent_url_string,FALSE));

	return get_string_from_json_object(information);
}
