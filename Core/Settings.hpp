#pragma once

class Settings
{
public:
	enum class Type
	{
		Render,
		World
	};

	virtual ~Settings() {}
};