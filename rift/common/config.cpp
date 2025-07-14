#include <tinyxml2.h>
#include <iostream>
#include <string>
#include "rift/common/config.h"

#define READ_XML_NODE(name, parent)\
    tinyxml2::XMLElement* name##_node = parent->FirstChildElement(#name);\
    if (!name##_node){\
        std::cout << "Start server error, failed to read node [" << #name << "]" << std::endl;\
        exit(0);\
    }\
    // printf("Element name: %s\n", name##_node->Name());

#define READ_STR_FROM_NODE(name, parent)\
    tinyxml2::XMLElement* name##_node = parent->FirstChildElement(#name);\
    if (!name##_node || !name##_node->GetText()){\
        printf("Start server error, failed to read node %s\n", "#name");\
        exit(0);\
    }\
    std::string name##_str = std::string(name##_node->GetText());

namespace rift{
    static Config* g_config = nullptr;

    Config* Config::getGlobalConfig(){
        return g_config;
    }

    void Config::setGlobalConfig(const char* xmlfile){
        if (g_config == nullptr){
            g_config = new Config(xmlfile);
        }
    }
    
    Config::Config(const char* xmlfile){
        tinyxml2::XMLDocument* xml_doc = new tinyxml2::XMLDocument();
        // printf("Try to open xml file: %s\n", xmlfile);
        int rt = xml_doc->LoadFile(xmlfile);
        // printf("rt: %d\n", rt);
        if (rt != 0){
            std::cout << "Start server error, failed to load file: " << xmlfile << std::endl;
            exit(0);
        }

        READ_XML_NODE(root, xml_doc);
        READ_XML_NODE(log, root_node);

        READ_STR_FROM_NODE(level, log_node);

        m_log_level = level_str;
        
        delete xml_doc;
    }
}