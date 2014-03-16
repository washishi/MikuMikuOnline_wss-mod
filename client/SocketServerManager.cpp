//
// SocketServerManager.cpp
//

#include "SocketServerManager.hpp"
#include "CardManager.hpp"

const char SocketServerManager::DELIMITOR[] = {0x0d, 0x0a, 0x0};

SocketServerManager::SocketServerManager(const ManagerAccessorPtr& manager_accessor) :
	manager_accessor_(manager_accessor),
	acceptor_(io_service_, tcp::endpoint(tcp::v4(), 39340))
{

}

void SocketServerManager::Start()
{
    auto new_session = boost::make_shared<Session>(manager_accessor_, io_service_);
    acceptor_.async_accept(new_session->socket(),
        boost::bind(&SocketServerManager::ReceiveSession, this, new_session,
          boost::asio::placeholders::error));

	boost::thread t([this](){
        try {
            io_service_.run();
        } catch (std::exception& e) {
            std::cout << e.what() << std::endl;
        }
	});
}

void SocketServerManager::ReceiveSession(const SessionPtr& session, const boost::system::error_code& error)
{
		
	if(!error) {
        session->Start();
    }
    auto new_session = boost::make_shared<Session>(manager_accessor_, io_service_);
    acceptor_.async_accept(new_session->socket(),
        boost::bind(&SocketServerManager::ReceiveSession, this, new_session,
          boost::asio::placeholders::error));

}

SocketServerManager::Session::Session(const ManagerAccessorPtr& manager_accessor, boost::asio::io_service& io_service) :

	socket_(io_service),
	card_(std::make_shared<Card>(manager_accessor, "", "sock", "", "",
                            std::vector<std::string>()))
{
//	auto callback = std::make_shared<std::function<void(const std::string&)>>(
	auto callback = std::make_shared<std::function<bool(const std::string&)>>(	// ※ エラー発生を認識できる様に修正
//		[this](const std::string& str){
		[this](const std::string& str)-> bool{									// ※ エラー発生を認識できる様に修正
// ※ ソケットへの書き込み時に切断されていた場合例外が発生するので修正
			boost::system::error_code err;
//			boost::asio::write(socket_, boost::asio::buffer(str.data(), str.size()));
			boost::asio::write(socket_, boost::asio::buffer(str.data(), str.size()),err);
			if (err == 0) {
				return true; 
			} else {
				return false;
			}
// ※ここまで  
		});
		card_->set_on_socket_reply(callback);
		if (auto card_manager = manager_accessor->card_manager().lock()) {
		    card_manager->AddCard(card_);
		}
}

void SocketServerManager::Session::Start()
{
    boost::asio::async_read_until(socket_,
        receive_buf_, DELIMITOR[1],
            boost::bind(&SocketServerManager::Session::ReceiveTCP, shared_from_this(),
              boost::asio::placeholders::error));
}

void SocketServerManager::Session::ReceiveTCP(const boost::system::error_code& error)
{
	boost::system::error_code err;
    if (!error) {
        std::string buffer(boost::asio::buffer_cast<const char*>(receive_buf_.data()),receive_buf_.size());
        auto length = buffer.find_last_of(DELIMITOR);

        if (length != std::string::npos) {

            receive_buf_.consume(length + 1);
            buffer.erase(length - 1);

            if (!buffer.empty()) {
				Logger::Debug(_T("Receive command: %d"), unicode::ToTString(buffer));
				card_->Execute(buffer, "", 
					[this](const Handle<Value>& value, const std::string error){
						if (!error.empty()) {
							std::string return_str(error);
							return_str += DELIMITOR;
// ※ ソケットへの書き込み時に切断されていた場合に例外が発生しない様に修正
							boost::system::error_code err;
//							boost::asio::write(socket_, boost::asio::buffer(return_str.data(), return_str.size()));
							boost::asio::write(socket_, boost::asio::buffer(return_str.data(), return_str.size()),err);
// ※ ここまで
						}
					});
            }

			boost::asio::async_read_until(socket_,
				receive_buf_, DELIMITOR[1],
					boost::bind(&SocketServerManager::Session::ReceiveTCP, shared_from_this(),
					  boost::asio::placeholders::error));

        }

    } else {

    }
}

tcp::socket& SocketServerManager::Session::socket()
{
	return socket_;
}