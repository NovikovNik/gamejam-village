#pragma once
#include <libs/CPPNanoString/includes/CPPNanoString.h>

nnstrINIT_TABLES(_nnBoxName, std::mutex, mt, std::vector, boxNamesRn, 64, std::array, boxNamesCt)
using BoxName = nnstr::NanoString;
