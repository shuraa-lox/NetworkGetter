#include "wifi_scanner.h"
#include <iostream>
#include <cstdio>
#include <string_view>
#include <algorithm>

#ifdef _WIN32
    #define POPEN _popen
    #define PCLOSE _pclose
    extern "C" FILE* _popen(const char*, const char*);
    extern "C" int _pclose(FILE*);
#else
    #define POPEN popen
    #define PCLOSE pclose
#endif

std::string_view trim_sv(std::string_view sv) {
    auto first = sv.find_first_not_of(" \t\r\n");
    if (first == std::string_view::npos) return "";
    sv.remove_prefix(first);
    auto last = sv.find_last_not_of(" \t\r\n");
    sv.remove_suffix(sv.size() - last - 1);
    return sv;
}

std::vector<std::string> get_wifi_networks() {
    std::system("chcp 437 > nul");
    std::string rawOutput;
    char buffer[256];
    
    FILE* pipe = POPEN("netsh wlan show profiles", "r");
    if (!pipe) return {};

    while (fgets(buffer, sizeof(buffer), pipe)) {
        rawOutput += buffer;
    }
    PCLOSE(pipe);

    std::vector<std::string> wifiNames;
    std::string_view view = rawOutput;
    const std::string_view target = "All User Profile";

    size_t pos = view.find(target);
    while (pos != std::string_view::npos) {
        view.remove_prefix(pos + target.size());
        size_t colonPos = view.find(':');
        size_t endLine = view.find('\n');

        if (colonPos != std::string_view::npos && (endLine == std::string_view::npos || colonPos < endLine)) {
            size_t len = (endLine == std::string_view::npos) ? std::string_view::npos : (endLine - colonPos - 1);
            std::string_view nameView = trim_sv(view.substr(colonPos + 1, len));
            if (!nameView.empty()) wifiNames.emplace_back(nameView);
        }
        pos = view.find(target);
    }
    return wifiNames;
}