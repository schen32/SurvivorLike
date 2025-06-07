#pragma once

#include "Animation.hpp"
#include <fstream>
#include <iostream>
#include <cassert>

class Assets
{
public:
	std::unordered_map<std::string, sf::Texture> m_textureMap;
	std::unordered_map<std::string, Animation> m_animationMap;
	std::unordered_map<std::string, sf::Font> m_fontMap;

	void addTexture(const std::string& textureName, const std::string& path,
		bool smooth = true)
	{
		m_textureMap[textureName] = sf::Texture();

		if (!m_textureMap[textureName].loadFromFile(path))
		{
			std::cerr << "Could not load texture from file: " << path << std::endl;
			return;
		}

		m_textureMap[textureName].setSmooth(true);
	}

	void addAnimation(const std::string& animationName, const std::string& textureName,
		size_t frameCount, size_t speed)
	{
		m_animationMap[animationName] = Animation(animationName, m_textureMap[textureName],
			frameCount, speed);
	}

	void addFont(const std::string& fontName, const std::string& path)
	{
		m_fontMap[fontName] = sf::Font();
		if (!m_fontMap[fontName].openFromFile(path))
		{
			std::cerr << "Could not load font file: " << path << std::endl;
		}
	}
	
	Assets() = default;
	void loadFromFile(const std::string& path)
	{
		auto file = std::ifstream(path);
		std::string str;
		while (file.good())
		{
			file >> str;
			if (str == "Texture")
			{
				std::string name, path;
				file >> name >> path;
				addTexture(name, path);
			}
			else if (str == "Animation")
			{
				std::string name, texture;
				size_t frames, speed;
				file >> name >> texture >> frames >> speed;
				addAnimation(name, texture, frames, speed);
			}
			else if (str == "Font")
			{
				std::string name, path;
				file >> name >> path;
				addFont(name, path);
			}
			else
			{
				std::cerr << "Unknown Asset Type: " << str << std::endl;
			}
		}
	}

	const sf::Texture& getTexture(const std::string& textureName) const
	{
		assert(m_textureMap.find(textureName) != m_textureMap.end());
		return m_textureMap.at(textureName);
	}

	const Animation& getAnimation(const std::string& animationName) const
	{
		assert(m_animationMap.find(animationName) != m_animationMap.end());
		return m_animationMap.at(animationName);
	}

	const sf::Font& getFont(const std::string& fontName) const
	{
		assert(m_fontMap.find(fontName) != m_fontMap.end());
		return m_fontMap.at(fontName);
	}
};