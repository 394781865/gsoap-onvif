#include <iostream>
#include "stdio.h"
#include "wsdd.nsmap"
#include "plugin/wsseapi.h"
#include "plugin/wsaapi.h"
#include  <openssl/rsa.h>
#include  "ErrorLog.h"
 
#include "include/soapDeviceBindingProxy.h"
#include "include/soapMediaBindingProxy.h"
#include "include/soapPTZBindingProxy.h"

#include "include/soapPullPointSubscriptionBindingProxy.h"
#include "include/soapRemoteDiscoveryBindingProxy.h" 

#include <algorithm>
#include <vector>
#include "onvifdevice.h"
using namespace std;

//#define DEV_PASSWORD "hik123456"
#define MAX_HOSTNAME_LEN 128
#define MAX_LOGMSG_LEN 256 
vector<string> s_dev;


void PrintErr(struct soap* _psoap)
{
	fflush(stdout);
	processEventLog(__FILE__, __LINE__, stdout, "error:%d faultstring:%s faultcode:%s faultsubcode:%s faultdetail:%s", _psoap->error, 
	*soap_faultstring(_psoap), *soap_faultcode(_psoap),*soap_faultsubcode(_psoap), *soap_faultdetail(_psoap));
}
#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3
#define ZOOMIN 4
#define ZOOMOUT 5
#define speed 1

void ptzContinuousMove(PTZBindingProxy*ptzProxy,string profile,int command)
{
  _tptz__ContinuousMove* continuousMove = soap_new__tptz__ContinuousMove(ptzProxy->soap, -1);
  _tptz__ContinuousMoveResponse* response = soap_new__tptz__ContinuousMoveResponse(ptzProxy->soap, -1);
  continuousMove->ProfileToken = const_cast<char *>(profile.c_str());
  tt__PTZSpeed* velocity = soap_new_tt__PTZSpeed(ptzProxy->soap, -1);
  continuousMove->Velocity = velocity;
  tt__Vector2D* panTilt = soap_new_tt__Vector2D(ptzProxy->soap, -1);
  continuousMove->Velocity->PanTilt = panTilt;
  std::string str="http://www.onvif.org/ver10/tptz/PanTiltSpaces/VelocityGenericSpace";
  continuousMove->Velocity->PanTilt->space =&str;
  tt__Vector1D* zoom = soap_new_tt__Vector1D(ptzProxy->soap, -1);
  continuousMove->Velocity->Zoom = zoom;
  switch (command)
  {
  case LEFT:
    continuousMove->Velocity->PanTilt->x = -((float)speed / 10);
    continuousMove->Velocity->PanTilt->y = 0;
    break;
  case RIGHT:
    continuousMove->Velocity->PanTilt->x = ((float)speed / 10);
    continuousMove->Velocity->PanTilt->y = 0;
    break;
  case UP:
    continuousMove->Velocity->PanTilt->x = 0;
    continuousMove->Velocity->PanTilt->y = ((float)speed / 10);
    break;
  case DOWN:
    continuousMove->Velocity->PanTilt->x = 0;
    continuousMove->Velocity->PanTilt->y = -((float)speed / 10);
    break;
  case ZOOMIN:
    continuousMove->Velocity->Zoom->x = ((float)speed / 10);
    break;
  case ZOOMOUT:
    continuousMove->Velocity->Zoom->x = -((float)speed / 10);
    break;
  default:
    break;
  }
  
  if (ptzProxy->ContinuousMove(ptzProxy->soap_endpoint,NULL,continuousMove, response) == SOAP_OK)
    //转动成功
    std::cout<<"\nsuccess\n";
  else
    std::cout<<"\nfail\n";
}

