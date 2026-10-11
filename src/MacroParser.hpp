#pragma once
#include "Types.hpp"
#include <filesystem>

namespace fbfc {

enum class MacroFormat { Unknown, GDR, GDR2, CML, TTR3 };

class MacroParser {
public:
    static MacroFormat detect(std::filesystem::path const& p, std::vector<uint8_t> const& bytes);
    static geode::Result<MacroData> load(std::filesystem::path const& p);
    static geode::Result<> save(MacroData const& m, std::filesystem::path const& p);

    static geode::Result<MacroData> parseGDR(std::string const& s);
    static geode::Result<MacroData> parseJSON(std::string const& s);
    static geode::Result<MacroData> parseCML(std::string const& s);
    static geode::Result<MacroData> parseTTR3(std::vector<uint8_t> const& b);

    static std::string toGDR(MacroData const& m);
    static std::string toJSON(MacroData const& m);
    static std::string toCML(MacroData const& m);
    static std::vector<uint8_t> toTTR3(MacroData const& m);
};

}  // namespace fbfc
