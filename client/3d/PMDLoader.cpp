#include "PMDLoader.hpp"
#include "BasicMotion.hpp"

namespace
{

unsigned int VMDNumberToInt(const tstring& vmd_path)
{
    auto dot_pos = vmd_path.find('.');
    tstring vmd_number = (dot_pos != tstring::npos) ? vmd_path.substr(0, dot_pos) : vmd_path;

//    std::cout << "vmd number = " << vmd_number << std::endl;

    if (vmd_number.length() == 4 && vmd_number[3] == 'L')
    {
        vmd_number = vmd_number.substr(0, 3);
    }

    if (vmd_number.length() != 3)
    {
        throw std::logic_error("vmd path doesn't have exactly 3 digits");
    }

    unsigned int num[3];
    for (size_t i = 0; i < 3; ++i)
    {
        num[i] = vmd_number[i] - '0';
        if (num[i] > 9)
        {
            throw std::logic_error("vmd path doesn't have exactly 3 digits");
        }
    }

    return num[0] * 100 + num[1] * 10 + num[2];
}

} // namespace

int PMDLoader::Load(const tstring& pmd_path, const tstring& dir_path, const std::vector<tstring>& basic_motion_paths)
{
    auto pmd_dirdelim_pos = pmd_path.rfind(_T("/"));

    if (pmd_dirdelim_pos != std::string::npos)
    {
        model_file_path_ = pmd_path.substr(pmd_dirdelim_pos + 1);
        if (dir_path.empty())
        {
            data_dir_path_ = pmd_path.substr(0, pmd_dirdelim_pos + 1);
        }
        else
        {
            data_dir_path_ = dir_path;
        }
    }
    else
    {
        model_file_path_ = pmd_path;
        data_dir_path_ = dir_path;
    }

    basic_motion_paths_ = basic_motion_paths;

//    std::cout << "PMDLoader::Load: PMD=" << pmd_path << ", DATA_DIR=" << data_dir_path_ << std::endl;

    std::ifstream pmd_file(unicode::ToString(pmd_path), std::ios::binary);
    auto end_pos = pmd_file.seekg(0, std::ios::end).tellg();
    pmd_file.seekg(0, std::ios::beg);

    std::vector<char> buf(static_cast<std::vector<char>::size_type>(end_pos));
    pmd_file.read(buf.data(), buf.size());

    return MV1LoadModelFromMem(buf.data(), buf.size(), PMDLoader::FileReadFunc, PMDLoader::FileReleaseFunc, this);
}

int PMDLoader::FileReadFunc(const TCHAR* file_path, void** file_image_addr, int* file_size, void* this_ptr)
{
    PMDLoader* this_ = static_cast<PMDLoader*>(this_ptr);

    tstring file_path_(file_path);
    if (file_path_.find(_T(".vmd")) != std::string::npos)
    {
        int vmd_number = VMDNumberToInt(file_path_);
        bool loop_motion = file_path_[3] == _T('L');

        /*
        auto model_file_dotpos = this_->model_file_path.rfind('.');
        if (model_file_dotpos != std::string::npos)
        {
            std::cout << "vmd file required: " << file_path_ << std::endl;
            file_path_ = this_->model_file_path.substr(0, model_file_dotpos) + file_path_;
        }
        else
        {
            return -1;
        }
        */
        if (static_cast<unsigned>(vmd_number) < this_->basic_motion_paths_.size())
        {
//            if ((vmd_number == BasicMotion::STAND && loop_motion) ||
//                    (vmd_number == BasicMotion::WALK && !loop_motion) ||
//                    (vmd_number == BasicMotion::RUN && !loop_motion))
//            {
//                return -1;
//            }
            file_path_ = this_->basic_motion_paths_[vmd_number];
        }
        else
        {
            auto model_file_dotpos = this_->model_file_path_.rfind('.');
            if (model_file_dotpos != std::string::npos)
            {
                file_path_ = this_->model_file_path_.substr(0, model_file_dotpos) +
                    (tformat(loop_motion ? _T("%03dL.vmd") : _T("%03d.vmd"))
                             % (vmd_number - this_->basic_motion_paths_.size())).str();
            }
            else
            {
                return -1;
            }
        }
    }

    tstring path(this_->data_dir_path_ + file_path_);
    std::ifstream file(unicode::ToString(path), std::ios::binary);
    if (!file)
    {
//        std::cout << "can't find in specified data directory: " << file_path_ << std::endl;
        path = this_->data_dir_path_ + _T("toon/") + file_path_;
        file.open(unicode::ToString(path), std::ios::binary);
        if (!file)
        {
            // std::cout << "can't open file: " << file_path_ << std::endl;
            return -1;
        }
    }

    auto end_pos = file.seekg(0, std::ios::end).tellg();
    file.seekg(0, std::ios::beg);

    auto size = static_cast<std::vector<char>::size_type>(end_pos);
    char* buf = new char[size];
    *file_size = size;
    *file_image_addr = buf;

    file.read(buf, size);

    return static_cast<size_t>(file.gcount()) == size ? 0 : -1;
}

int PMDLoader::FileReleaseFunc(void* memory_addr, void* this_ptr)
{
    char* buf = static_cast<char*>(memory_addr);
    delete[] buf;
    return 0;
}
