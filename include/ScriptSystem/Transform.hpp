#pragma  once

#include <string>

struct Vector2
{
	float x = 0;
	float y = 0;
};

struct Vector3
{
	float x = 0;
	float y = 0;
	float z = 0;
};

class Transform3D
{
public:
	void setPosition(float x, float y, float z)
	{
		m_position.x = x;
		m_position.y = y;
		m_position.z = z;
	};

	void setRotation(float x, float y, float z)
	{
		m_rotation.x = x;
		m_rotation.y = y;
		m_rotation.z = z;
	};

	void setScale(float x, float y, float z)
	{
		m_scale.x = x;
		m_scale.y = y;
		m_scale.z = z;
	};

	float getPX() const { return m_position.x; };
	float getPY() const { return m_position.y; };
	float getPZ() const { return m_position.z; };

	float getRX() const { return m_rotation.x; };
	float getRY() const { return m_rotation.y; };
	float getRZ() const { return m_rotation.z; };

	float getSX() const { return m_scale.x; };
	float getSY() const { return m_scale.y; };
	float getSZ() const { return m_scale.z; };

	Vector3 getPos() const{ return m_position; };
	Vector3 getRot() const{ return m_rotation; };
	Vector3 getScale() const{ return m_scale; };
	
private:
	Vector3 m_position;
	Vector3 m_rotation;
	Vector3 m_scale;
};

class Transform2D
{
public:
	void setPosition(float x, float y)
	{
		m_position.x = x;
		m_position.y = y;
	};

	void setRotation(float x, float y)
	{
		m_rotation.x = x;
		m_rotation.y = y;
	};

	void setScale(float x, float y)
	{
		m_scale.x = x;
		m_scale.y = y;
	};

	float getPX() const { return m_position.x; };
	float getPY() const { return m_position.y; };

	float getRX() const { return m_rotation.x; };
	float getRY() const { return m_rotation.y; };

	float getSX() const { return m_scale.x; };
	float getSY() const { return m_scale.y; };

	Vector2 getPos() const { return m_position; };
	Vector2 getRot() const { return m_rotation; };
	Vector2 getScale() const { return m_scale; };

private:
	Vector2 m_position;
	Vector2 m_rotation;
	Vector2 m_scale;
};
