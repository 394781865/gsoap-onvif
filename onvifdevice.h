#ifndef __ONVIFDEVICE_H
#define __ONVIFDEVICE_H


#include <iostream>
#include "stdio.h"

#include <string>
#include <vector>
using namespace std;


class OnvifDevice{
public:
  OnvifDevice(string url,string username,string passwd);
  ~OnvifDevice();
  int getMediaUrl(string& mediaAddr);
  int getPTZUrl(string& PTZAddr);
  int getProfile(const char*mediaAddr,string&profileToken);
  int getRTSPUrl(const char*mediaAddr,string&rtspUrl);
  int ptzContinuousMove(int command);
  int ptzRelativeMove(int command);

private:
  string m_deviceurl;
  string m_username;
  string m_passwd;
};

void detectDevice( vector<string>&vecDevAddr);

#endif

