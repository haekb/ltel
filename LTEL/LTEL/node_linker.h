#pragma once
#include <Godot.hpp>
#include <Spatial.hpp>

class GameObject;

class NodeLinker : public godot::Spatial {
    GODOT_CLASS(NodeLinker, godot::Spatial);
public:
    NodeLinker();
    ~NodeLinker();

    /** `_init` must exist as it is called by Godot. */
    void _init() { }

    void SetGameObject(GameObject* pObj);
    GameObject* GetGameObject() { return m_pObj; };

    static void _register_methods() {}

protected:
    GameObject* m_pObj;
};