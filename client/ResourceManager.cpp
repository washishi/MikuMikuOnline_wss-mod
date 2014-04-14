//
// ResourceManager.cpp
//

#include <fstream>
#include <iostream> // ※ ファイルの非同期読み込みを行うために追加
#include <boost/filesystem.hpp>
#include <boost/crc.hpp>
#include "ResourceManager.hpp"
#include "../common/Logger.hpp"
#include "../common/unicode.hpp"
#include "Music.hpp"

// ※ DXライブラリの仕様変更によりコメント(DXlib.hに定義が含まれている、またファイル名のデータ型が異なる)
// MV1書き出し関数
//namespace DxLib
//{
// extern int MV1SaveModelToMV1File( int MHandle, const char *FileName,
//	int SaveType = ((0x0001) | (0x0002)), int AnimMHandle = -1,
//	int AnimNameCheck = TRUE, int Normal8BitFlag = 1,
//	int Position16BitFlag = 1, int Weight8BitFlag = 0,
//	int Anim16BitFlag = 1 );
//};

ResourceManager::MemoryPool ResourceManager::mempool;

const static TCHAR* CHAT_FONT_NAME = _T("UmePlus P Gothic");
static int CHAT_FONT_SIZE = 15;
static int CHAT_FONT_THICK = 1;
static int CHAT_FONT_TYPE = DX_FONTTYPE_ANTIALIASING;
const static TCHAR* UNKNOWN_MODEL_NAME = _T("char:アンノウン:おぼろ(へうげもん)式");

const static char* MOTIONS_PATH = "./motions";
const static char* TEXTURES_PATH = "./system/textures";
const static char* DEFAULT_INFO_JSON = "./system/default.info.json";

tlsf_pool& ResourceManager::memory_pool()
{
	return mempool.pool();
}

int ResourceManager::default_font_handle_ = -1;
int ResourceManager::default_font_handle()
{
	if (default_font_handle_ < 0) {
		//TCHAR font_name[] = CHAT_FONT_NAME;
		const TCHAR* font_name = CHAT_FONT_NAME;
		default_font_handle_ = CreateFontToHandle(font_name, CHAT_FONT_SIZE, CHAT_FONT_THICK, CHAT_FONT_TYPE);
	}

	return default_font_handle_;

}

int ResourceManager::default_font_size()
{
	return CHAT_FONT_SIZE;
}

std::unordered_map<tstring, ImageHandlePtr> ResourceManager::graph_handles_;
std::unordered_map<tstring, std::vector<ImageHandlePtr>> ResourceManager::div_graph_handles_;
ImageHandlePtr ResourceManager::LoadCachedGraph(const tstring& filename)
{
	ImageHandlePtr handle;
	if(graph_handles_.find(filename) == graph_handles_.end()) {
		handle = std::make_shared<ImageHandle>(DxLib::LoadGraph(filename.c_str()));
		graph_handles_[filename] = handle;
	} else {
		handle = graph_handles_[filename];
	}
	return ImageHandlePtr(handle);
}

void ResourceManager::ClearCache()
{
	graph_handles_.clear();
	div_graph_handles_.clear();

	model_names_.clear();
	model_handles_.clear();
	model_name_tree_.clear();

	InitGraph();
	MV1InitModel();
}

namespace {
	void MergePtree(ptree* dst,
		const ptree& source,
		const ptree::path_type& current_path = ptree::path_type(""))
	{
		BOOST_FOREACH(const auto& tree, source) {
			if (!tree.first.empty()) {
				auto new_path = current_path;
				new_path /= tree.first;
				if (tree.second.empty() || tree.second.front().first.empty()) {
					if (!dst->get_child_optional(new_path)) {
						dst->put_child(new_path, tree.second);
					}
				} else {
					MergePtree(dst, tree.second, new_path);
				}
			}
		}
	}
};

const ptree& ResourceManager::GetDefaultInfoJSON()
{
	static ptree pt_default_json;
	if (pt_default_json.empty()) {
		read_json(DEFAULT_INFO_JSON, pt_default_json);
	}
	return pt_default_json;
}

std::unordered_map<tstring, tstring> ResourceManager::model_names_;
std::unordered_map<tstring, ModelHandle> ResourceManager::model_handles_;
std::unordered_map<tstring, SharedModelDataPtr> ResourceManager::shared_model_data_;

