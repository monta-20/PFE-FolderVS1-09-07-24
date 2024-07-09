#include "main_app.h"
#include <Poco/Net/HTTPServer.h>
#include <Poco/Util/ServerApplication.h>
#include "websocket_server_req.h" 
#include "websocket_server.h" 
#include "websocket_server_req_two.h" 
#include "websocket_server_two.h" 
#include "pcan_communication.h"
#include "Service.h"

using namespace Poco::Net;
using namespace Poco::Util;
using namespace std;


void MyServer::initialize(Application& self)
 {
     loadConfiguration(); // Load default configuration
     ServerApplication::initialize(self);
 }

 void MyServer:: uninitialize()
 {
     ServerApplication::uninitialize();
 }


 int MyServer::main(const vector<string>& args)
 {
     HTTPServerParams* pParams = new HTTPServerParams;
     pParams->setMaxQueued(100);//maximum send 100 to the server and wait for the Queue became available
     pParams->setMaxThreads(16);
     int time = 520000;
     HTTPServer srv(new RequestHandlerFactory, ServerSocket(1234), pParams);
     //HTTPServer srvTwo(new RequestHandlerFactoryTwo, ServerSocket(8080), pParams);
     
     
      srv.start();
     //srvTwo.start();
     
     Sleep(time);
     Communication c;
    
     c.notify("1", "250", 4, {1,3,4}, 8);
     cout << "Server started on port 1234" << endl;
     waitForTerminationRequest();
     srv.stop();
     //srvTwo.stop();
     return Application::EXIT_OK;
     
 }


 int main(int argc, char** argv)
 {
     MyServer app;
     return app.run(argc, argv);
 }