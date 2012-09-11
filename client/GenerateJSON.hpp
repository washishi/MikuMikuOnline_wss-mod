#pragma once

#include <io.h>
#include <fcntl.h>
#include <vector>
#include <string>
#include <math.h>
#include <Windows.h>
#include <shlwapi.h>
#include <DxLib.h>

#pragma comment(lib, "shlwapi.lib")

#define PMDINFO_SIZE 0x11A

class JsonGen
{
public:
	JsonGen();
	virtual ~JsonGen(){};
};