std::vector<std::string> ResourceManager::model_name_list_;
ptree ResourceManager::model_name_tree_;
void ResourceManager::BuildModelFileTree()
{
	using namespace boost::filesystem;
	using namespace std;

	model_name_tree_.clear();
	model_name_list_.clear();

	std::function<void(const path& p)> search_dir;
	search_dir = [&search_dir](const path& p) {

		using namespace boost::filesystem;

		if (exists(p) && is_directory(p)) {
			for (auto it_dir = directory_iterator(p); it_dir != directory_iterator(); ++it_dir) {
				if (is_directory(*it_dir)) {
					path json_path = it_dir->path() / "info.json";
					if (exists(json_path)) {

						path model_path;
						for (auto it = directory_iterator(*it_dir); it != directory_iterator(); ++it) {
							auto extension = it->path().extension().string();

							if (extension == ".mv1" || extension == ".x"
								|| extension == ".pmd" || extension == ".pmx") {
									model_path = it->path();
									break;
							}
						}

						if (!model_path.empty()) {
							ptree pt_json;
							read_json(json_path.string(), pt_json);
							MergePtree(&pt_json, GetDefaultInfoJSON());

							std::string name = pt_json.get<std::string>("name", "");
							auto model_path_str = unicode::sjis2utf8(model_path.string());

							pt_json.put<std::string>("modelpath", model_path_str);
							if (!name.empty()) {
								model_name_list_.push_back(name);
								model_name_tree_.put_child(ptree::path_type(name + ":_info_", ':'), pt_json);
								// ※ /reload で即指定されたモデルが表示されるように修正 ここから
								auto name1 = unicode::ToTString(name);
								auto name2 = model_names_.find(unicode::ToTString(name));
								if (name2 != model_names_.end() && name1 != name2->second ) {
									// 代替モデルになっていた場合に対応を正規のモデルに戻すために一度テーブルから消去
									model_names_.erase(name1);
								}
								// ※ ここまで
								// ステージデータをキャッシュ
								// if (name.find("stage:") == 0) {
								// ※ スカイドームもキャッシュ対象に追加
								if (name.find("stage:") == 0 || name.find("skydome:") == 0 ) {
									CreateModelCache(model_path_str, pt_json);
								}
							}
						}
					} else {
						search_dir(it_dir->path());
					}
				}
			}
		}
	};

	try {
		search_dir("./models");
	} catch (const filesystem_error& ex) {
		Logger::Error(_T("%s"), unicode::ToTString(ex.what()));
	}
}

int LoadFile(const TCHAR *FilePath, void **FileImageAddr, int *FileSize)
{
	Logger::Debug(_T("Load %s"), FilePath);
	tstring path(FilePath);

	std::ifstream ifs(path.c_str(), std::ios::binary);

	if (!ifs) {
		*FileImageAddr = nullptr;
		return -1;
	}

	ifs.seekg (0, std::ios::end);
	*FileSize = static_cast<int>(ifs.tellg());
	ifs.seekg (0, std::ios::beg);

	auto buffer = reinterpret_cast<char*>(tlsf_new(ResourceManager::memory_pool(), *FileSize));
	ifs.read(buffer, *FileSize);
	*FileImageAddr = buffer;
	
    return 0;
}

int LoadFile(const TCHAR *FilePath, std::shared_ptr<char>* FileImage, int *FileSize)
{
	Logger::Debug(_T("Load %s"), FilePath);
	tstring path(FilePath);
	std::ifstream ifs(path.c_str(), std::ios::binary);

	if (!ifs) {
		return -1;
	}

	ifs.seekg (0, std::ios::end);
	*FileSize = static_cast<int>(ifs.tellg());
	ifs.seekg (0, std::ios::beg);

	auto buffer = reinterpret_cast<char*>(tlsf_new(ResourceManager::memory_pool(), *FileSize));
	ifs.read(buffer, *FileSize);

	*FileImage = std::shared_ptr<char>(buffer, [](char* ptr){
		tlsf_delete(ResourceManager::memory_pool(), reinterpret_cast<void*>(ptr));
	});

	return 0;
}

