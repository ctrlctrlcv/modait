#pragma once
#include "OlkFPC.hpp"

class MacroParser {
public:
    static MacroFormat detectFormat(const std::filesystem::path& path);
    static Result<std::vector<ClickData>> parse(const std::filesystem::path& path, MacroFormat format);
    static Result<> exportTo(const std::vector<ClickData>& clicks, const std::filesystem::path& path, MacroFormat format);

private:
    static Result<std::vector<ClickData>> parseCML(const std::string& content);
    static Result<std::vector<ClickData>> parseTTR3(const std::string& content);
    static Result<std::vector<ClickData>> parseGDR(const std::string& content);
    static Result<std::vector<ClickData>> parseGDR2(const std::string& content);
    static Result<std::vector<ClickData>> parseJSON(const std::string& content);
};