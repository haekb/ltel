#include "model_helper.h"

ModelHelper::ModelHelper()
{
	m_pObj = nullptr;
}

ModelHelper::~ModelHelper()
{
}

void ModelHelper::OnAnimationCommandString(godot::String sCommandString)
{
	m_pObj->SendAnimationCommandString(sCommandString);
}

void ModelHelper::SetGameObject(GameObject* pObj)
{
	m_pObj = pObj;

	pObj->GetNode()->connect("animation_command_string", this, "on_animation_command_string");
}
