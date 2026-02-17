#pragma once
#include <libs/CPPNanoString/includes/CPPNanoString.h>

nnstrINIT_TABLES(_nnInteractId, std::mutex, mt, std::vector, interactIdsRn, 64, std::array, interactIdsCt)
using InteractId = nnstr::NanoString;
