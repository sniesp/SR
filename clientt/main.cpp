#include <iostream>
#include <boost/asio.hpp>

#include "TcpConnection.hpp"

using namespace boost::asio;

void printerFunction(TcpConnection & connection)
{
	while (connection.connected())
	{
		boost::shared_ptr<Json::Value> data(new Json::Value());
		try
		{
			if (connection.popJsonData(data)) {
				std::cout << (*data).get("message", "NO_DATA").asString()
						<< std::endl;
				//FIXME
				Json::StyledWriter writer;
				std::cout << writer.write(*data) << std::endl;
			}
		}
		catch (std::runtime_error & e)
		{
			break;
		}
		boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
	}
}

int main(int argc, char *argv[])
{
	std::cout << "Chat Client" << std::endl;

	int nodeAddress = 1234567890;
	if (argc > 1)
	{
		nodeAddress = atoi(argv[1]);
	}

	io_service io_service;

	ip::tcp::endpoint endpoint;
	if (argc < 3)
	{
		ip::tcp::resolver resolver(io_service);
		ip::tcp::resolver::query query("127.0.0.1", "5555");
		endpoint = *(resolver.resolve(query));
	}
	else if (argc == 3)
	{
		ip::tcp::resolver resolver(io_service);
		ip::tcp::resolver::query query(argv[2], "5555");
		endpoint = *(resolver.resolve(query));
	}
	else if (argc > 3)
	{
		ip::tcp::resolver resolver(io_service);
		ip::tcp::resolver::query query(argv[2], argv[3]);
		endpoint = *(resolver.resolve(query));
	}

	boost::shared_ptr<ip::tcp::socket> pSocket(new ip::tcp::socket(io_service));

	try
	{
		pSocket->connect(endpoint);
	}
	catch (boost::system::system_error & e)
	{
		std::cout << "Could not connect to gateway." << std::endl;
		exit(0);
	}

	TcpConnection conn(pSocket);

	// initiate connection
	boost::shared_ptr<Json::Value> initialMessage(new Json::Value());
	(*initialMessage)["peerType"] = "client";
	(*initialMessage)["address"] = nodeAddress; // this node address
	conn.pushJsonData(initialMessage);

	std::cout << "Input format: <destination> <message>" << std::endl;

	boost::thread(boost::bind(printerFunction, boost::ref(conn)));

	while (conn.connected())
	{
		boost::shared_ptr<Json::Value> data(new Json::Value());
		int destinationAddress;
		std::string message;

		std::cin >> destinationAddress;
		getline(std::cin, message);

		if (destinationAddress != 0)
		{
			(*data)["destination"] = destinationAddress;
			(*data)["message"] = message;
			try
			{
				conn.pushJsonData(data);
			}
			catch (std::runtime_error & e)
			{
				break;
			}
		}
		else
		{
			std::cout << "Enter proper destination." << std::endl;
		}
	}

	std::cout << "Connection lost." << std::endl;

	return 0;
}