#if 0
int getdevice(char*szHostName)
{

	bool blSupportPTZ = false;
	//char szHostName[MAX_HOSTNAME_LEN] = { 0 };
	char sLogMsg[MAX_LOGMSG_LEN] = { 0 };

	DeviceBindingProxy proxyDevice;
	RemoteDiscoveryBindingProxy proxyDiscovery; 
	MediaBindingProxy proxyMedia;
	PTZBindingProxy proxyPTZ;
	PullPointSubscriptionBindingProxy proxyEvent;

	/*if (argc > 1)
	{
		strcat(szHostName, "http://");
		strcat(szHostName, argv[1]);
		strcat(szHostName, "/onvif/device_service");

		proxyDevice.soap_endpoint = szHostName;
	}
	else
	{
		processEventLog(__FILE__, __LINE__, stdout, "wrong args,usage: ./a.out 172.18.4.100 ");
		return -1;
	}*/
  proxyDevice.soap_endpoint = szHostName;

	soap_register_plugin(proxyDevice.soap, soap_wsse);
	soap_register_plugin(proxyDiscovery.soap, soap_wsse);
	soap_register_plugin(proxyMedia.soap, soap_wsse);
	soap_register_plugin(proxyPTZ.soap, soap_wsse);
	soap_register_plugin(proxyEvent.soap, soap_wsse);

	soap_register_plugin(proxyEvent.soap, soap_wsa);

	struct soap *soap = soap_new();

	if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(proxyDevice.soap, NULL, "admin", DEV_PASSWORD))
	{
		return -1;
	}

	if (SOAP_OK != soap_wsse_add_Timestamp(proxyDevice.soap, "Time", 10)) 
	{
		return -1;
	}

	_tds__GetWsdlUrl *tds__GetWsdlUrl = soap_new__tds__GetWsdlUrl(soap, -1);
	_tds__GetWsdlUrlResponse *tds__GetWsdlUrlResponse = soap_new__tds__GetWsdlUrlResponse(soap, -1);

	if (SOAP_OK == proxyDevice.GetWsdlUrl(tds__GetWsdlUrl, tds__GetWsdlUrlResponse))
	{
		processEventLog(__FILE__, __LINE__, stdout, "-------------------WsdlUrl-------------------");
		processEventLog(__FILE__, __LINE__, stdout, "WsdlUrl:%s ", tds__GetWsdlUrlResponse->WsdlUrl.c_str());
	}
	else
	{
		PrintErr(proxyDevice.soap);
	}

	soap_destroy(soap); 
	soap_end(soap); 

	if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(proxyDevice.soap, NULL, "admin", DEV_PASSWORD))
	{
		return -1;
	}

	_tds__GetCapabilities *tds__GetCapabilities = soap_new__tds__GetCapabilities(soap, -1);
	tds__GetCapabilities->Category.push_back(tt__CapabilityCategory__All);

	_tds__GetCapabilitiesResponse *tds__GetCapabilitiesResponse = soap_new__tds__GetCapabilitiesResponse(soap, -1);

	if (SOAP_OK == proxyDevice.GetCapabilities(tds__GetCapabilities, tds__GetCapabilitiesResponse))
	{
		if (tds__GetCapabilitiesResponse->Capabilities->Analytics != NULL)
		{
			processEventLog(__FILE__, __LINE__, stdout, "-------------------Analytics-------------------");
			processEventLog(__FILE__, __LINE__, stdout, "XAddr:%s", tds__GetCapabilitiesResponse->Capabilities->Analytics->XAddr.c_str());
			processEventLog(__FILE__, __LINE__, stdout, "RuleSupport:%s", (tds__GetCapabilitiesResponse->Capabilities->Analytics->RuleSupport) ? "Y" : "N");
			processEventLog(__FILE__, __LINE__, stdout, "AnalyticsModuleSupport:%s", (tds__GetCapabilitiesResponse->Capabilities->Analytics->AnalyticsModuleSupport) ? "Y" : "N");
		}

		if (tds__GetCapabilitiesResponse->Capabilities->Device != NULL)
		{
			processEventLog(__FILE__, __LINE__, stdout, "-------------------Device-------------------");
			processEventLog(__FILE__, __LINE__, stdout, "XAddr:%s", tds__GetCapabilitiesResponse->Capabilities->Device->XAddr.c_str());

			processEventLog(__FILE__, __LINE__, stdout, "-------------------Network-------------------");
			processEventLog(__FILE__, __LINE__, stdout, "IPFilter:%s", (tds__GetCapabilitiesResponse->Capabilities->Device->Network->IPFilter) ? "Y" : "N");
			processEventLog(__FILE__, __LINE__, stdout, "ZeroConfiguration:%s", (tds__GetCapabilitiesResponse->Capabilities->Device->Network->ZeroConfiguration) ? "Y" : "N");
			processEventLog(__FILE__, __LINE__, stdout, "IPVersion6:%s", (tds__GetCapabilitiesResponse->Capabilities->Device->Network->IPVersion6) ? "Y" : "N");
			processEventLog(__FILE__, __LINE__, stdout, "DynDNS:%s", (tds__GetCapabilitiesResponse->Capabilities->Device->Network->DynDNS) ? "Y" : "N");

			processEventLog(__FILE__, __LINE__, stdout, "-------------------System-------------------");
			processEventLog(__FILE__, __LINE__, stdout, "DiscoveryResolve:%s", (tds__GetCapabilitiesResponse->Capabilities->Device->System->DiscoveryResolve) ? "Y" : "N");
			processEventLog(__FILE__, __LINE__, stdout, "DiscoveryBye:%s", (tds__GetCapabilitiesResponse->Capabilities->Device->System->DiscoveryBye) ? "Y" : "N");
			processEventLog(__FILE__, __LINE__, stdout, "RemoteDiscovery:%s", (tds__GetCapabilitiesResponse->Capabilities->Device->System->RemoteDiscovery) ? "Y" : "N");

			int iSize = tds__GetCapabilitiesResponse->Capabilities->Device->System->SupportedVersions.size();

			if (iSize > 0)
			{
				processEventLog(__FILE__, __LINE__, stdout, "SupportedVersions:");

				for (int i = 0; i < iSize; i++)
				{
					processEventLog(__FILE__, __LINE__, stdout, "%d.%d ", tds__GetCapabilitiesResponse->Capabilities->Device->System->SupportedVersions[i]->Major,
																		  tds__GetCapabilitiesResponse->Capabilities->Device->System->SupportedVersions[i]->Minor);
				}

				processEventLog(__FILE__, __LINE__, stdout, "");
			}

			processEventLog(__FILE__, __LINE__, stdout, "SystemBackup:%s", (tds__GetCapabilitiesResponse->Capabilities->Device->System->SystemBackup) ? "Y" : "N");
			processEventLog(__FILE__, __LINE__, stdout, "FirmwareUpgrade:%s", (tds__GetCapabilitiesResponse->Capabilities->Device->System->FirmwareUpgrade) ? "Y" : "N");
			processEventLog(__FILE__, __LINE__, stdout, "SystemLogging:%s", (tds__GetCapabilitiesResponse->Capabilities->Device->System->SystemLogging) ? "Y" : "N");

			processEventLog(__FILE__, __LINE__, stdout, "-------------------IO-------------------");
			processEventLog(__FILE__, __LINE__, stdout, "InputConnectors:%d", tds__GetCapabilitiesResponse->Capabilities->Device->IO->InputConnectors);
			processEventLog(__FILE__, __LINE__, stdout, "RelayOutputs:%d", tds__GetCapabilitiesResponse->Capabilities->Device->IO->RelayOutputs);

			processEventLog(__FILE__, __LINE__, stdout, "-------------------Security-------------------");
			processEventLog(__FILE__, __LINE__, stdout, "TLS1.1:%s", (tds__GetCapabilitiesResponse->Capabilities->Device->Security->TLS1_x002e1) ? "Y" : "N");
			processEventLog(__FILE__, __LINE__, stdout, "TLS1.2:%s", (tds__GetCapabilitiesResponse->Capabilities->Device->Security->TLS1_x002e2) ? "Y" : "N");
			processEventLog(__FILE__, __LINE__, stdout, "OnboardKeyGeneration:%s", (tds__GetCapabilitiesResponse->Capabilities->Device->Security->OnboardKeyGeneration) ? "Y" : "N");
			processEventLog(__FILE__, __LINE__, stdout, "AccessPolicyConfig:%s", (tds__GetCapabilitiesResponse->Capabilities->Device->Security->AccessPolicyConfig) ? "Y" : "N");
			processEventLog(__FILE__, __LINE__, stdout, "X.509Token:%s", (tds__GetCapabilitiesResponse->Capabilities->Device->Security->X_x002e509Token) ? "Y" : "N");
			processEventLog(__FILE__, __LINE__, stdout, "SAMLToken:%s", (tds__GetCapabilitiesResponse->Capabilities->Device->Security->SAMLToken) ? "Y" : "N");
			processEventLog(__FILE__, __LINE__, stdout, "KerberosToken:%s", (tds__GetCapabilitiesResponse->Capabilities->Device->Security->KerberosToken) ? "Y" : "N");
			processEventLog(__FILE__, __LINE__, stdout, "RELToken:%s", (tds__GetCapabilitiesResponse->Capabilities->Device->Security->RELToken) ? "Y" : "N");
		}

		if (tds__GetCapabilitiesResponse->Capabilities->Events != NULL)
		{
			processEventLog(__FILE__, __LINE__, stdout, "-------------------Events-------------------");
			processEventLog(__FILE__, __LINE__, stdout, "XAddr:%s", tds__GetCapabilitiesResponse->Capabilities->Events->XAddr.c_str());
			processEventLog(__FILE__, __LINE__, stdout, "WSSubscriptionPolicySupport:%s", (tds__GetCapabilitiesResponse->Capabilities->Events->WSSubscriptionPolicySupport) ? "Y" : "N");
			processEventLog(__FILE__, __LINE__, stdout, "WSPullPointSupport:%s", (tds__GetCapabilitiesResponse->Capabilities->Events->WSPullPointSupport) ? "Y" : "N");
			processEventLog(__FILE__, __LINE__, stdout, "WSPausableSubscriptionManagerInterfaceSupport:%s", 
																				 (tds__GetCapabilitiesResponse->Capabilities->Events->WSPausableSubscriptionManagerInterfaceSupport) ? "Y" : "N");

			proxyEvent.soap_endpoint = tds__GetCapabilitiesResponse->Capabilities->Events->XAddr.c_str();
		}

		if (tds__GetCapabilitiesResponse->Capabilities->Imaging != NULL)
		{
			processEventLog(__FILE__, __LINE__, stdout, "-------------------Imaging-------------------");
			processEventLog(__FILE__, __LINE__, stdout, "XAddr:%s", tds__GetCapabilitiesResponse->Capabilities->Imaging->XAddr.c_str());
		}

		if (tds__GetCapabilitiesResponse->Capabilities->Media != NULL)
		{
			processEventLog(__FILE__, __LINE__, stdout, "-------------------Media-------------------");
			processEventLog(__FILE__, __LINE__, stdout, "XAddr:%s", tds__GetCapabilitiesResponse->Capabilities->Media->XAddr.c_str());

			processEventLog(__FILE__, __LINE__, stdout, "-------------------streaming-------------------");
			processEventLog(__FILE__, __LINE__, stdout, "RTPMulticast:%s", (tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTPMulticast) ? "Y" : "N");
			processEventLog(__FILE__, __LINE__, stdout, "RTP_TCP:%s", (tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTP_USCORETCP) ? "Y" : "N");
			processEventLog(__FILE__, __LINE__, stdout, "RTP_RTSP_TCP:%s", (tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTP_USCORERTSP_USCORETCP) ? "Y" : "N");

			proxyMedia.soap_endpoint = tds__GetCapabilitiesResponse->Capabilities->Media->XAddr.c_str();
		}

		if (tds__GetCapabilitiesResponse->Capabilities->PTZ != NULL)
		{
			processEventLog(__FILE__, __LINE__, stdout, "-------------------PTZ-------------------");
			processEventLog(__FILE__, __LINE__, stdout, "XAddr:%s", tds__GetCapabilitiesResponse->Capabilities->PTZ->XAddr.c_str());

			proxyPTZ.soap_endpoint = tds__GetCapabilitiesResponse->Capabilities->PTZ->XAddr.c_str();
			blSupportPTZ = true;
		}
	}
	else
	{
		PrintErr(proxyDevice.soap);
	}

	soap_destroy(soap); 
	soap_end(soap); 

	if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(proxyDevice.soap, NULL, "admin", DEV_PASSWORD))
	{
		return -1;
	}

	_tds__GetDeviceInformation *tds__GetDeviceInformation = soap_new__tds__GetDeviceInformation(soap, -1);
	_tds__GetDeviceInformationResponse *tds__GetDeviceInformationResponse = soap_new__tds__GetDeviceInformationResponse(soap, -1);

	if (SOAP_OK == proxyDevice.GetDeviceInformation(tds__GetDeviceInformation, tds__GetDeviceInformationResponse))
	{
		processEventLog(__FILE__, __LINE__, stdout, "-------------------DeviceInformation-------------------");
		processEventLog(__FILE__, __LINE__, stdout, "Manufacturer:%sModel:%s\r\nFirmwareVersion:%s\r\nSerialNumber:%s\r\nHardwareId:%s", tds__GetDeviceInformationResponse->Manufacturer.c_str(),
			tds__GetDeviceInformationResponse->Model.c_str(), tds__GetDeviceInformationResponse->FirmwareVersion.c_str(),
			tds__GetDeviceInformationResponse->SerialNumber.c_str(), tds__GetDeviceInformationResponse->HardwareId.c_str());
	}
	else
	{
		PrintErr(proxyDevice.soap);
	}

	soap_destroy(soap); 
	soap_end(soap); 

	if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(proxyDevice.soap, NULL, "admin", DEV_PASSWORD))
	{
		return -1;
	}

	_tds__GetNetworkInterfaces *tds__GetNetworkInterfaces = soap_new__tds__GetNetworkInterfaces(soap, -1);
	_tds__GetNetworkInterfacesResponse *tds__GetNetworkInterfacesResponse = soap_new__tds__GetNetworkInterfacesResponse(soap, -1);

	if (SOAP_OK == proxyDevice.GetNetworkInterfaces(tds__GetNetworkInterfaces, tds__GetNetworkInterfacesResponse))
	{
		processEventLog(__FILE__, __LINE__, stdout, "-------------------NetworkInterfaces-------------------");
		processEventLog(__FILE__, __LINE__, stdout, "%s", tds__GetNetworkInterfacesResponse->NetworkInterfaces[0]->token.c_str());
		processEventLog(__FILE__, __LINE__, stdout, "%s", tds__GetNetworkInterfacesResponse->NetworkInterfaces[0]->Info->HwAddress.c_str());
	}
	else
	{
		PrintErr(proxyDevice.soap);
	}
  #if 0
  string key = tds__GetNetworkInterfacesResponse->NetworkInterfaces[0]->Info->HwAddress;
  string value = szHostName;
  map<string,string>::iterator it = s_dev.find(key);
  if(it ==s_dev.end()){
    s_dev.insert(pair<string,string>(key,value));
  }
  #endif
	soap_destroy(soap);
	soap_end(soap);


	if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(proxyMedia.soap, NULL, "admin", DEV_PASSWORD))
	{
		return -1;
	}

	if (SOAP_OK != soap_wsse_add_Timestamp(proxyMedia.soap, "Time", 10)) 
	{
		return -1;
	}

	_trt__GetProfiles *trt__GetProfiles = soap_new__trt__GetProfiles(soap, -1);
	_trt__GetProfilesResponse *trt__GetProfilesResponse = soap_new__trt__GetProfilesResponse(soap, -1);

	if (SOAP_OK == proxyMedia.GetProfiles(trt__GetProfiles, trt__GetProfilesResponse))
	{
		_trt__GetStreamUri *trt__GetStreamUri = soap_new__trt__GetStreamUri(soap, -1);
		trt__GetStreamUri->StreamSetup = soap_new_tt__StreamSetup(soap, -1);
		trt__GetStreamUri->StreamSetup->Stream = tt__StreamType__RTP_Unicast;
		trt__GetStreamUri->StreamSetup->Transport = soap_new_tt__Transport(soap, -1);
		trt__GetStreamUri->StreamSetup->Transport->Protocol = tt__TransportProtocol__RTSP;

		_trt__GetStreamUriResponse *trt__GetStreamUriResponse = soap_new__trt__GetStreamUriResponse(soap, -1);

		processEventLog(__FILE__, __LINE__, stdout, "-------------------MediaProfiles-------------------");
		for (int i = 0; i < trt__GetProfilesResponse->Profiles.size(); i++)
		{
			processEventLog(__FILE__, __LINE__, stdout, "profile%d:%s Token:%s\r", i, trt__GetProfilesResponse->Profiles[i]->Name.c_str(), trt__GetProfilesResponse->Profiles[i]->token.c_str());
			trt__GetStreamUri->ProfileToken = trt__GetProfilesResponse->Profiles[i]->token;

      if(proxyPTZ.soap_endpoint){
			  if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(proxyPTZ.soap, NULL, "admin", DEV_PASSWORD))
			  {
				  return -1;
			  }

        //add test
        ptzContinuousMove(&proxyPTZ, trt__GetStreamUri->ProfileToken, 0);
      }

			if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(proxyMedia.soap, NULL, "admin", DEV_PASSWORD))
			{
				return -1;
			}

			if (SOAP_OK == proxyMedia.GetStreamUri(trt__GetStreamUri, trt__GetStreamUriResponse))
			{
				processEventLog(__FILE__, __LINE__, stdout, "RTSP URI:%s", trt__GetStreamUriResponse->MediaUri->Uri.c_str());
			}
			else
			{
				PrintErr(proxyMedia.soap);
			}
		}
	}
	else
	{
		PrintErr(proxyMedia.soap);
	}

	soap_destroy(soap); 
	soap_end(soap); 

	if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(proxyMedia.soap, NULL, "admin", DEV_PASSWORD))
	{
		return -1;
	}

	_trt__GetVideoEncoderConfigurations *trt__GetVideoEncoderConfigurations = soap_new__trt__GetVideoEncoderConfigurations(soap, -1);
	_trt__GetVideoEncoderConfigurationsResponse *trt__GetVideoEncoderConfigurationsResponse = soap_new__trt__GetVideoEncoderConfigurationsResponse(soap, -1);

	if (SOAP_OK == proxyMedia.GetVideoEncoderConfigurations(trt__GetVideoEncoderConfigurations, trt__GetVideoEncoderConfigurationsResponse))
	{
		 processEventLog(__FILE__, __LINE__, stdout, "-------------------VideoEncoderConfigurations-------------------");

		for (int i = 0; i < trt__GetVideoEncoderConfigurationsResponse->Configurations.size(); i++)
		{
            if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(proxyMedia.soap, NULL, "admin", DEV_PASSWORD))
			{
				return -1;
			}
			_trt__GetVideoEncoderConfiguration *trt__GetVideoEncoderConfiguration = soap_new__trt__GetVideoEncoderConfiguration(soap, -1);
			_trt__GetVideoEncoderConfigurationResponse *trt__GetVideoEncoderConfigurationResponse = soap_new__trt__GetVideoEncoderConfigurationResponse(soap, -1);

			trt__GetVideoEncoderConfiguration->ConfigurationToken = trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->token;

			if (SOAP_OK == proxyMedia.GetVideoEncoderConfiguration(trt__GetVideoEncoderConfiguration, trt__GetVideoEncoderConfigurationResponse))
			{

			}
			else
			{
				PrintErr(proxyMedia.soap);
			}

			 processEventLog(__FILE__, __LINE__, stdout, "Encoding:%s", 
					 					(trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->Encoding == tt__VideoEncoding__JPEG) ? "tt__VideoEncoding__JPEG" : 
										(trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->Encoding == tt__VideoEncoding__MPEG4) ? "tt__VideoEncoding__MPEG4" :
										(trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->Encoding == tt__VideoEncoding__H264) ? "tt__VideoEncoding__H264" : "Error VideoEncoding");
			 processEventLog(__FILE__, __LINE__, stdout, "name:%s UseCount:%d token:%s\r\n", trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->Name.c_str(),
										trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->UseCount, trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->token.c_str());
			 processEventLog(__FILE__, __LINE__, stdout, "Width:%d Height:%d\r\n", trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->Resolution->Width,
					 					trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->Resolution->Height);

			if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(proxyMedia.soap, NULL, "admin", DEV_PASSWORD))
			{
				return -1;
			}

			_trt__GetVideoEncoderConfigurationOptions *trt__GetVideoEncoderConfigurationOptions = soap_new__trt__GetVideoEncoderConfigurationOptions(soap, -1);
			_trt__GetVideoEncoderConfigurationOptionsResponse *trt__GetVideoEncoderConfigurationOptionsResponse = soap_new__trt__GetVideoEncoderConfigurationOptionsResponse(soap, -1);

			trt__GetVideoEncoderConfigurationOptions->ConfigurationToken = &trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->token;

			if (SOAP_OK == proxyMedia.GetVideoEncoderConfigurationOptions(trt__GetVideoEncoderConfigurationOptions, trt__GetVideoEncoderConfigurationOptionsResponse))
			{
			}
			else
			{
				PrintErr(proxyMedia.soap);
			}
		}
	}
	else
	{
		PrintErr(proxyMedia.soap);
	}


	soap_destroy(soap);
	soap_end(soap); 
	
	if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(proxyEvent.soap, NULL, "admin", DEV_PASSWORD))
	{
		return -1;
	}

	if (SOAP_OK != soap_wsse_add_Timestamp(proxyEvent.soap, "Time", 10)) 
	{
		return -1;
	}

	_tev__GetEventProperties *tev__GetEventProperties = soap_new__tev__GetEventProperties(soap, -1);
	_tev__GetEventPropertiesResponse *tev__GetEventPropertiesResponse = soap_new__tev__GetEventPropertiesResponse(soap, -1);

	if (SOAP_OK != soap_wsa_request(proxyEvent.soap, NULL, NULL, "http://www.onvif.org/ver10/events/wsdl/EventPortType/GetEventPropertiesRequest"))
	{
		return -1;
	}

	if (proxyEvent.GetEventProperties(tev__GetEventProperties, tev__GetEventPropertiesResponse) == SOAP_OK)
	{
		 processEventLog(__FILE__, __LINE__, stdout, "-------------------EventProperties-------------------");

		for (int i = 0; i < tev__GetEventPropertiesResponse->TopicNamespaceLocation.size(); i++)
		{
			 processEventLog(__FILE__, __LINE__, stdout, "TopicNamespaceLocation[%d]:%s", i, tev__GetEventPropertiesResponse->TopicNamespaceLocation[i].c_str());
		}
		

		for (int i = 0; i < tev__GetEventPropertiesResponse->MessageContentFilterDialect.size(); i++)
		{
			 processEventLog(__FILE__, __LINE__, stdout, "MessageContentFilterDialect[%d]:%s", i, tev__GetEventPropertiesResponse->MessageContentFilterDialect[i].c_str());
		}

		for (int i = 0; i < tev__GetEventPropertiesResponse->MessageContentSchemaLocation.size(); i++)
		{
			 processEventLog(__FILE__, __LINE__, stdout, "MessageContentSchemaLocation[%d]:%s", i, tev__GetEventPropertiesResponse->MessageContentSchemaLocation[i].c_str());
		}
	}
	else
	{
		PrintErr(proxyEvent.soap);
	}

	soap_destroy(soap);
	soap_end(soap); 
	
	return 0;
}
#endif

