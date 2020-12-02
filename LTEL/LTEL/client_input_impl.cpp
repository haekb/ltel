#include "client.h"
#include "shared.h"
#include "helpers.h"

extern LTELClient* g_pLTELClient;

DRESULT impl_GetDeviceName(DDWORD nDeviceType, char* pStrBuffer, DDWORD nBufferSize)
{
	// Input not supported yet!
	return DE_NOTFOUND;
}

DeviceObject* impl_GetDeviceObjects(DDWORD nDeviceFlags)
{
	return nullptr;
}

void impl_FreeDeviceObjects(DeviceObject* pList)
{
	return;
}

DeviceBinding* impl_GetDeviceBindings(DDWORD nDevice)
{
	return nullptr;
}

void impl_FreeDeviceBindings(DeviceBinding* pBindings)
{
	return;
}

void impl_SetInputState(DBOOL bOn)
{
	godot::Godot::print("[impl_SetInputState] Input State: {0}", bOn);
	g_pLTELClient->m_bAllowInput = bOn;

	if (!bOn)
	{
		g_pLTELClient->ClearInput();
	}
}

DRESULT impl_ClearInput()
{
	g_pLTELClient->m_mCommands.clear();
	return DE_OK;
}


DBOOL impl_IsCommandOn(int commandNum)
{
	bool bOn = FALSE;

	if (g_pLTELClient->m_mCommands.find(commandNum) != g_pLTELClient->m_mCommands.end())
	{
		bOn = g_pLTELClient->m_mCommands.at(commandNum);
	}

	return bOn;
}

void impl_GetAxisOffsets(DFLOAT* offsets)
{
	offsets[0] = 0.0f;
	offsets[1] = 0.0f;
	offsets[2] = 0.0f;


	POINT lpPoint;
	int deltaX = 0, deltaY = 0;

	deltaX = g_pLTELClient->m_vRelativeMouse.x;
	deltaY = g_pLTELClient->m_vRelativeMouse.y;

	static int m_iCurrentMouseX = 0;
	static int m_iCurrentMouseY = 0;
	static int m_iPreviousMouseX = 0;
	static int m_iPreviousMouseY = 0;
	static float m_fMouseSensitivity = 0.10f;

	m_iCurrentMouseX += deltaX;
	m_iCurrentMouseY += deltaY;

	float nScaleX = m_fMouseSensitivity + (1.0f * m_fMouseSensitivity);

	// Nerf the sensitivity scale so it matches the OG games.
	nScaleX *= 0.10f;
	float nScaleY = nScaleX;

	offsets[0] = (float)(m_iCurrentMouseX - m_iPreviousMouseX) * nScaleX;
	offsets[1] = (float)(m_iCurrentMouseY - m_iPreviousMouseY) * (nScaleY);
	offsets[2] = 0.0f;

	//g_pLTELClient->CPrint((char*)"[GetAxisOffset] %f/%f/%f", offsets[0], offsets[1], offsets[2]);

	m_iPreviousMouseX = m_iCurrentMouseX;
	m_iPreviousMouseY = m_iCurrentMouseY;

	// Cache the results so it can be used again this frame
	//m_fOffsets[0] = offsets[0];
	//m_fOffsets[1] = offsets[1];
	//m_fOffsets[2] = offsets[2];

	//m_bGetAxisOffsetCalledThisFrame = true;
}

void LTELClient::InitInputImpl()
{
	// Input functionality
	GetDeviceName = impl_GetDeviceName;
	GetDeviceObjects = impl_GetDeviceObjects;
	FreeDeviceObjects = impl_FreeDeviceObjects;
	GetDeviceBindings = impl_GetDeviceBindings;
	FreeDeviceBindings = impl_FreeDeviceBindings;
	SetInputState = impl_SetInputState;
	ClearInput = impl_ClearInput;
	IsCommandOn = impl_IsCommandOn;
	GetAxisOffsets = impl_GetAxisOffsets;
}