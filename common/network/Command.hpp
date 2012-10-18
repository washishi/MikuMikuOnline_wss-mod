//
// Command.hpp
//

#pragma once

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/asio.hpp>
#include <stdint.h>
#include "CommandHeader.hpp"
#include "Utils.hpp"
#include "../database/AccountProperty.hpp"

namespace network {

class Session;
typedef boost::weak_ptr<Session> SessionWeakPtr;

    class Command {
        public:
            Command(header::CommandHeader header,
				const std::string body) :
                header_(header), body_(body), plain_(false) {}

            Command(header::CommandHeader header,
				const std::string body,
				const SessionWeakPtr& session) :
                header_(header), body_(body), session_(session), plain_(false) {}

            Command(header::CommandHeader header,
				const std::string body,
				const boost::asio::ip::udp::endpoint& udp_endpoint) :
                header_(header), body_(body), udp_endpoint_(udp_endpoint), plain_(false) {}

            header::CommandHeader header() const;
            const std::string& body() const;
            SessionWeakPtr session();
			boost::asio::ip::udp::endpoint udp_endpoint() const;
			bool plain() const;

        private:
            header::CommandHeader header_;

        protected:
            std::string body_;
            SessionWeakPtr session_;
			boost::asio::ip::udp::endpoint udp_endpoint_;
			bool plain_;
    };

	template<header::CommandHeader Header>
	class CommandTemplate0 : public Command {
		public:
			CommandTemplate0() :
			  Command(Header, "") {}
	};

	template<header::CommandHeader Header, class T1>
	class CommandTemplate1 : public Command {
		public:
			CommandTemplate1(T1 t1) :
			  Command(Header, Utils::Serialize(t1)) {}
	};

	template<header::CommandHeader Header, class T1, class T2>
	class CommandTemplate2 : public Command {
		public:
			CommandTemplate2(T1 t1, T2 t2) :
			  Command(Header, Utils::Serialize(t1, t2)) {}
	};

	template<header::CommandHeader Header, class T1, class T2, class T3>
	class CommandTemplate3 : public Command {
		public:
			CommandTemplate3(T1 t1, T2 t2, T3 t3) :
			  Command(Header, Utils::Serialize(t1, t2, t3)) {}
	};

	template<header::CommandHeader Header, class T1, class T2, class T3, class T4>
	class CommandTemplate4 : public Command {
		public:
			CommandTemplate4(T1 t1, T2 t2, T3 t3, T4 t4) :
			  Command(Header, Utils::Serialize(t1, t2, t3, t4)) {}
	};

	template<header::CommandHeader Header, class T1, class T2, class T3, class T4, class T5>
	class CommandTemplate5 : public Command {
		public:
			CommandTemplate5(T1 t1, T2 t2, T3 t3, T4 t4, T5 t5) :
			  Command(Header, Utils::Serialize(t1, t2, t3, t4, t5)) {}
	};

	template<header::CommandHeader Header, class T1, class T2, class T3, class T4, class T5, class T6>
	class CommandTemplate6 : public Command {
		public:
			CommandTemplate6(T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6) :
			  Command(Header, Utils::Serialize(t1, t2, t3, t4, t5, t6)) {}
	};
	
	typedef CommandTemplate0<header::FatalConnectionError>					FatalConnectionError;
	typedef CommandTemplate0<header::ServerStartEncryptedSession>			ServerStartEncryptedSession;
	typedef CommandTemplate0<header::ClientStartEncryptedSession>			ClientStartEncryptedSession;
	typedef CommandTemplate0<header::ClientRequestedPublicKey>				ClientRequestedPublicKey;
	typedef CommandTemplate0<header::ClientRequestedClientInfo>				ClientRequestedClientInfo;
	typedef CommandTemplate0<header::ClientReceiveServerCrowdedError>		ClientReceiveServerCrowdedError;
	typedef CommandTemplate0<header::ServerRequestedFullServerInfo>			ServerRequestedFullServerInfo;
	typedef CommandTemplate0<header::ServerRequestedPlainFullServerInfo>	ServerRequestedPlainFullServerInfo;

	typedef CommandTemplate1<header::ServerReceivePublicKey,
		const std::string&>	ServerReceivePublicKey;

	typedef CommandTemplate1<header::ClientReceiveAccountRevisionPatch,
		const std::string&>	ClientReceiveAccountRevisionPatch;

	typedef CommandTemplate6<header::ClientUpdatePlayerPosition,
		uint32_t, int16_t, int16_t, int16_t, uint8_t, uint8_t> ClientUpdatePlayerPosition;

	typedef CommandTemplate5<header::ServerUpdatePlayerPosition,
		int16_t, int16_t, int16_t, uint8_t, uint8_t> ServerUpdatePlayerPosition;

	typedef CommandTemplate2<header::ServerRequestedAccountRevisionPatch,
		uint32_t, int> ServerRequestedAccountRevisionPatch;

	typedef CommandTemplate2<header::ClientReceiveAccountRevisionUpdateNotify,
		uint32_t, int> ClientReceiveAccountRevisionUpdateNotify;

	typedef CommandTemplate3<header::ClientReceiveCommonKey,
		const std::string&, const std::string&, uint32_t> ClientReceiveCommonKey;
	
	typedef CommandTemplate2<header::ServerUpdateAccountProperty,
		AccountProperty, const std::string&> ServerUpdateAccountProperty;

	typedef CommandTemplate3<header::ServerReceiveClientInfo,
		const std::string&, uint16_t, uint16_t> ServerReceiveClientInfo;

	typedef CommandTemplate1<header::ClientReceiveWriteAverageLimitUpdate,
		uint16_t> ClientReceiveWriteAverageLimitUpdate;

	typedef CommandTemplate1<header::ClientReceiveUnsupportVersionError,
		uint32_t> ClientReceiveUnsupportVersionError;

	typedef CommandTemplate1<header::ServerReceiveAccountInitializeData,
		const std::string&> ServerReceiveAccountInitializeData;

	typedef CommandTemplate1<header::ServerReceiveJSON,
		const std::string&> ServerReceiveJSON;

	typedef CommandTemplate2<header::ClientReceiveJSON,
		const std::string&, const std::string&> ClientReceiveJSON;

	typedef CommandTemplate1<header::ClientReceiveServerInfo,
		const std::string&> ClientReceiveServerInfo;

	typedef CommandTemplate1<header::ClientReceiveFullServerInfo,
		const std::string&> ClientReceiveFullServerInfo;

	typedef CommandTemplate1<header::UserFatalConnectionError,
		uint32_t> UserFatalConnectionError;

}
