#pragma once
#include "Settings.hpp"
#include <map>

class SettingsHolder
{
public:
	SettingsHolder() {}
	SettingsHolder(const SettingsHolder& rhs) = delete;
	SettingsHolder(SettingsHolder&& rhs) = delete;
	SettingsHolder& operator=(const SettingsHolder& rhs) = delete;
	SettingsHolder& operator=(SettingsHolder&& rhs) = delete;

	~SettingsHolder()
	{
		for (auto& setting : m_settings)
		{
			delete setting.second;
		}
	}

	void addSetting(Settings::Type type, Settings* setting)
	{
		m_settings[type] = setting;
	}

	template<typename T>
	T* getSetting(Settings::Type type)
	{
		auto foundIt = m_settings.find(type);
		Settings* ret = foundIt != m_settings.end() ? foundIt->second : nullptr;

		return static_cast<T*>(ret);
	}

	static SettingsHolder& getInstance()
	{
		static SettingsHolder holder;
		return holder;
	}

private:
	std::map<Settings::Type, Settings*> m_settings;
};