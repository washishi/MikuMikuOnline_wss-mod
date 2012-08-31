#pragma once

#include <vector>
#include <DxLib.h>
#include "../../common/unicode.hpp"


class PMDLoader
{
public:
    int Load(const tstring& pmd_path, const tstring& dir_path, const std::vector<tstring>& basic_motion_paths);

private:
    static int FileReadFunc(const TCHAR* file_path, void** file_image_addr, int* file_size, void* this_ptr);
    static int FileReleaseFunc(void* memory_addr, void* this_ptr);

    tstring model_file_path_;
    tstring data_dir_path_;
    std::vector<tstring> basic_motion_paths_;
};