#define SOAP_TO         "urn:schemas-xmlsoap-org:ws:2005:04:discovery"
#define SOAP_ACTION     "http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe"

#define SOAP_MCAST_ADDR "soap.udp://239.255.255.250:3702"                       // onvif规定的组播地址

#define SOAP_ITEM       ""                                                      // 寻找的设备范围
#define SOAP_TYPES      "dn:NetworkVideoTransmitter"                            // 寻找的设备类型


OnvifDevice::OnvifDevice(string url,string username,string passwd):m_deviceurl(url),
m_username(username),
m_passwd(passwd){

}

OnvifDevice::~OnvifDevice(){

}


int OnvifDevice::ptzRelativeMove(/*const char* deviceAddr,*/int command)
{
  PTZBindingProxy proxyPTZ;
  proxyPTZ.soap_endpoint = m_deviceurl.c_str();
  
   std::string strMediaAddr;
   int result = 0;
   result = getMediaUrl(strMediaAddr);
   if (result!=SOAP_OK){
     return -1;
   }
   std::string strProfileToken;
   result = getProfile(strMediaAddr.c_str(), strProfileToken);
   if (result!=SOAP_OK){
     return -1;
   }

   soap_register_plugin(proxyPTZ.soap, soap_wsse);
   if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(proxyPTZ.soap, NULL, m_username.c_str(), m_passwd.c_str()))
   {
     return -1;
   }
  _tptz__RelativeMove ptz_req;
  _tptz__RelativeMoveResponse ptz_resp;
  //memset(&ptz_req, 0x0, sizeof(ptz_req));
  //memset(&ptz_resp, 0x0, sizeof(ptz_resp));

  ptz_req.ProfileToken = strProfileToken;
  ptz_req.Speed = soap_new_tt__PTZSpeed(proxyPTZ.soap, -1);;
  //memset(ptz_req.Translation, 0x0, sizeof(tt__PTZVector));
  ptz_req.Speed->PanTilt = (struct tt__Vector2D *)soap_new_tt__Vector2D(proxyPTZ.soap, -1);
  //memset(ptz_req.Translation->PanTilt, 0x0, sizeof(tt__Vector2D));
  //std::string str="http://www.onvif.org/ver10/tptz/PanTiltSpaces/VelocityGenericSpace";
  //ptz_req.Translation->PanTilt->space =&str;
  ptz_req.Translation = soap_new_tt__PTZVector(proxyPTZ.soap, -1);
  ptz_req.Translation->PanTilt = (struct tt__Vector2D *)soap_new_tt__Vector2D(proxyPTZ.soap, -1);
  tt__Vector1D* zoom = (struct tt__Vector1D *)soap_new_tt__Vector1D(proxyPTZ.soap, -1);
  ptz_req.Translation->Zoom = zoom;
  std::string str="http://www.onvif.org/ver10/tptz/PanTiltSpaces/TranslationGenericSpace";
  ptz_req.Translation->PanTilt->space =&str;
  std::string str2 ="http://www.onvif.org/ver10/tptz/ZoomSpaces/TranslationGenericSpace";
  ptz_req.Translation->Zoom->space = &str2;
  //memset(ptz_req.Translation->Zoom, 0x0, sizeof(tt__Vector1D));

  switch (command)
  {
  case LEFT:
    //ptz_req.Speed->PanTilt->x = -((float)speed / 10);
    ptz_req.Translation->PanTilt->x = -((float)speed / 10);
    ptz_req.Translation->PanTilt->y = 0;
    break;
  case RIGHT:
    //ptz_req.Speed->PanTilt->x = ((float)speed / 10);
    ptz_req.Translation->PanTilt->x = ((float)speed / 10);
    ptz_req.Translation->PanTilt->y = 0;
    break;
  case UP:
    ptz_req.Translation->PanTilt->x = 0;
    //ptz_req.Speed->PanTilt->y = ((float)speed / 10);
    ptz_req.Translation->PanTilt->y = ((float)speed / 10);
    break;
  case DOWN:
    ptz_req.Translation->PanTilt->x = 0;
    //ptz_req.Speed->PanTilt->y = -((float)speed / 10);
    ptz_req.Translation->PanTilt->y = -((float)speed / 10);
    break;
  case ZOOMIN:
    ptz_req.Translation->Zoom->x = ((float)speed / 10);
    break;
  case ZOOMOUT:
    ptz_req.Translation->Zoom->x = -((float)speed / 10);
    break;
  default:
    break;
  }
  if (proxyPTZ.RelativeMove(proxyPTZ.soap_endpoint,NULL,&ptz_req, &ptz_resp) == SOAP_OK)
    //转动成功
    //std::cout<<"\nsuccess\n";
    return 0;
  else
    //std::cout<<"\nfail\n";
    return -1;
}

