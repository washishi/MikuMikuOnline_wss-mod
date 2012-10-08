//
// Lobby.hpp
//

#pragma once
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include "../common/network/Session.hpp"

class LobbySession : public network::Session {
	public:
		LobbySession(boost::asio::io_service& io_service,
			boost::asio::ip::tcp::resolver::iterator endpoint_iterator);

		void Start();
		void Connect(const boost::system::error_code& error);

	private:
        boost::asio::io_service& io_service_;
        boost::asio::ip::tcp::resolver::iterator endpoint_iterator_;
};

typedef boost::shared_ptr<LobbySession> LobbySessionPtr;

class Lobby {
	class Server {
		public:
			Server();
			void Start(const std::string& host);

		private:
			boost::asio::io_service io_service_;
			LobbySessionPtr session_;
			boost::thread thread_;

			bool loaded_;
			std::string host_, name_, note_, stage_;
			int capacity_, player_num_;

		public:
			bool loaded() const;
			std::string host() const;
			std::string name() const;
			std::string note() const;
			std::string stage() const;
			int player_num() const;
			int capacity() const;
	};

	typedef std::shared_ptr<Server> ServerPtr;

	public:
		Lobby();
		bool Reload(const std::string& lobby_server);
		const std::vector<ServerPtr>& servers() const;

	private:
		std::string GetServerFullInfo(const std::string& host);

	private:
		std::vector<ServerPtr> servers_;
};