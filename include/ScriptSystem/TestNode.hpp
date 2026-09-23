#pragma  once

#include <string>

/// @brief Testnode to test stuff with
class TestNode
{
public: 
	/// @brief setPositon of node
	/// @param x, y, z cordinates
	void setPosition(float x, float y, float z)
	{
		m_x = x;
		m_y = y;
		m_z = z;
	};
		
	/// @brief GetX position of node
	/// @return x coordinate as a float
	float getX() const;
	float getY() const;
	float getZ() const;

	void printMessage(const std::string& message);

private:
	float m_x = 0.0f;
	float m_y = 0.0f;
	float m_z = 0.0f; 
};