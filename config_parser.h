//
//	config_parser.h
//	LogCompression
//	
//	Created by Chongbo Wei on 2019/4/12
//	Copyright © 2019 Chongbo Wei. All rights reserved.

#ifndef CONFIG_PARSER_H_INCLUDED
#define CONFIG_PARSER_H_INCLUDED

#include "base.h"

class ConfigParser {

private:
	YAML::Node node;
	Attributes attributes;
	std::string filepath;
	uint32_t memory_MB;
	static ConfigParser *m_Instance;
	ConfigParser();

public:
	static ConfigParser GetInstance() {
		if(m_Instance == NULL)
				m_Instance = new ConfigParser();
			return *m_Instance;
	}

	uint32_t getAttributesNum() {
		return this->attributes.num;
	}

	Attributes getAttributes() {
		return this->attributes;
	}

	std::vector<Attribute> getAttributeVec() {
		return this->attributes.attributeVec;
	}

	std::string getFilePath() {
		return this->filepath;
	}

	uint32_t getMemoryLimit() {
		return this->memory_MB;
	}
};

ConfigParser::ConfigParser() {
	try {
		node = YAML::LoadFile("config.yaml");
		const YAML::Node& attributesNode = node["attributes"];
		const YAML::Node& detailNode = attributesNode["detail"];
		this->filepath = node["filepath"].as<std::string>();
		this->memory_MB = node["Memery(MB)"].as<uint32_t>();
		this->attributes.num = attributesNode["amount"].as<int>();
		this->attributes.separator = attributesNode["separator"].as<std::string>()[0];
		for(uint32_t i=0; i<this->attributes.num; i++) {
			Attribute attribute;
			attribute.name = detailNode[i]["name"].as<std::string>();
			attribute.type = getDateTypeByStr(detailNode[i]["type"].as<std::string>().c_str());
			this->attributes.attributeVec.push_back(attribute);
		}
	} catch (YAML::ParserException& e) {
		std::cout << e.what() << std::endl;
	}
}

ConfigParser *ConfigParser::m_Instance = NULL;

#define CONFIGURATION ConfigParser::GetInstance()

const uint32_t AttributesNum = CONFIGURATION.getAttributesNum();

#endif // CONFIG_PARSER_H_INCLUDED
