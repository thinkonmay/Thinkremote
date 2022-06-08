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
#include <json-handler.h>
#include <json-glib/json-glib.h>
#include <global-var.h>

#include <device.h>

/**
 * @brief 
 * 
 */
typedef struct _DeviceInformation
{
	gchar cpu[100];
	gchar gpu[512];
	gint  ram_capacity;
	gchar OS[100];
	gchar IP[100];
	gchar Name[100];
	gchar User[100];
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

	#define INFO_BUFFER_SIZE 32767
	TCHAR  infoBuf[INFO_BUFFER_SIZE];
	DWORD  bufCharCount = INFO_BUFFER_SIZE;

	// Get and display the name of the computer.
	GetComputerName( infoBuf, &bufCharCount ) ;
	memcpy(device_info->Name,infoBuf,bufCharCount);


	GetUserName( infoBuf, &bufCharCount ) ;
	memcpy(device_info->User,infoBuf,bufCharCount);

	gchar* ip = get_local_ip();

	memcpy(device_info->OS , &OS,strlen(OS));
	memcpy(device_info->IP,ip,strlen(ip));

	return device_info;
}
#else

DeviceInformation* 			
get_device_information()
{
	DeviceInformation* device_info = malloc(sizeof(DeviceInformation));
	memset(device_info,0, sizeof(DeviceInformation));
	return device_info;
}


#endif 


gchar*
get_registration_message(gboolean port_forward, 
						 gchar* port)
{
	DeviceInformation* infor = get_device_information();
	JsonObject* information = json_object_new();

	json_object_set_int_member(information,		"RAMcapacity", infor->ram_capacity);
	json_object_set_string_member(information,	"CPU", infor->cpu);
	json_object_set_string_member(information,	"GPU", infor->gpu);
	json_object_set_string_member(information,	"OS", infor->OS);
	json_object_set_string_member(information,	"Name", infor->Name);
	json_object_set_string_member(information,	"User", infor->User);


	GString* agent_url_string = g_string_new("http://");

	if(!port_forward)
	{
		g_string_append(agent_url_string, infor->IP );

		g_string_append(agent_url_string, ":");

		g_string_append(agent_url_string, port);
	}
	else
	{
		g_string_append(agent_url_string,"localhost" );

		g_string_append(agent_url_string, ":");

		g_string_append(agent_url_string, port);
	}
	
	json_object_set_string_member(information,	"AgentUrl", g_string_free(agent_url_string,FALSE));

	return get_string_from_json_object(information);
}
