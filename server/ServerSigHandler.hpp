//
// 割り込みハンドラーを用いてgracefull delete
//

#pragma once
#include <csignal>
#include <boost/thread.hpp>
//#include "../common/network/Session.hpp"
#include "Server.hpp"

namespace network {

class ServerSigHandler {
public:
  ServerSigHandler(int sig,Server *_server) : sig_to_wait(sig){
    sigset_t ss;
    sigemptyset( &ss );
    sigaddset( &ss, sig_to_wait );
    // マスクをセットして現在のマスクのバックアップも行う
    sigprocmask( SIG_BLOCK, &ss, &backup );
    // シグナルを待機して、補足、メンバー関数の実行をするスレッドを実行
    signal_thread = new boost::thread( boost::bind(&ServerSigHandler::wait_and_exec, this) );
    signal_thread->detach();
    server=_server;
  }
  
  ~ServerSigHandler() {
    sigprocmask( SIG_SETMASK, &backup, NULL );
    //signal_thread->join(); いらない
    delete signal_thread;
  }

private:
  // トラップするシグナル
  int sig_to_wait;
  // マスクのバックアップ
  sigset_t backup;
  boost::thread *signal_thread;
  Server *server;
  bool wait_and_exec() {
    sigset_t ss;
    sigemptyset(&ss);
    int ret = sigaddset(&ss, sig_to_wait);
    if (ret != 0) 
      return false;
    // シグナルをブロック
    ret = pthread_sigmask(SIG_BLOCK, &ss, NULL);
    if (ret != 0) 
      return false;
    while(1) {
      int signo;
      if (sigwait(&ss, &signo) == 0) {
	//Serverをstopする.
	(server->Stop)( signo );
	break;
      }
    }
    return true;
  }
};

}