int OnvifDevice::ptzContinuousMove(/*const char* deviceAddr,*/int command)
{

 PTZBindingProxy proxyPTZ;
  proxyPTZ.soap_endpoint = m_deviceurl.c_str();

  std::string strMediaAddr;
	int result = 0;
	result = getMediaUrl(strMediaAddr);
	if (result!=SOAP_OK){
		return -1;
	}
	std::string strProfileToken;
	result = getProfile(strMediaAddr.c_str(), strProfileToken);
	if (result!=SOAP_OK){
		return -1;
	}

	soap_register_plugin(proxyPTZ.soap, soap_wsse);
	if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(proxyPTZ.soap, NULL,  m_username.c_str(), m_passwd.c_str()))
	{
		return -1;
	}

  _tptz__ContinuousMove continuousMove;// = soap_new__tptz__ContinuousMove(proxyDevice.soap, -1);
  _tptz__ContinuousMoveResponse response;// = soap_new__tptz__ContinuousMoveResponse(proxyDevice.soap, -1);
  continuousMove.ProfileToken = const_cast<char *>(strProfileToken.c_str());
  tt__PTZSpeed* velocity = soap_new_tt__PTZSpeed(proxyPTZ.soap, -1);
  continuousMove.Velocity = velocity;
  tt__Vector2D* panTilt = soap_new_tt__Vector2D(proxyPTZ.soap, -1);
  continuousMove.Velocity->PanTilt = panTilt;
  std::string str="http://www.onvif.org/ver10/tptz/PanTiltSpaces/VelocityGenericSpace";
  continuousMove.Velocity->PanTilt->space =&str;
  tt__Vector1D* zoom = soap_new_tt__Vector1D(proxyPTZ.soap, -1);
  continuousMove.Velocity->Zoom = zoom;
  switch (command)
  {
  case LEFT:
    continuousMove.Velocity->PanTilt->x = -((float)speed / 10);
    continuousMove.Velocity->PanTilt->y = 0;
    break;
  case RIGHT:
    continuousMove.Velocity->PanTilt->x = ((float)speed / 10);
    continuousMove.Velocity->PanTilt->y = 0;
    break;
  case UP:
    continuousMove.Velocity->PanTilt->x = 0;
    continuousMove.Velocity->PanTilt->y = ((float)speed / 10);
    break;
  case DOWN:
    continuousMove.Velocity->PanTilt->x = 0;
    continuousMove.Velocity->PanTilt->y = -((float)speed / 10);
    break;
  case ZOOMIN:
    continuousMove.Velocity->Zoom->x = ((float)speed / 10);
    break;
  case ZOOMOUT:
    continuousMove.Velocity->Zoom->x = -((float)speed / 10);
    break;
  default:
    break;
  }
  //wait for test
  if (proxyPTZ.ContinuousMove(proxyPTZ.soap_endpoint,NULL,&continuousMove, &response) == SOAP_OK)
    //转动成功
    //std::cout<<"\nsuccess\n";
    return 0;
  else
    //std::cout<<"\nfail\n";
    return -1;
}



