#include "BehaviourInspector.hpp"

using namespace Droplet::Editor;

BehaviourInspector::BehaviourInspector(std::shared_ptr<Droplet::Engine> p_engine) : EditorWindow(p_engine)
{
}

void BehaviourInspector::InitImpl()
{
	SetName("Behaviours");
}

void BehaviourInspector::CloseImpl()
{
}

void BehaviourInspector::RenderImpl()
{
}

void BehaviourInspector::RenderToolbar()
{
}
