#include "onvifdevice.h"

#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3
#define ZOOMIN 4
#define ZOOMOUT 5

int main(int argc,char*argv[]){
  vector<string>addr;
  detectDevice(addr);
  string mediaAddr;
  string profileToken;
  string rtspUrl;
  string PTZUrl;
  //addr.push_back("http://192.168.2.128/onvif/device_service");
  for(vector<string>::iterator it = addr.begin();it!=addr.end();++it)
  {
    cout<<*it<<endl;
    OnvifDevice onvifDevice(*it,"admin","hik123456");
    onvifDevice.getMediaUrl(mediaAddr);
    cout<<mediaAddr<<endl;
    onvifDevice.getProfile(mediaAddr.c_str(),profileToken);
    cout<<profileToken<<endl;
    onvifDevice.getRTSPUrl(mediaAddr.c_str(),rtspUrl);
    cout<<rtspUrl<<endl;

    onvifDevice.getPTZUrl(PTZUrl);
    //onvifDevice.ptzContinuousMove(it->c_str(),DOWN);
    //onvifDevice.ptzContinuousMove(it->c_str(),ZOOMOUT);
    onvifDevice.ptzRelativeMove(DOWN);
    onvifDevice.ptzRelativeMove(ZOOMOUT);
    onvifDevice.ptzRelativeMove(RIGHT);
  }
return 0;
}
