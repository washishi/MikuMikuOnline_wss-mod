﻿//
// ResourceManager.hpp
//

#pragma once

#include <DxLib.h>
#include <string>
#include <array>
#include <map>
#include <memory>
#include <vector>
#include <unordered_map>
#include "../common/unicode.hpp"
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>

using boost::property_tree::ptree;

class ImageHandle;
class ModelHandle;
class Music;

class SharedModelData;
class ModelHandle;

typedef std::shared_ptr<ImageHandle> ImageHandlePtr;
typedef std::shared_ptr<ModelHandle> ModelHandlePtr;
typedef std::shared_ptr<Music> MusicPtr;
typedef std::shared_ptr<SharedModelData> SharedModelDataPtr;

struct ReadFuncData {
	ReadFuncData(){};
	ReadFuncData(const ptree& info);
    boost::filesystem::wpath model_dir;
    std::list<std::pair<std::string, std::string>> motions;
    std::unordered_map<std::string, std::string> set_motions;
    std::list<std::pair<std::string, std::string>>::iterator motions_it;
};
typedef std::shared_ptr<ReadFuncData> ReadFuncDataPtr;

class ResourceManager {
	private:
		static class MemoryPool {
			private:
				char mem_block_[1024*1024*512];
				tlsf_pool pool_;
			public:
				MemoryPool()
				{
					pool_ = tlsf_create(mem_block_,sizeof(mem_block_));
				}

				~MemoryPool()
				{
					tlsf_destroy(pool_);
				}

				tlsf_pool& pool()
				{
					return pool_;
				}
		} mempool;

    public:
        static void ClearCache();

    public:
        static tlsf_pool& memory_pool();

    // Fonts
    public:
        static int default_font_handle();
        static int default_font_size();

    // Graphics
    public:
        static ImageHandlePtr LoadCachedGraph(const tstring&);

        template <std::size_t AllNum>
        static std::array<ImageHandlePtr, AllNum> LoadCachedDivGraph(const tstring& filename, int XNum, int YNum, int XSize, int YSize)
        {
            if(div_graph_handles_.find(filename) == div_graph_handles_.end()) {
                std::array<int, AllNum> int_handles;
                if(DxLib::LoadDivGraph(filename.c_str(), AllNum, XNum, YNum, XSize, YSize, int_handles.data()) == -1){
                    return std::array<ImageHandlePtr, AllNum>();
                }
                std::vector<ImageHandlePtr> handle_vector;

                for (auto it = int_handles.begin(); it != int_handles.end(); ++it) {
                    auto item = *it;
                    handle_vector.push_back(std::make_shared<ImageHandle>(item));
                }
                div_graph_handles_[filename] = handle_vector;
            }

            std::array<ImageHandlePtr, AllNum> handles;
            const std::vector<ImageHandlePtr>& handle_vector = div_graph_handles_[filename];
            for (std::size_t i = 0; i < AllNum; i++) {
                handles[i] = handle_vector[i];
            }

            return handles;
        }

        // models
        static void BuildModelFileTree();
        static void CacheBakedModel();

//      static ModelHandle LoadModelFromName(const tstring&);
		static ModelHandle LoadModelFromName(const tstring&, bool async = false); // ※ 非同期読み込みを復活させるため修正
        static void ClearModelHandle();

		static std::string GetCacheFilename(const ptree& info, const std::shared_ptr<char>& fileimage, int filesize);
		static void CreateModelCache(std::string filepath, const ptree& info);

        static void RequestModelFromName(const tstring&);
        static bool IsCachedModelName(const tstring&);
		static const std::vector<std::string>& GetModelNameList();

		static void SetModelEdgeSize(int handle);
		static void set_model_edge_size(float edge_size);

		static tstring NameToFullPath(const tstring& name);

		//Musics
		static MusicPtr& music();

		static std::unordered_map<std::string, std::string>& set_motions();

private:
        static int default_font_handle_;
        static std::unordered_map<tstring, ImageHandlePtr> graph_handles_;
        static std::unordered_map<tstring, std::vector<ImageHandlePtr>> div_graph_handles_;

        static ptree model_name_tree_;
        static std::unordered_map<tstring, tstring> model_names_;
        static std::unordered_map<tstring, ModelHandle> model_handles_;

        static std::unordered_map<tstring, SharedModelDataPtr> shared_model_data_;

		static std::vector<std::string> model_name_list_;
		static float model_edge_size_;
		//Musics
		static MusicPtr music_;

		static const ptree& GetDefaultInfoJSON();

		//Motions
		static std::unordered_map<std::string, std::string> set_motions_;

};

class ImageHandle {
    public:
        ImageHandle();
        explicit ImageHandle(int handle);
        operator int() const;

    private:
        int handle_;

	public:
		void *operator new(size_t size)
		{
			return tlsf_new(ResourceManager::memory_pool(), size);
		}
		void *operator new(size_t, void *p){return p;}
		void operator delete(void *p)
		{
			tlsf_delete(ResourceManager::memory_pool(), p);
		}
		void operator delete(void *, void *){};

};

typedef std::shared_ptr<ptree> PtreePtr;

class SharedModelData {
	public:
//		SharedModelData(int base_handle, const PtreePtr& property);
		SharedModelData(int base_handle, const ReadFuncDataPtr& funcdata, const PtreePtr& property, bool async_load = false); // ※非同期読み込みを行えるよう修正
		~SharedModelData();

		const ptree& property() const;
		int DuplicateHandle();
		operator bool() const;     // ※非同期読み込みを行えるよう修正
		ReadFuncDataPtr funcdata_; // ※非同期読み込みを行えるよう修正
		bool async_load_;          // ※非同期読み込みを行えるよう修正
	private:
		int base_handle_;
		std::list<int> handles_;
		PtreePtr property_;


};

class ModelHandle {
	public:
		ModelHandle(const SharedModelDataPtr& shared_data);
		ModelHandle();
		operator bool() const;
	    int handle() const;
        const ptree& property() const;
        std::string name() const;
		int CheckLoaded(); // ※ 非同期読み込みを復活させるために追加
	private:
		SharedModelDataPtr shared_data_;
		int handle_;
		// ※ ここから  非同期読み込みを復活させるために追加
		//	ReadFuncDataPtr funcdata_;
		//	bool async_load_;
		// ※ ここまで
	public:
		void *operator new(size_t size)
		{
			return tlsf_new(ResourceManager::memory_pool(), size);
		}
		void *operator new(size_t, void *p){return p;}
		void operator delete(void *p)
		{
			tlsf_delete(ResourceManager::memory_pool(), p);
		}
		void operator delete(void *, void *){};
};