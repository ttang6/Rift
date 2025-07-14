#ifndef RIFT_COMMON_CONFIG_H
#define RIFT_COMMON_CONFIG_H

#include <map>
#include <string>

namespace rift{
    class Config{
        public:
            Config(const char* xmlfile);

        public:
           static Config* getGlobalConfig();
           static void setGlobalConfig(const char* xmlfile);
        
        public:
            std::string m_log_level;

    };
}

#endif