// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、または
// 参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。
//

#pragma once

#ifdef _WIN32
#include <tchar.h>
#endif

// C ランタイム ヘッダー ファイル
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <stdint.h>
#include <assert.h>
#include <cmath>
#include <iostream>

// TODO: プログラムに必要な追加ヘッダーをここで参照してください。

#include <vector>
#include <list>
#include <map>
#include <deque>
#include <queue>
#include <array>
#include <string>
#include <memory>
#include <iostream>
#include <fstream>
#include <algorithm>

#include <pssr.h>
#include <osrng.h>
#include <modes.h>
#include <aes.h>
#include <rsa.h>
#include <sha.h>

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/filesystem.hpp>
#include <boost/timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/thread.hpp>
#include <boost/foreach.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/property_tree/ptree_serialization.hpp>

#include "../common/Logger.hpp"