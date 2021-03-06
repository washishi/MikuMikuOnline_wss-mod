//
// ServerLauncher.cpp
//

#include "ServerLauncher.hpp"
#include "../common/Logger.hpp"
#include <boost/interprocess/mapped_region.hpp>

using namespace boost::interprocess;
using boost::asio::ip::tcp;  // ※追加
ServerLauncher::ServerLauncher() :
shm_(create_only, "MMO_SERVER_WITH_CLIENT", read_write, 32)
{
	// ※サーバ起動済み確認　ここから
	boost::asio::io_service io_service;
	tcp::socket socket(io_service);

	boost::system::error_code error;
    socket.connect(tcp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 39390), error);

	if (!error) {
		// 接続できる場合はサーバ起動済みなので切断して終了
		socket.close();
    } else {
	// ※サーバ起動済み確認　ここまで
		// サーバーを起動
		STARTUPINFO si;
		ZeroMemory(&si,sizeof(si));
		si.cb=sizeof(si);

		// カレントディレクトリを取得
		TCHAR crDir[MAX_PATH + 1];
		GetCurrentDirectory(MAX_PATH + 1 , crDir);
		_tcscat(crDir, _T("\\server"));

		Logger::Info(_T("Starting Server..."));

		mapped_region region(shm_, read_write);
		CreateProcess(_T("./server/Server.exe"), nullptr, nullptr, nullptr, FALSE,
			NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW, nullptr, crDir, &si, &pi_);
	} // ※追加
}

ServerLauncher::~ServerLauncher()
{

}
