#pragma  once

#include <string>

class TestNode
{
public: 
	void setPosition(float x, float y, float z)
	{
		m_x = x;
		m_y = y;
		m_z = z;
	};
		
	float getX() const;
	float getY() const;
	float getZ() const;

	void printMessage(const std::string& message);

private:
	float m_x = 0.0f;
	float m_y = 0.0f;
	float m_z = 0.0f;
};