int OnvifDevice::getRTSPUrl(const char*mediaAddr,string&rtspUrl){
  if(mediaAddr==NULL){
    return -1;
  }
  MediaBindingProxy proxyMedia;
  proxyMedia.soap_endpoint = mediaAddr;
  soap_register_plugin(proxyMedia.soap, soap_wsse);
	if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(proxyMedia.soap, NULL, m_username.c_str(), m_passwd.c_str()))
	{
		return -1;
	}

  tt__Transport               transport;
  transport.Tunnel = NULL;
  transport.Protocol = tt__TransportProtocol__RTSP;
  
  tt__StreamSetup             setup;
  setup.Transport = &transport;
  setup.Stream = tt__StreamType__RTP_Unicast;
  
  _trt__GetStreamUri          StreamUri_req;
  _trt__GetStreamUriResponse  StreamUri_rep;
  
  StreamUri_req.StreamSetup = &setup;

  string profileToken;
  getProfile(mediaAddr,profileToken);
  StreamUri_req.ProfileToken = profileToken;

  
  int result = proxyMedia.GetStreamUri(mediaAddr, NULL, &StreamUri_req, &StreamUri_rep);
  if (SOAP_OK != result)
  {
      //printf("GetStreamUri error.\n");
      return -1;
  }
  rtspUrl = StreamUri_rep.MediaUri->Uri;
  return SOAP_OK;
}
int OnvifDevice::getProfile(const char*mediaAddr,string&profileToken){
  if(mediaAddr==NULL){
    return -1;
  }
  MediaBindingProxy proxyMedia;
  proxyMedia.soap_endpoint = mediaAddr;
  soap_register_plugin(proxyMedia.soap, soap_wsse);
	if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(proxyMedia.soap, NULL, m_username.c_str(), m_passwd.c_str()))
	{
		return -1;
	}
  _trt__GetProfiles           Profiles_req;
  //存储的是获取回来的信息
  _trt__GetProfilesResponse   Profiles_rep;
  int result = proxyMedia.GetProfiles(mediaAddr, NULL, &Profiles_req, &Profiles_rep);
  if (SOAP_OK != result)
  {
      //printf("GetProfiles error.\n");		
      return -1;
  }
  ///< 可能会有好几路流，相应的也会有好几个profile,这里只取第一路码流
  profileToken = Profiles_rep.Profiles[0]->token;
  return SOAP_OK;
}

