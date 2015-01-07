/*
 * RpcServer.cpp
 *
 *  Created on: Jan 6, 2015
 *      Author: slawek
 */

#include "RpcServer.hpp"

void RpcServer::run()
{
	connectionList conns = acceptor.getConnections();
	if (!conns.empty())
	{
		for(connectionList::iterator ii = conns.begin(); ii != conns.end(); ii++)
		{
			newConnections.push_back(*ii);
		}
	}

	for(connectionList::iterator ii = newConnections.begin(); ii != newConnections.end(); ii++)
	{
		pValue pJsonValue;
		try
		{
			if((*ii)->popJsonData(pJsonValue))
			{
				std::string method = pJsonValue->get("method", "").asString();
				if (method == "nodeIdentify" && pJsonValue->isMember("params"))
				{

					int nodeId = (*pJsonValue)["params"].get("nodeId", 0).asInt();
					if (nodeId != 0)
					{
						localConnections[nodeId] = *ii;
					}

					//TODO local NODE list has changed - broadcast it to other gateways

				}
				else if (method == "gatewayIdentify" && pJsonValue->isMember("params"))
				{
					// gateway identification
					int gatewayId = (*pJsonValue)["params"].get("gatewayId", 0).asInt();
					bool response = pJsonValue->get("response", false).asBool();
					if (gatewayId != 0)
					{
						globalConnections[gatewayId] = *ii;

						if (response == false)
						{
							// gateway identification response
							pValue pJsonResponse(new Json::Value());
							(*pJsonResponse)["method"] = method;
							(*pJsonResponse)["response"] = true;
							(*pJsonResponse)["params"]["gatewayId"] = gatewayId;
							(*ii)->pushJsonData(pJsonResponse);
						}

						//TODO GW interconnection has changed !!

					}
				}
				newConnections.erase(ii); // remove connection from new connection list
				break;
			}
		}
		catch (std::runtime_error & e) // connection lost
		{
			newConnections.erase(ii); // remove connection from connection list
			break;
		}
	}

	for(connectionMap::iterator ii = localConnections.begin(); ii != localConnections.end(); ii++)
	{
		try
		{
			pValue pJsonValue;
			if(ii->second->popJsonData(pJsonValue))
			{
				std::string method = pJsonValue->get("method", "").asString();
				if (method == "forwardMessage" && pJsonValue->isMember("params"))
				{
					Json::Value stub;
					forwardMessage((*pJsonValue)["params"], stub);
				}
			}
		}
		catch (std::runtime_error & e) // connection lost
		{
			localConnections.erase(ii); // remove connection from connection list

			//TODO local node list has changed - broadcast !

			break;
		}
	}

	for(connectionMap::iterator ii = globalConnections.begin(); ii != globalConnections.end(); ii++)
	{
		pValue pJsonValue;
		try
		{
			if(ii->second->popJsonData(pJsonValue))
			{
				std::string method = pJsonValue->get("method", "").asString();
				if (pJsonValue->isMember("params"))
				{
					for (std::map<std::string, remoteFunction>::iterator jj = remoteFunctions.begin(); jj != remoteFunctions.end(); jj++)
					{
						if (method == jj->first)
						{
							bool response = pJsonValue->get("response", false).asBool();
							Json::Value result;
							if (!response && (this->*(jj->second))((*pJsonValue)["params"], result))
							{
								pValue pResult(new Json::Value());
								(*pResult)["method"] = method;
								(*pResult)["response"] = true;
								(*pResult)["params"] = result;
								ii->second->pushJsonData(pResult);
							}
						}
					}
				}
			}
		}
		catch (std::runtime_error & e) // connection lost
		{
			localConnections.erase(ii); // remove connection from connection list

			//TODO local node list has changed - broadcast !

			break;
		}
	}
}

bool RpcServer::forwardMessage(Json::Value & params, Json::Value & result)
{
	//TODO check if it is not addressed to THIS gateway
	//TODO transmit this message to proper gateway (according to MST algorithm)
	return false;
}
