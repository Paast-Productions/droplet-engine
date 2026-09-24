#include "TestNode.hpp"
#include <iostream>

using namespace Droplet::Script;

float TestNode::getX() const
{
	return m_x;
}

float TestNode::getY() const
{
	return m_y;
}

float TestNode::getZ() const
{
	return m_z;
}

void TestNode::printMessage(const std::string& message)
{
	std::cout << "TestNode Message: " << message << std::endl;
}