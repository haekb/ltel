#pragma once
#include "client.h"
#include <string>
#include <Quat.hpp>

// helpers
bool replace(std::string& str, const std::string& from, const std::string& to);

godot::Quat LT2GodotQuat(DRotation* pDRotation);