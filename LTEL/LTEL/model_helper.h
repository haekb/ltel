#pragma once
#include <Godot.hpp>
#include <Spatial.hpp>
#include "game_object.h"


class ModelHelper : public godot::Spatial {
    GODOT_CLASS(ModelHelper, godot::Spatial);
public:
    ModelHelper();
    ~ModelHelper();

    /** `_init` must exist as it is called by Godot. */
    void _init() { }

    void OnAnimationCommandString(godot::String sCommandString);

    void SetGameObject(GameObject* pObj);

    GameObject* m_pObj;


    static void _register_methods() {
        register_method("on_animation_command_string", &ModelHelper::OnAnimationCommandString);
    }
};