int OnvifDevice::getMediaUrl(/* const char* deviceAddr,*/string& mediaAddr){
  DeviceBindingProxy proxyDevice;
 
  proxyDevice.soap_endpoint = m_deviceurl.c_str();

	soap_register_plugin(proxyDevice.soap, soap_wsse);
	if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(proxyDevice.soap, NULL, m_username.c_str(), m_passwd.c_str()))
	{
		return -1;
	}
/*
	_tds__GetCapabilities *tds__GetCapabilities = soap_new__tds__GetCapabilities(soap, -1);
	tds__GetCapabilities->Category.push_back(tt__CapabilityCategory__All);

	_tds__GetCapabilitiesResponse *tds__GetCapabilitiesResponse = soap_new__tds__GetCapabilitiesResponse(soap, -1);
*/
  _tds__GetCapabilities           Bilities_req;
  _tds__GetCapabilitiesResponse   Bilities_rep;
  int result = proxyDevice.GetCapabilities(m_deviceurl.c_str(), NULL, &Bilities_req, &Bilities_rep);

  if (SOAP_OK != result)
  {
      //printf("GetCapabilities error.\n");
      return -1;
  }
  else{
    if (Bilities_rep.Capabilities == NULL
      || Bilities_rep.Capabilities->Media == NULL)
    {
      //printf("no capability\n");
      return -1;
    }
  }
  mediaAddr = Bilities_rep.Capabilities->Media->XAddr;
  //printf("mediaAddr %s\n",mediaAddr.c_str());
  return SOAP_OK;
}

