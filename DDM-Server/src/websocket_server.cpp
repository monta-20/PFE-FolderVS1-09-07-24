#include "websocket_server.h"
#include "Service.h"
#include "canMessage.h"
#include "pcan_communication.h"
//Conversion Function
DWORD WebSocketRequestHandler::hexStringToUint(const string& hexString) {
	string strippedString = hexString.substr(2); // Start from index 2 (after "0x")
	DWORD result = strtol(strippedString.c_str(), nullptr, 16); 
	return result;
}
WORD WebSocketRequestHandler::hexStringToUSint(const string& hexString) {
	return stoi(hexString, nullptr, 16);
}
vector<BYTE> WebSocketRequestHandler::hexStringToByte(const string& hexString) {
	vector<BYTE> bytes;
	string strippedString = hexString.substr(2);
	for (size_t i = 0; i < strippedString.size(); i += 2) {
		string byteString = strippedString.substr(i, 2);
		unsigned int byteValue = stoi(byteString, nullptr, 16);
		std::cout << "Data[" << i / 2 << "]=" << hex << byteValue<<endl;
		bytes.push_back(static_cast<BYTE>(byteValue));
	}
	return bytes;
}
int WebSocketRequestHandler::hexStringToInt(const string& hexString) {
	return stoi(hexString);
}