int FileReadFunc(const TCHAR *FilePath, void **FileImageAddr, int *FileSize, void *FileReadFuncData)
{
	ReadFuncData& funcdata = *static_cast<ReadFuncData*>(FileReadFuncData);

	using namespace boost::filesystem;
	wpath filepath(FilePath);

	bool load_motion = false;
	if (funcdata.motions_it != funcdata.motions.end() &&
	//	filepath.string().find_last_of("L.vmd") != std::string::npos) { // ※ 条件がおかしいので修正
	filepath.string().rfind("L.vmd") != std::string::npos) {

		filepath = funcdata.motions_it->second;
		load_motion = true;
	}

	Logger::Debug(_T("Request %s"), unicode::ToTString(filepath.wstring()));

	wpath full_path = funcdata.model_dir / filepath;
	if (!exists(full_path)) {
		if (load_motion) {
			full_path = MOTIONS_PATH / filepath;
		} else {
			full_path = TEXTURES_PATH / filepath;
		}
	}

	int result = LoadFile(full_path.wstring().c_str(), FileImageAddr, FileSize);

	if (load_motion) {
		// 読み込み失敗したモーションを削除
		if (result == -1) {
			funcdata.motions_it->second = "";
		}
		++funcdata.motions_it;
	}

	return result;
}

int FileReleaseFunc(void *MemoryAddr, void *FileReadFuncData)
{
	tlsf_delete(ResourceManager::memory_pool(), MemoryAddr);
	return 0;
}

void SetMotionNames(int handle, const ReadFuncData& funcdata)
{
	// モーションの名前を設定
	int motion_index = 0;
	for (auto it = funcdata.motions.begin(); it != funcdata.motions.end(); ++it) {
		if (!it->second.empty()) {
			MV1SetAnimName(handle, motion_index, unicode::ToTString(it->first).c_str());
			Logger::Debug(_T("Motion  %d"), unicode::ToTString(it->first));
			motion_index++;
		}
	}
}

std::string ResourceManager::GetCacheFilename(const ptree& info, const std::shared_ptr<char>& fileimage, int filesize)
{
    boost::crc_32_type crc;
	crc.process_block(static_cast<void*>(fileimage.get()),
		static_cast<void*>(fileimage.get() + filesize));

	boost::crc_32_type crc_info;
	std::stringstream stream;
	boost::archive::text_oarchive oa(stream);
	oa << info;
	while(stream.good()) {
		crc_info.process_byte(stream.get());
	}

	auto cache_filename = "./cache/" + 
		boost::lexical_cast<std::string>(crc.checksum()) + "_" +
		boost::lexical_cast<std::string>(crc_info.checksum()) +
		".mv1";

	return cache_filename;
}

void ResourceManager::CreateModelCache(std::string filepath, const ptree& info)
{
	auto funcdata = std::make_shared<ReadFuncData>(info);

	std::shared_ptr<char> fileimage;
	int filesize;
// ※ ここから  キャッシュファイルのされたファイル名の生成元を
//              モデルファイルの内容 から モデルファイル名(パス付き)と更新日に変更

//
//	LoadFile(unicode::ToTString(filepath).c_str(), &fileimage, &filesize );
//	auto cache_filename = GetCacheFilename(info, fileimage, filesize);
//	if (!boost::filesystem::exists(cache_filename)) {
	auto time = boost::filesystem::last_write_time(unicode::ToTString(filepath));
	auto keystring = filepath + to_simple_string(boost::posix_time::from_time_t(time));
	filesize = strlen(keystring.c_str()) + 1;
    char *nstring = new char[filesize];
    strcpy_s(nstring, filesize, keystring.c_str());
	fileimage = (std::shared_ptr<char>)nstring;
    auto cache_filename = GetCacheFilename(info, fileimage, filesize);
	if (!boost::filesystem::exists(cache_filename)) {
		fileimage.reset();
		LoadFile(unicode::ToTString(filepath).c_str(), &fileimage, &filesize);
// ※ ここまで
		int handle = MV1LoadModelFromMem(fileimage.get(), filesize, FileReadFunc, FileReleaseFunc, &(*funcdata));

		if (!boost::filesystem::exists("./cache")) {
			boost::filesystem::create_directory("./cache");
		}
//		MV1SaveModelToMV1File(handle, cache_filename.c_str()) ; // ※ DXライブラリの仕様変更により修正
//		MV1SaveModelToMV1File(handle, unicode::ToTString(cache_filename).c_str()) ; // ※ 座標精度を変更(MODクライアント互換)
		MV1SaveModelToMV1File(handle, unicode::ToTString(cache_filename).c_str(),MV1_SAVETYPE_NORMAL,-1,1,0,0,0,0);
		MV1DeleteModel(handle);
		boost::filesystem::last_write_time(unicode::ToTString(cache_filename),time);
	}
}

void ResourceManager::RequestModelFromName(const tstring& name)
{
	if (!IsCachedModelName(name)) {

	}
}

