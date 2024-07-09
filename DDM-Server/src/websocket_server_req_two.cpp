#include "websocket_server_two.h"
#include "websocket_server_req_two.h"
HTTPRequestHandler* RequestHandlerFactoryTwo::createRequestHandler(const HTTPServerRequest& request) {

	return new WebSocketRequestHandlerTwo;
}