#include "helpers.h"





// Helper functions
bool replace(std::string& str, const std::string& from, const std::string& to) {
	size_t start_pos = str.find(from);
	if (start_pos == std::string::npos)
		return false;
	str.replace(start_pos, from.length(), to);
	return true;
}

godot::Color LT2GodotColor(HDECOLOR hColor)
{
	float r = GETR(hColor);
	float g = GETG(hColor);
	float b = GETB(hColor);
	float a = 1.0f;

	
	if (hColor & COLOR_TRANSPARENCY_MASK)
	{
		a = 0.5f;
	}
	
	r /= 255;
	g /= 255;
	b /= 255;

	return godot::Color(r, g, b, a);
}

godot::Vector3 LT2GodotVec3(DVector pVector)
{
	return godot::Vector3( pVector.x, pVector.y, pVector.z );
}

godot::Quat LT2GodotQuat(DRotation* pDRotation)
{
	return godot::Quat(pDRotation->m_Vec.x, pDRotation->m_Vec.y, pDRotation->m_Vec.z, pDRotation->m_Spin);
}

GameObject* HObject2GameObject(HOBJECT hObj)
{
	if (!hObj)
	{
		return nullptr;
	}

	return (GameObject*)hObj;
}