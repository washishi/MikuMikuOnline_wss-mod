//
// SocketServerManager.hpp
//

#pragma once

#include "ManagerAccessor.hpp"
#include <boost/asio.hpp>
#include "Card.hpp"

using namespace boost::asio::ip;

class SocketServerManager {
	public:
		class Session : public boost::enable_shared_from_this<Session> {
			public:
				Session(const ManagerAccessorPtr& manager_accessor, boost::asio::io_service& io_service);
				void Start();
				tcp::socket& socket();

			private:
				void ReceiveTCP(const boost::system::error_code& error);

			private:
				CardPtr card_;
				tcp::socket socket_;
				boost::asio::streambuf receive_buf_;
		};
		typedef boost::shared_ptr<Session> SessionPtr;

	public:
		SocketServerManager(const ManagerAccessorPtr& manager_accessor = ManagerAccessorPtr());
		void Start();
		void ReceiveSession(const SessionPtr& session, const boost::system::error_code& error);

	private:
		ManagerAccessorPtr manager_accessor_;

		boost::asio::io_service io_service_;
		tcp::acceptor acceptor_;

		static const char DELIMITOR;
};
typedef std::shared_ptr<SocketServerManager> SocketServerManagerPtr;
typedef std::weak_ptr<SocketServerManager> SocketServerManagerWeakPtr;