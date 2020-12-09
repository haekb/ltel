#include "node_linker.h"

NodeLinker::NodeLinker()
{
	m_pObj = nullptr;
}

NodeLinker::~NodeLinker()
{
}

void NodeLinker::SetGameObject(GameObject* pObj)
{
	m_pObj = pObj;
}