void WebSocketRequestHandler::handleRequest(HTTPServerRequest & request, HTTPServerResponse & response){
		char buffer[1024]; 
		int flags;
		int n;		
		try
		{

					WebSocket ws(request, response);
					// Set timeouts for the WebSocket connection
					ws.setReceiveTimeout(Timespan(60*60, 0)); // 180 seconds receive timeout
					ws.setSendTimeout(Timespan(60*60, 0)); // 180 seconds send timeout				
					do
					{
						n = ws.receiveFrame(buffer, sizeof(buffer), flags);
						string receivedData(buffer, n);
						processJsonData(receivedData, ws);
					} while (n > 0 && (flags & WebSocket::FRAME_OP_BITMASK) != WebSocket::FRAME_OP_CLOSE);
		}
		catch (WebSocketException& exc)
		{
			switch (exc.code())
			{
			case WebSocket::WS_ERR_HANDSHAKE_UNSUPPORTED_VERSION:
				
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

		catch (Poco::TimeoutException& e) {
			cout << "Timeout Exception: " << e.displayText() << endl;
			response.setStatusAndReason(HTTPServerResponse::HTTP_REQUEST_TIMEOUT);
			response.setContentLength(0);
			response.send();
		}
		catch (JSONException& e) {
			cout << "JSON Exception: " << e.message() << endl;
		}
		catch (Poco::Exception& e) {
			cout << "Poco Exception: " << e.displayText() << endl;
		}
		catch (std::exception& e) {
			cout << "Standard Exception: " << e.what() << endl;
		}
}

Object::Ptr parseJson(const string& data) {
		Parser parser;
		Poco::Dynamic::Var result = parser.parse(data);
		Object::Ptr jsonObject = result.extract<Object::Ptr>();
		
		return jsonObject;
}
void WebSocketRequestHandler::processJsonData(const string& Data,WebSocket& ws) {
	
	Parser parser;
	Poco::Dynamic::Var result = parser.parse(Data);
	Object::Ptr jsonObject = result.extract<Object::Ptr>();
	
	if (jsonObject->has("Baud Rate") && jsonObject->has("Channel") && jsonObject->has("ID") && jsonObject->has("Data") && jsonObject->has("DLC")) {

		baudRate = jsonObject->get("Baud Rate").toString();
		channel = jsonObject->get("Channel").toString();
		id = hexStringToUint(jsonObject->get("ID").toString());
		std::cout << "Converted DWORD value: 0x" << std::hex << id << std::endl;
		data = hexStringToByte(jsonObject->get("Data").toString());
		dlc = hexStringToInt(jsonObject->get("DLC").toString());

		cout << "Baud Rate: " << baudRate << endl;
		cout << "Channel: " << channel << endl;
		cout << "ID: " << id << endl;
		cout << "DLC: " << dlc << endl;
		Communication comm;
		canMessage mg;
		mg = comm.notify(baudRate, channel, id, data, dlc);
		Service s;
		s.SendDescription(baudRate, channel, id, data, dlc);

		
		string result;
		for (BYTE b : mg.data) {
			stringstream ss;
			ss << hex << setfill('0') << setw(2) << (int)b;
			result += ss.str();
		}
		
		cout << result;
		Service ser;		
		string sst = "0x" + ser.byteToHex(data[1]);
		string sstr = "0x0" + ser.byteToHex(data[2]);//au sub-func 0x10.. , because byteToHex return left number for 0 appear example 04->4
		string sstrr = "0x" + ser.byteToHex(data[2]) + ser.byteToHex(data[3]);
		cout <<"Value of DID " << sstrr << endl;;
		vector<Service> serv = ser.SendDescription(baudRate, channel, id, data, dlc);
		vector<Service> servR = ser.RecieveDescription(baudRate, channel, id, data, dlc);
		
		string ServiceDescription;
		string ServiceValue;

		string SBFName;
		string SBFValue;
		string SBFDescription;

		string DIDName;
		string DIDValue;
		string DIDDescription;

		for (const auto& service : serv) {
			if (sst == service.value) {
				std::cout << "Service Name: " << service.name << endl;
				std::cout << "Service Value: " << service.value << endl;
				std::cout << "Service Description: " << service.desc << endl;
				ServiceDescription = service.desc;
				ServiceValue = service.value;
				for (const auto& did : service.dids) {
					std::cout << "\tCategory: " << did.category << ", Size: " << did.size << endl;
				}
			}
			if (sst != "0x22" && sst != "0x2E" && sst==service.value) {
				for (const auto& subFunc : service.sbfs) {
					if (sstr == subFunc.value) {
						std::cout << "\tName: " << subFunc.name << ", Value: " << subFunc.value << ", Description: " << subFunc.description << endl;
						SBFName = subFunc.name;
						SBFValue = subFunc.value;
						SBFDescription = subFunc.description;
					}											     
				}
			}
			else {
				for (const auto& subFunc : service.sbfs) {
					if (sstrr == subFunc.value) {
						std::cout << "\tName: " << subFunc.name << ", Value: " << subFunc.value << ", Description: " << subFunc.description << endl;
						DIDName = subFunc.name;
						DIDValue = subFunc.value;
						DIDDescription = subFunc.description;
					}	
				}
			}
		}
		



		string SBFNameRes;
		string SBFValueRes;
		string SBFDescriptionRes;
		
		for (const auto& service : servR){
			if (("0x"+result.substr(2, 2)) != "0x7f"){
				for (const auto& subFunc : service.sbfs) {
					if (("0x"+result.substr(2, 2)) == subFunc.value) {						
						std::cout << "\tName: " << subFunc.name << ", Value: " << subFunc.value << ", Description: " << subFunc.description << endl;
						SBFNameRes = subFunc.name;
						SBFValueRes = subFunc.value;
						SBFDescriptionRes = subFunc.description;
					}
				}					
			}
			else {
				
				for (const auto& subFunc : service.sbfs) {
					if (("0x"+result.substr(6, 2)) == subFunc.value) {
						std::cout << "\tName: " << subFunc.name << ", Value: " << subFunc.value << ", Description: " << subFunc.description << endl;
						SBFNameRes = subFunc.name;
						SBFValueRes = subFunc.value;
						SBFDescriptionRes = subFunc.description;
					}
				}
			}			
		}
		Object::Ptr receivedObject = parseJson(Data);
		receivedObject->set("SRVName", ServiceDescription);
		receivedObject->set("SRVValue", ServiceValue);
		receivedObject->set("SBFName", SBFName);
		receivedObject->set("SBFValue", SBFValue);
		receivedObject->set("SBFDescription", SBFDescription);
		receivedObject->set("DIDName", DIDName);
		receivedObject->set("DIDValue", DIDValue);
		receivedObject->set("DIDDescription", DIDDescription);

		receivedObject->set("Id", mg.id);
		receivedObject->set("Dlc", mg.dlc);
		receivedObject->set("Data", result);

		receivedObject->set("SBFNameRes", SBFNameRes);
		receivedObject->set("SBFValueRes", SBFValueRes);
		receivedObject->set("SBFDescriptionRes", SBFDescriptionRes);

        Object::Ptr responseObject = new Object;
		responseObject->set("response", "Data received successfully!");
		responseObject->set("SRVName", ServiceDescription);
		responseObject->set("SRVValue", ServiceValue);
		responseObject->set("SBFName", SBFName);
		responseObject->set("SBFValue", SBFValue);
		responseObject->set("SBFDescription", SBFDescription);
		responseObject->set("DIDName", DIDName);
		responseObject->set("DIDValue", DIDValue);
		responseObject->set("DIDDescription", DIDDescription);

		responseObject->set("Id", mg.id);
		responseObject->set("Dlc", mg.dlc);
		responseObject->set("Data", result);

		responseObject->set("SBFNameRes", SBFNameRes);
		responseObject->set("SBFValueRes", SBFValueRes);
		responseObject->set("SBFDescriptionRes", SBFDescriptionRes);

		std::ostringstream oss;
		Stringifier::stringify(*responseObject, oss);
		string responseStr = oss.str();
		ws.sendFrame(responseStr.c_str(), responseStr.size(), WebSocket::FRAME_TEXT);
	}
	
}
//Getter Fuunctions

string WebSocketRequestHandler::getbaudRate() const {return baudRate; }
string WebSocketRequestHandler::getchannel() const  {return channel; }
DWORD WebSocketRequestHandler::getid() const  {return id; }
vector<BYTE> WebSocketRequestHandler::getdata() const {return data; }
int WebSocketRequestHandler::getdlc() const  {return dlc; }