bool ResourceManager::IsCachedModelName(const tstring& name)
{
	auto name_it = model_names_.find(name);
	if (name_it != model_names_.end()) {
		// ※ model_handles_ を調べないように変更
		// return model_handles_.find(unicode::ToTString(name_it->second)) != model_handles_.end();
		return true;
	} else {
		return false;
	}
}

ReadFuncData::ReadFuncData(const ptree& info)
{
	tstring filepath = unicode::ToTString(info.get<std::string>("modelpath", ""));
	auto path = boost::filesystem::wpath(unicode::ToWString(filepath));
	model_dir = path.parent_path();

	if (path.extension() == _T(".pmd") || path.extension() == _T(".pmx")) {
		auto motions_array = info.get_child("character.motions", ptree());
		BOOST_FOREACH(auto& it,motions_array){
			auto dat = motions_array.get_child(it.first,ptree());
			if(!dat.empty()){
				auto type = dat.get<std::string>("type","");
				typedef std::unordered_map<std::string,std::function<void ()>> SwitchFunction;
				SwitchFunction switch_func;
				auto IntToString = [](int num)->std::string{std::stringstream ss;ss << num;return ss.str();};
				switch_func.insert(SwitchFunction::value_type("el",[&]()->void{
					int cnt = 0;
					std::string str;
					std::string motion;
					set_motions.insert(std::unordered_map<std::string, std::string>::value_type(it.first,"el"));
					bool flag = true;
					while(flag){
						str = IntToString(cnt);
						motion = dat.get<std::string>(str,"");
						if(motion.empty())flag = false;
						motions.push_back(std::pair<std::string,std::string>(it.first+"@"+str,motion));
						++cnt;
					}
				}));
				switch_func.insert(SwitchFunction::value_type("ch",[&]()->void{
					int cnt = 0;
					std::string str;
					std::string motion;
					set_motions.insert(std::unordered_map<std::string, std::string>::value_type(it.first,"ch"));
					bool flag = true;
					while(flag){
						str = IntToString(cnt);
						motion = dat.get<std::string>(str,"");
						if(motion.empty())flag = false;
						motions.push_back(std::pair<std::string,std::string>(it.first+"@"+str,motion));
						++cnt;
					}
				}));
				auto it = switch_func.find(type);
				if( it != switch_func.end() ){
					it->second();
				}else{
					throw(_T("無効なモーション指定型です。"));
				}
			}else{
				motions.push_back(
					std::pair<std::string, std::string>(it.first,
					it.second.get_value<std::string>()));
			}
		}
	}
	motions_it = motions.begin();
}

std::unordered_map<std::string, std::string> ResourceManager::set_motions_ = std::unordered_map<std::string, std::string>();
float ResourceManager::model_edge_size_ = 1.0f;

