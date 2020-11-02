#include "helpers.h"

bool replace(std::string& str, const std::string& from, const std::string& to) {
	size_t start_pos = str.find(from);
	if (start_pos == std::string::npos)
		return false;
	str.replace(start_pos, from.length(), to);
	return true;
}

godot::Quat LT2GodotQuat(DRotation* pDRotation)
{
	return godot::Quat(pDRotation->m_Vec.x, pDRotation->m_Vec.y, pDRotation->m_Vec.z, pDRotation->m_Spin);
}
