CXX =/usr/local/gcc-6.3.0/bin/g++
CPPFLAG = -Wall  -fexceptions -fPIC -O2 -DWITH_NONAMESPACES  -DWITH_DOM  -DWITH_OPENSSL -DSOAP_DEBUG 

BASE_DIR=.
SOURCE=$(BASE_DIR)

INCLUDE +=-I$(SOURCE)/include -I$(BASE_DIR)
LIB= -lssl -lcrypto
PROXYSOURCE=$(BASE_DIR)/proxycpp
ProxyOBJ=$(PROXYSOURCE)/soapDeviceBindingProxy.o $(PROXYSOURCE)/soapMediaBindingProxy.o $(PROXYSOURCE)/soapPTZBindingProxy.o \
		 $(PROXYSOURCE)/soapPullPointSubscriptionBindingProxy.o $(PROXYSOURCE)/soapRemoteDiscoveryBindingProxy.o
PluginSOURCE=$(BASE_DIR)/plugin
PluginOBJ=$(PluginSOURCE)/wsaapi.o $(PluginSOURCE)/wsseapi.o $(PluginSOURCE)/threads.o $(PluginSOURCE)/duration.o \
		  $(PluginSOURCE)/smdevp.o $(PluginSOURCE)/mecevp.o $(PluginSOURCE)/dom.o
SRC= $(SOURCE)/stdsoap2.o  $(SOURCE)/soapC.o $(SOURCE)/soapClient.o  $(PluginOBJ) $(ProxyOBJ)
DEV= $(SOURCE)/onvifdevice.o
OBJECTS = $(patsubst %.cpp,%.o,$(SRC))
TARGET=libipconvif.so
test: $(TARGET) 
	$(CXX)   $(INCLUDE) $(CPPFLAG) $(SOURCE)/main.cpp -o $@  $(TARGET) $(LIB)
$(TARGET):$(OBJECTS) $(SOURCE)/onvifdevice.o
	$(CXX) -shared $(CPPFLAG) $(OBJECTS)  $(SOURCE)/onvifdevice.o $(INCLUDE)  $(LIB) -o $(TARGET)
$(OBJECTS):%.o : %.cpp
	$(CXX) -c $(CPPFLAG) $(INCLUDE) $< -o $@
$(DEV):%.o : %.cpp
	$(CXX) -c $(CPPFLAG) $(INCLUDE) $< -o $@
cleantest:
	rm -rf $(SOURCE)/main.o test
cleanonvif:
	rm -rf $$(SOURCE)/onvifdevice.o $(TARGET)  $(SOURCE)/main.o test
clean:
	rm -rf  $(OBJECTS) $(TARGET) $(SOURCE)/main.o test