// ModelHandle ResourceManager::LoadModelFromName(const tstring& name)
ModelHandle ResourceManager::LoadModelFromName(const tstring& name, bool async) // ※ 非同期読み込みを復活させるため修正
{
	auto fullpath = ptree::path_type(unicode::ToString(NameToFullPath(name)), ':');
	ptree p = model_name_tree_.get_child(fullpath, ptree());

	ptree info = p.get_child("_info_", ptree());
	tstring filepath = unicode::ToTString(info.get<std::string>("modelpath", ""));
	if(!filepath.size())
	{
		fullpath = ptree::path_type(unicode::ToString(NameToFullPath(UNKNOWN_MODEL_NAME)), ':');
		p = model_name_tree_.get_child(fullpath, ptree());
		info = p.get_child("_info_", ptree());
		filepath = unicode::ToTString(info.get<std::string>("modelpath", ""));
	}
	if (filepath.size() > 0) {
		auto it = shared_model_data_.find(unicode::ToTString(filepath));
		if (it != shared_model_data_.end()) {
			return ModelHandle(it->second);
		}else{
			auto funcdata = std::make_shared<ReadFuncData>(info);
			set_motions_ = funcdata->set_motions;

			std::shared_ptr<char> FileImage;
			int FileSize;

// ※ ここから ステージかスカイドームの場合はキャッシュを探しキャッシュ済みの場合は元のモデルファイルを読まないように修正
//			LoadFile(unicode::ToTString(filepath).c_str(), &FileImage, &FileSize );
//			// キャッシュ読み込み
//			// モーションなしのモデルでないと上手くいかない
//			auto cache_filename = GetCacheFilename(info, FileImage, FileSize);
//			if (boost::filesystem::exists(cache_filename)) {
			std::string cache_filename;
			if (name.find(L"stage:")== 0||name.find(L"skydome:")== 0) {
				async = false; // ステージ、スカイドームは必ず同期読み込みにする
				auto time = boost::posix_time::from_time_t( boost::filesystem::last_write_time(unicode::ToTString(filepath).c_str()));
                auto keystring = unicode::ToString(filepath) + to_simple_string(time);
				FileSize = strlen(keystring.c_str()) + 1;
				char *nstring = new char[FileSize];
				strcpy_s(nstring, FileSize, keystring.c_str());
				FileImage = (std::shared_ptr<char>)nstring;
				cache_filename = GetCacheFilename(info, FileImage, FileSize);
			}
			if ((!cache_filename.empty()) &&  boost::filesystem::exists(cache_filename)) {
// ※ ここまで

				FileImage.reset();
				LoadFile(unicode::ToTString(cache_filename).c_str(), &FileImage, &FileSize);
			}
// ※ ここから キャッシュがない場合はモデルファイル読み込み
			else {
				FileImage.reset();
				LoadFile(unicode::ToTString(filepath).c_str(), &FileImage, &FileSize );
			}
// ※ ここまで
// ※ ここから  非同期読み込みを復活させるため修正
			if (async) {
				SetUseASyncLoadFlag(TRUE);
			}
// ※ ここまで
			int handle = MV1LoadModelFromMem( FileImage.get(), FileSize, FileReadFunc, FileReleaseFunc, &(*funcdata));

			auto material_num = MV1GetMaterialNum(handle);
			for(int i = 0; i < material_num; ++i){
				MV1SetMaterialType(handle,i,DX_MATERIAL_TYPE_TOON_2);
			}
// ※ ここから  非同期読み込みを復活させるため修正
			// SetMotionNames(handle, *funcdata);
			// // CreateModelCache(handle, info, FileImage, FileSize);
			// SharedModelDataPtr shared_data = 
			//	std::make_shared<SharedModelData>(handle, std::make_shared<ptree>(info));
			if (async) {
				SetUseASyncLoadFlag(FALSE);
			} else {
				SetMotionNames(handle, *funcdata);
			}

			SharedModelDataPtr shared_data = 
				std::make_shared<SharedModelData>(handle,funcdata, std::make_shared<ptree>(info),async);
// ※ ここまで

			shared_model_data_[unicode::ToTString(filepath)] = shared_data;

			Logger::Debug(_T("Model %d"), handle);
			return ModelHandle(shared_data);
		}
	} else {
		return ModelHandle();
	}
}

void ResourceManager::ClearModelHandle()
{
	std::list<tstring> erase_keys;
	BOOST_FOREACH(const auto& it, shared_model_data_) {
		if (it.second.unique()) {
			erase_keys.push_back(it.first);
		}
	}

	BOOST_FOREACH(const tstring& key, erase_keys) {
		// ※ モデル解放時にモデルの種別がワープオブジェクトまたはスカイドームの場合は解放しないように修正
		auto path = key.substr(0,key.rfind('\\'))+ _T("\\info.json");
		auto json_path = boost::filesystem::wpath(unicode::ToWString(path));
		if (exists(json_path)) {
			ptree pt_json;
			read_json(json_path.string(), pt_json);
			auto type = pt_json.get<std::string>("name", "");
			type = type.substr(0,type.find(':'));
			if (type=="skydome"||type=="warpobj" ) {
				continue; // 以下のモデル解放をスキップする
			}
		}
		// ※ ここまで
		shared_model_data_.erase(key);
	}
}

void ResourceManager::SetModelEdgeSize(int handle)
{
	int MaterialNum = MV1GetMaterialNum( handle );
	for( int i = 0 ; i < MaterialNum ; i ++ )
	{
		// マテリアルの元々の輪郭線の太さを取得
		float dotwidth = MV1GetMaterialOutLineDotWidth( handle, i );
		float width = MV1GetMaterialOutLineWidth( handle , i );
		// マテリアルの輪郭線の太さ
		MV1SetMaterialOutLineDotWidth( handle, i, dotwidth * model_edge_size_);
		MV1SetMaterialOutLineWidth( handle, i, width * model_edge_size_);
	}
}


