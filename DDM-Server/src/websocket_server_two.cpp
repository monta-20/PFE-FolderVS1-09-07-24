#include "websocket_server_two.h"

void WebSocketRequestHandlerTwo::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) {
	Object::Ptr jsonObjectt = new Object;
	std::ostringstream oss;
	Stringifier::stringify(*jsonObjectt, oss);
	std::string jsonString = oss.str();
	char buffer[1024];
	int flags;
	int n;

	try
	{

		WebSocket ws(request, response);

		do
		{
			n = ws.receiveFrame(buffer, sizeof(buffer), flags);
			string receivedData(buffer, n);
			processJSONDATA(receivedData, ws);
		} while (n > 0 && (flags & WebSocket::FRAME_OP_BITMASK) != WebSocket::FRAME_OP_CLOSE);
	}
	catch (WebSocketException& exc)
	{
		switch (exc.code())
		{
		case WebSocket::WS_ERR_HANDSHAKE_UNSUPPORTED_VERSION:
			response.set("Sec-WebSocket-Version", WebSocket::WEBSOCKET_VERSION);
			// fallthrough
		case WebSocket::WS_ERR_NO_HANDSHAKE:
		case WebSocket::WS_ERR_HANDSHAKE_NO_VERSION:
		case WebSocket::WS_ERR_HANDSHAKE_NO_KEY:
		case WebSocket::WS_ERR_HANDSHAKE_ACCEPT:
		case WebSocket::WS_ERR_UNAUTHORIZED:
		case WebSocket::WS_ERR_PAYLOAD_TOO_BIG:
		case WebSocket::WS_ERR_INCOMPLETE_FRAME:
			response.setStatusAndReason(HTTPServerResponse::HTTP_BAD_REQUEST);
			response.setContentLength(0);
			response.send();
			break;
		}
	}
	catch (JSONException& e) {
		cout << "Json Exception: " << e.message() << endl;
	}
}

void WebSocketRequestHandlerTwo::processJSONDATA(const string& Data, WebSocket& ws) {

	Parser parser;
	Poco::Dynamic::Var result = parser.parse(Data);
	Object::Ptr JSONObject = result.extract<Object::Ptr>();

	if (JSONObject->has("D0") && JSONObject->has("D1") && JSONObject->has("D2") && JSONObject->has("D3") && JSONObject->has("D4") &&
		JSONObject->has("D5") && JSONObject->has("D6") && JSONObject->has("D7")) {

		D0 = JSONObject->get("D0").toString();
		D1 = JSONObject->get("D1").toString();
		D2 = JSONObject->get("D2").toString();
		D3 = JSONObject->get("D3").toString();
		D4 = JSONObject->get("D4").toString();
		D5 = JSONObject->get("D5").toString();
		D6 = JSONObject->get("D6").toString();
		D7 = JSONObject->get("D7").toString();

		cout << "D0: " << D0 << endl;
		cout << "D1: " << D1 << endl;
		cout << "D2: " << D2 << endl;
		cout << "D3: " << D3 << endl;
		cout << "D4: " << D4 << endl;
		cout << "D5: " << D5 << endl;
		cout << "D6: " << D6 << endl;
		cout << "D7: " << D7 << endl;
		/*std::ostringstream oss;
		Stringifier::stringify(*responseObject, oss);
		string responseStr = oss.str();
		ws.sendFrame(responseStr.c_str(), responseStr.size(), WebSocket::FRAME_TEXT);*/
	}
}