int OnvifDevice::getPTZUrl(string& PTZAddr){
  DeviceBindingProxy proxyDevice;
  proxyDevice.soap_endpoint = m_deviceurl.c_str();

	soap_register_plugin(proxyDevice.soap, soap_wsse);
	if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(proxyDevice.soap, NULL, m_username.c_str(), m_passwd.c_str()))
	{
		return -1;
	}
/*
	_tds__GetCapabilities *tds__GetCapabilities = soap_new__tds__GetCapabilities(soap, -1);
	tds__GetCapabilities->Category.push_back(tt__CapabilityCategory__All);

	_tds__GetCapabilitiesResponse *tds__GetCapabilitiesResponse = soap_new__tds__GetCapabilitiesResponse(soap, -1);
*/
  _tds__GetCapabilities           Bilities_req;
  _tds__GetCapabilitiesResponse   Bilities_rep;
  int result = proxyDevice.GetCapabilities(m_deviceurl.c_str(), NULL, &Bilities_req, &Bilities_rep);

  if (SOAP_OK != result)
  {
      //printf("GetCapabilities error.\n");
      return -1;
  }
  else{
    if (Bilities_rep.Capabilities == NULL
      || Bilities_rep.Capabilities->PTZ == NULL)
    {
      //printf("no capability\n");
      return -1;
    }
  }
  PTZAddr = Bilities_rep.Capabilities->PTZ->XAddr;
  //printf("PTZAddr %s\n",PTZAddr.c_str());
  return SOAP_OK;
}



