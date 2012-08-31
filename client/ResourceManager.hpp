//
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

typedef std::shared_ptr<ImageHandle> ImageHandlePtr;
typedef std::shared_ptr<ModelHandle> ModelHandlePtr;

class ResourceManager {

    public:
        static void ClearCache();

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
        static ModelHandle LoadModelFromName(const tstring&);

    private:
        static int default_font_handle_;
        static std::unordered_map<tstring, ImageHandlePtr> graph_handles_;
        static std::unordered_map<tstring, std::vector<ImageHandlePtr>> div_graph_handles_;

        static ptree model_name_tree_;
        static std::unordered_map<tstring, tstring> model_names_;
        static std::unordered_map<tstring, ModelHandle> model_handles_;

};

class ImageHandle {
    public:
        ImageHandle();
        explicit ImageHandle(int handle);
        operator int() const;

    private:
        int handle_;
};

class ModelHandle {
    friend class ResourceManager;

    private:
        ModelHandle(int handle, const std::shared_ptr<ptree>& property, bool async_load = false);
        ModelHandle Clone();

    public:
        ModelHandle();
        ~ModelHandle();

        int handle() const;
        const ptree& property() const;
        std::string name() const;

    private:
        int handle_;
        std::shared_ptr<ptree> property_;
        std::string name_;
        bool async_load_;
};
