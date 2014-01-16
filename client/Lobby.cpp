//
// Lobby.cpp
//

#include "Lobby.hpp"
#include <boost/algorithm/string/split.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "../common/network/Utils.hpp"
#include "../common/network/CommandHeader.hpp"
#include <boost/archive/text_iarchive.hpp>
#include <boost/property_tree/ptree_serialization.hpp>
#include <boost/serialization/string.hpp>

LobbySession::LobbySession(boost::asio::io_service& io_service,
	boost::asio::ip::tcp::resolver::iterator endpoint_iterator) :
		network::Session(io_service),
		io_service_(io_service),
		endpoint_iterator_(endpoint_iterator)
{
}

void LobbySession::Start()
{
	//using namespace boost::asio::ip;
 //   try {
	//	tcp::resolver resolver(io_service_);
 //       tcp::resolver::query query("m2op.net", "39390");
 //       endpoint_iterator_ = resolver.resolve(query);
 //   } catch (const std::exception& e) {
 //       Logger::Error(_T("%s"), unicode::ToTString(e.what()));
 //   }

	boost::asio::async_connect(socket_tcp_, endpoint_iterator_,
		boost::bind(&LobbySession::Connect, this,
                boost::asio::placeholders::error));

}

void LobbySession::Connect(const boost::system::error_code& error)
{
    if (!error) {
		Send(network::ServerRequestedFullServerInfo());
        boost::asio::async_read_until(socket_tcp_, receive_buf_, NETWORK_UTILS_DELIMITOR,
                boost::bind(&LobbySession::ReceiveTCP, shared_from_this(),
                        boost::asio::placeholders::error));
    } else {
		Logger::Error(_T("%s"), unicode::ToTString(error.message()));
	}
}

Lobby::Server::Server() :
	loaded_(false),
	capacity_(0),
	player_num_(0)
{


}

bool Lobby::Server::loaded() const
{
	return loaded_;
}

std::string Lobby::Server::host() const
{
	return host_;
}


std::string Lobby::Server::name() const
{
	return name_;
}

std::string Lobby::Server::note() const
{
	return note_;
}

std::string Lobby::Server::stage() const
{
	return stage_;
}

int Lobby::Server::player_num() const
{
	return player_num_;
}

int Lobby::Server::capacity() const
{
	return capacity_;
}

void Lobby::Server::Start(const std::string& host)
{
	host_ = host;

	boost::asio::ip::tcp::resolver::iterator endpoint_iterator;
    try {
		boost::asio::ip::tcp::resolver resolver(io_service_);
        boost::asio::ip::tcp::resolver::query query(host, "39390");
        endpoint_iterator = resolver.resolve(query);

		session_ = boost::make_shared<LobbySession>(io_service_, endpoint_iterator);
		session_->set_on_receive(std::make_shared<network::CallbackFunc>(
				[this](network::Command c) {

					switch (c.header()) {

						// クライアント情報要求
						case network::header::ClientReceiveFullServerInfo:
						{
							using namespace boost::property_tree;
							std::string buffer;
							network::Utils::Deserialize(c.body(), &buffer);
							std::stringstream stream(buffer);
							ptree pt;
							try {
								boost::archive::text_iarchive ia(stream);
								ia >> pt;
								loaded_ = true;
							} catch (std::exception& e) {
								Logger::Error(_T("%s"), unicode::ToTString(e.what()));
							}

							name_ = pt.get<std::string>("config.server_name","");
							note_ = pt.get<std::string>("config.server_note","");
							stage_ = pt.get<std::string>("config.stage","");
							capacity_ = pt.get<int>("config.capacity", 0);
							player_num_ = pt.get_child("players", ptree()).size();

							Logger::Info(_T("%s %s %d %d"), unicode::ToTString(name_)
								, unicode::ToTString(note_)
								, capacity_
								, player_num_);
						}
						break;
					}
		}));

    } catch (const std::exception& e) {
		Logger::Error(_T("%s"), unicode::ToTString(e.what()));
	}

	if (session_) {
		session_->Start();
		thread_ = boost::thread([this]() {
			try {
				io_service_.run();
			} catch (std::exception& e) {
				std::cout << e.what() << std::endl;
			}
		});
	}

}

Lobby::Lobby()
{

}

bool Lobby::Reload(const std::string& lobby_server)
{
	servers_.clear();

	using boost::asio::ip::tcp;
	boost::asio::io_service io_service;
	try {  // ※ 名前解決できない場合に例外が発生するのでトラップ
		tcp::resolver resolver(io_service);
		tcp::resolver::query query(tcp::v4(), lobby_server.c_str(), "39380");
		tcp::resolver::iterator iterator = resolver.resolve(query);

		boost::system::error_code error;
		tcp::socket s(io_service);
		boost::asio::connect(s, iterator, error);

		boost::asio::streambuf buf;
		size_t reply_length = boost::asio::read_until(s, buf, ';', error);

		if (error) {
			Logger::Error(_T("Cannot connect to lobby servers"));
			return false;
		} else {
			std::istream is(&buf);
			std::string line;
			std::getline(is, line);

			std::vector<std::string> result;
			boost::algorithm::split(result, line.substr(0, line.size() - 1), boost::is_any_of(","));


			BOOST_FOREACH(const auto& host, result) {
				//for (int i = 0; i < 6; i++) {
				auto ptr = std::make_shared<Server>();
				boost::thread([ptr, host]() {
					ptr->Start(host);
				});
				servers_.push_back(ptr);
				//}
			}
			return true;
		}
	}
	catch (const std::exception& e) {
		Logger::Error(_T("Cannot resolve Hostname:%s"),unicode::ToTString(lobby_server));
		return false;
	}

}

const std::vector<Lobby::ServerPtr>& Lobby::servers() const
{
	return servers_;
}