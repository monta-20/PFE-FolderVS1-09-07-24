#pragma once 
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Timespan.h>
#include <Poco/JSON/Object.h>
#include <Poco/Net/HTTPServer.h>
#include "Poco/Dynamic/Var.h"
#include <Poco/Net/WebSocket.h>
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include <iostream>
#include <Poco/Net/NetException.h>
#include <string>
#include <iomanip>
#include <Poco/JSON/Stringifier.h>


//Websocket Implementation
using namespace Poco::Net;
//Parsing Data Recieved from IHM
using namespace Poco::JSON;

using namespace Poco;
//Printing and Debogage code 
using namespace std;

class Communication;
class WebSocketRequestHandler : public HTTPRequestHandler {
    friend class Communication;

    //Members
    string baudRate;
    string  channel;
    DWORD id;
    
    vector<BYTE> data;
    int dlc;
public:
    //handleRequest
    void processJsonData(const string&,WebSocket&);
    void processJson(const string& data);
    void handleRequest(HTTPServerRequest&, HTTPServerResponse&);
    //Conversion Type
    static DWORD hexStringToUint(const string&);
    WORD hexStringToUSint(const string&);
    static vector<BYTE> hexStringToByte(const string&);
    static int hexStringToInt(const string&);
     //Security Access 0x27 Get Key
     //unsigned short usMain_eSecKey_Calcul(unsigned short, unsigned short);
   //Getter
    string getbaudRate() const;
    string getchannel() const;
    DWORD getid() const ;
    vector<BYTE> getdata() const ;
    int getdlc()const ;
};