tstring ResourceManager::NameToFullPath(const tstring& name)
{
	if (model_name_tree_.empty()) {
		BuildModelFileTree();
	}

	tstring fullpath;
	ptree info;

	auto name_it = model_names_.find(name);
	if (name_it != model_names_.end()) {
		fullpath = name_it->second;

	} else {
		ptree p;
		auto path = ptree::path_type(unicode::ToString(name), ':');

		p = model_name_tree_.get_child(path, ptree());

		// ルートで探索を打ち切る
		while (1) {
			if (p.empty()) {
				Logger::Debug(_T("EMPTY %s"), unicode::ToTString(path.dump()));
				// 親ノードを検索
				if (!path.single()) {
					return _T("");
					break;
				}
				std::string path_str = path.dump();
				size_t separator_pos = path_str.find_last_of(':');
				assert(separator_pos != std::string::npos);

				path = ptree::path_type(path_str.substr(0, separator_pos), ':');
				p = model_name_tree_.get_child(path, ptree());
			} else {
				info = p.get_child("_info_", ptree());
				if (info.empty()) {
					Logger::Debug(_T("CHILD_FOUND"));
					// データがない場合は最初の子ノードへ移動
					path /= ptree::path_type(p.front().first, ':');
					p = p.get_child(ptree::path_type(p.front().first, ':'), ptree());
				} else {
					Logger::Debug(_T("FOUND"));
					break;
				}
			}

		}

		Logger::Debug(_T("ModelName to fullpath %s -> %s"), name, unicode::ToTString(path.dump()));
		fullpath = unicode::ToTString(path.dump());
		model_names_[name] = fullpath;
	}

	return fullpath;
}

const std::vector<std::string>& ResourceManager::GetModelNameList()
{
	return model_name_list_;
}

void ResourceManager::CacheBakedModel()
{

}

void ResourceManager::set_model_edge_size(float edge_size)
{
	model_edge_size_ = edge_size;
}

MusicPtr ResourceManager::music_ = std::shared_ptr<Music>(new Music);
MusicPtr& ResourceManager::music()
{
	return music_;
}

std::unordered_map<std::string, std::string>& ResourceManager::set_motions()
{
	return set_motions_;
}

ImageHandle::ImageHandle() :
handle_(-1)
{

}

ImageHandle::ImageHandle(int handle) :
handle_(handle)
{
}

ImageHandle::operator int() const
{
	return handle_;
}

//SharedModelData::SharedModelData(int base_handle, const PtreePtr& property) :
SharedModelData::SharedModelData(int base_handle, const ReadFuncDataPtr& funcdata, const PtreePtr& property, bool async_load) : // ※ 非同期読み込み対応のため修正
base_handle_(base_handle),
//	property_(property)  // ※ 非同期読み込み対応のため修正
funcdata_(funcdata),     // ※ 非同期読み込み対応のため修正
property_(property),     // ※ 非同期読み込み対応のため修正
async_load_(async_load)  // ※ 非同期読み込み対応のため修正
{

}

const ptree& SharedModelData::property() const
{
	return *property_;
}

int SharedModelData::DuplicateHandle()
{
	int handle = MV1DuplicateModel(base_handle_);
    if ( handle != -1){
    //assert(handle != -1);
    	handles_.push_back(handle);
	    return handle;
    } else {
        return base_handle_;
    }
}

SharedModelData::~SharedModelData()
{
	BOOST_FOREACH(int handle, handles_) {
		MV1DeleteModel(handle);
	}
	MV1DeleteModel(base_handle_);
}

// ※ ここから  　非同期読み込みを復活させるために追加
bool ModelHandle::CheckLoaded()
{
	if (!shared_data_->async_load_) {
		return true;
	} else if (shared_data_->async_load_ && CheckHandleASyncLoad(handle_) == FALSE) {
        handle_ = MV1DuplicateModel(handle_);
		SetMotionNames(handle_, *shared_data_->funcdata_);
		shared_data_->async_load_ = false;
		return true;
	} else {
		return false;
	}
}
// ※ ここまで


ModelHandle::ModelHandle(const SharedModelDataPtr& shared_data) :
shared_data_(shared_data),
	handle_(shared_data->DuplicateHandle())
{

}

ModelHandle::ModelHandle()
{

}

ModelHandle::operator bool() const
{
	return static_cast<bool>(shared_data_);
}

int ModelHandle::handle() const
{
	return handle_;
}

const ptree& ModelHandle::property() const
{
	return shared_data_->property();
}

std::string ModelHandle::name() const
{
	return property().get<std::string>("name", "");
}