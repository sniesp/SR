#include <iostream>
#include <boost/asio.hpp>

#include "TcpAcceptor.hpp"
#include "TcpConnection.hpp"

int main(int argc, char *argv[])
{
	boost::asio::io_service io_service;
	TcpAcceptor acc(io_service, 5555);

	std::list<boost::shared_ptr<TcpConnection> > connectionList;

	while (1)
	{
		std::list<boost::shared_ptr<TcpConnection> > newConnections =
				acc.getConnections();
		for (std::list<boost::shared_ptr<TcpConnection> >::iterator ii =
				newConnections.begin(); ii != newConnections.end(); ii++)
		{
			connectionList.push_back(*ii);
		}

		for (std::list<boost::shared_ptr<TcpConnection> >::iterator ii =
				connectionList.begin(); ii != connectionList.end(); ii++)
		{
			boost::shared_ptr<Json::Value> pJsonData;
			try
			{
				while ((*ii)->popJsonData(pJsonData))
				{
					(*ii)->pushJsonData(pJsonData);
				}
			}
			catch (std::runtime_error & e)
			{
				connectionList.erase(ii);
				break;
			}
		}
	}

	return 0;
}

