#ifndef RIFT_INCLUDE_KRPCCONFIG_H
#define RIFT_INCLUDE_KRPCCONFIG_H

#include <unordered_map>
#include <string>

class Config{
    public:
    void loadConfigFile(const char *config_file); //加载配置文件
    std::string load(const std::string &key); //查找key对应的value
    private:
    std::unordered_map<std::string, std::string> config_map;
    void trim(std::string &read_buf); //去掉字符串前后的空格
};

#endif