void detectDevice( vector<string>&vecDevAddr)
{
  s_dev.clear();
  struct soap *soap = soap_new();
  soap_set_namespaces(soap, namespaces);                                      // 设置soap的namespaces
  soap->recv_timeout    = 10;                                            // 设置超时（超过指定时间没有数据就退出）
  soap->send_timeout    = 10;
  soap->connect_timeout = 10;
  soap->socket_flags = MSG_NOSIGNAL;
  soap_set_mode(soap, SOAP_C_UTFSTRING);

  struct SOAP_ENV__Header *header = NULL;
  header = (struct SOAP_ENV__Header *)soap_malloc(soap, sizeof(struct SOAP_ENV__Header));
  //assert(NULL != header);
  memset(header, 0x00 ,sizeof(struct SOAP_ENV__Header));
  soap_default_SOAP_ENV__Header(soap, header);

  header->wsa__MessageID = (char*)soap_wsa_rand_uuid(soap);
  header->wsa__To        = (char*)soap_malloc(soap, strlen(SOAP_TO) + 1);
  memset(header->wsa__To, 0, strlen(SOAP_TO) + 1);
  header->wsa__Action    = (char*)soap_malloc(soap, strlen(SOAP_ACTION) + 1);
  memset(header->wsa__Action, 0, strlen(SOAP_ACTION) + 1);
  strcpy(header->wsa__To, SOAP_TO);
  strcpy(header->wsa__Action, SOAP_ACTION);
  soap->header = header;
  
  struct wsdd__ProbeType  req;

  struct wsdd__ScopesType *scope = NULL;                                      // 用于描述查找哪类的Web服务
  
  //assert(NULL != soap);
  
  scope = (struct wsdd__ScopesType *)soap_malloc(soap, sizeof(struct wsdd__ScopesType));
  memset(scope, 0, sizeof(struct wsdd__ScopesType));
  soap_default_wsdd__ScopesType(soap, scope);                                 // 设置寻找设备的范围
  scope->__item = (char*)soap_malloc(soap, strlen(SOAP_ITEM) + 1);
  memset(scope->__item, 0, strlen(SOAP_ITEM) + 1);
  strcpy(scope->__item, SOAP_ITEM);
  
  memset(&req, 0x00, sizeof(struct wsdd__ProbeType));
  soap_default_wsdd__ProbeType(soap, &req);
  req.Scopes = scope;
  req.Types  = (char*)soap_malloc(soap, strlen(SOAP_TYPES) + 1);     // 设置寻找设备的类型
  memset(req.Types, 0, strlen(SOAP_TYPES) + 1);
  strcpy(req.Types, SOAP_TYPES);
  
  int result = soap_send___wsdd__Probe(soap, SOAP_MCAST_ADDR, NULL, &req);   

  struct __wsdd__ProbeMatches rep; 
  struct wsdd__ProbeMatchType *probeMatch;
  unsigned int count = 0;
  int i;


  std::string strXAddr;

  while (SOAP_OK == result)                                                   // 开始循环接收设备发送过来的消息
  {
      memset(&rep, 0x00, sizeof(rep));
      result = soap_recv___wsdd__ProbeMatches(soap, &rep);
      if (SOAP_OK == result) {
          if (!soap->error) {
          //    printf("[soap] %s error: %d, %s, %s\n", "ProbeMatches",soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
          //} else {                                                            // 成功接收到设备的应答消息
              //dump__wsdd__ProbeMatches(&rep);
              //printf("ADD %s\n",rep.wsdd__ProbeMatches->ProbeMatch->wsa__EndpointReference.Address);
              string key = rep.wsdd__ProbeMatches->ProbeMatch->wsa__EndpointReference.Address;
              vector<string>::iterator iter=find(s_dev.begin(),s_dev.end(),key);
              if (iter!=s_dev.end())
                continue;
              else
                s_dev.push_back(key);
              if (NULL != rep.wsdd__ProbeMatches) {
                  count += rep.wsdd__ProbeMatches->__sizeProbeMatch;
                  for(i = 0; i < rep.wsdd__ProbeMatches->__sizeProbeMatch; i++) {
                      probeMatch = rep.wsdd__ProbeMatches->ProbeMatch + i;
                      //printf("addr %s \n", probeMatch->XAddrs);                             // 使用设备服务地址执行函数回调
                      strXAddr = probeMatch->XAddrs;
                      vecDevAddr.push_back(strXAddr);
                      //getdevice(probeMatch->XAddrs);
                      
                  }
              }
          }
      } else if (soap->error) {
          break;
      }
  }
  //printf("\ndetect end! It has detected %d devices!\n", count);
  //printf("\ndetect end! It has detected %d devices!\n", s_dev.size());
  soap_destroy(soap);
  soap_end(soap);
}
