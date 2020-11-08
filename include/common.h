//
// Created by matf-rg on 8.11.20..
//

#ifndef PROJECT_BASE_COMMON_H
#define PROJECT_BASE_COMMON_H
#include <string>
#include <fstream>
#include <sstream>

std::string readFileContents(std::string path) {
    std::ifstream in(path);
    std::stringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
}

void appendShaderFolderIfNotPresent(std::string& path) {
    std::ifstream file(path);
    if (!file) {
        path = "resources/shaders/" + path;
    }
}
#endif //PROJECT_BASE_COMMON_H
