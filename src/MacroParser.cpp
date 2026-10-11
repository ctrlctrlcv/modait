#include "MacroParser.hpp"
#include <algorithm>
#include <cctype>
#include <cstring>
#include <regex>
#include <sstream>

using namespace geode::prelude;

namespace fbfc {

static std::string lowerExt(std::filesystem::path const& p) {
    auto e = p.extension().string();
    std::ranges::transform(e, e.begin(), [](unsigned char c) { return std::tolower(c); });
    return e;
}

static std::string asText(std::vector<uint8_t> const& b) { return {b.begin(), b.end()}; }

MacroFormat MacroParser::detect(std::filesystem::path const& p, std::vector<uint8_t> const& b) {
    // 1) Firma binaria
    if (b.size() >= 4 && std::memcmp(b.data(), "TTR3", 4) == 0) return MacroFormat::TTR3;
    // 2) Primer carácter útil
    for (auto c : b) {
        if (std::isspace(c)) continue;
        if (c == '{' || c == '[') return MacroFormat::GDR2;
        if (c == '<') return MacroFormat::CML;
        break;
    }
    // 3) Extensión
    auto e = lowerExt(p);
    if (e == ".gdr") return MacroFormat::GDR;
    if (e == ".json" || e == ".gdr2") return MacroFormat::GDR2;
    if (e == ".cml") return MacroFormat::CML;
    if (e == ".ttr3") return MacroFormat::TTR3;
    return b.empty() ? MacroFormat::Unknown : MacroFormat::GDR;
}

Result<MacroData> MacroParser::load(std::filesystem::path const& p) {
    auto r = file::readBinary(p);
    if (!r) return Err("Cannot read '{}': {}", p.string(), r.unwrapErr());
    auto bytes = std::move(r).unwrap();
    Result<MacroData> out = Err("Unknown macro format");
    switch (detect(p, bytes)) {
        case MacroFormat::GDR: out = parseGDR(asText(bytes)); break;
        case MacroFormat::GDR2: out = parseJSON(asText(bytes)); break;
        case MacroFormat::CML: out = parseCML(asText(bytes)); break;
        case MacroFormat::TTR3: out = parseTTR3(bytes); break;
        default: return Err("Unrecognized macro format for '{}'", p.filename().string());
    }
    if (out) {
        auto m = std::move(out).unwrap();
        std::ranges::stable_sort(m.events, {}, &ClickEvent::frame);
        if (m.name.empty()) m.name = p.stem().string();
        return Ok(std::move(m));
    }
    return out;
}

Result<> MacroParser::save(MacroData const& m, std::filesystem::path const& p) {
    auto e = lowerExt(p);
    if (e == ".gdr") return file::writeString(p, toGDR(m));
    if (e == ".json" || e == ".gdr2") return file::writeString(p, toJSON(m));
    if (e == ".cml") return file::writeString(p, toCML(m));
    if (e == ".ttr3") return file::writeBinary(p, toTTR3(m));
    return Err("Unsupported export extension '{}'", e);
}

// ---------- GDR (CSV simplificado) ----------
// fps,240 / frame,down,player
Result<MacroData> MacroParser::parseGDR(std::string const& s) {
    MacroData m;
    std::istringstream in(s);
    std::string line;
    size_t ln = 0;
    while (std::getline(in, line)) {
        ++ln;
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty() || line[0] == '#' || line.starts_with("frame")) continue;
        if (line.starts_with("fps")) {
            auto comma = line.find(',');
            if (comma == std::string::npos) return Err("GDR line {}: bad fps header", ln);
            m.fps = std::strtod(line.c_str() + comma + 1, nullptr);
            continue;
        }
        char* end = nullptr;
        long long f = std::strtoll(line.c_str(), &end, 10);
        if (end == line.c_str() || *end != ',') return Err("GDR line {}: expected frame,down[,player]", ln);
        int d = static_cast<int>(std::strtol(end + 1, &end, 10));
        int pl = (*end == ',') ? static_cast<int>(std::strtol(end + 1, nullptr, 10)) : 1;
        ClickEvent e; e.frame = f; e.down = d != 0; e.player2 = pl == 2;
        m.events.push_back(e);
    }
    if (m.fps <= 0) return Err("GDR: invalid fps");
    return Ok(std::move(m));
}

std::string MacroParser::toGDR(MacroData const& m) {
    std::string s = "# FBFrameCounter GDR\nfps," + fmt::format("{}", m.fps) + "\nframe,down,player\n";
    for (auto& e : m.events) s += fmt::format("{},{},{}\n", e.frame, e.down ? 1 : 0, e.player2 ? 2 : 1);
    return s;
}

// ---------- JSON / GDR2 (matjson) ----------
Result<MacroData> MacroParser::parseJSON(std::string const& s) {
    auto parsed = matjson::parse(s);
    if (!parsed) return Err("Invalid JSON: {}", parsed.unwrapErr());
    auto root = std::move(parsed).unwrap();
    MacroData m;
    m.fps = root["fps"].asDouble().unwrapOr(240.0);
    m.name = root["name"].asString().unwrapOr("");
    // Acepta {"inputs":[...]} o un array raíz
    matjson::Value const& list = root.isArray() ? root : root["inputs"];
    auto arr = list.asArray();
    if (!arr) return Err("JSON macro: missing 'inputs' array");
    for (auto const& v : arr.unwrap()) {
        ClickEvent e;
        e.frame = v["frame"].asInt().unwrapOr(0);
        e.down = v["down"].asBool().unwrapOr(true);
        e.player2 = v["2p"].asBool().unwrapOr(v["p2"].asBool().unwrapOr(false));
        m.events.push_back(e);
    }
    return Ok(std::move(m));
}

std::string MacroParser::toJSON(MacroData const& m) {
    matjson::Value root = matjson::Value::object();
    root["format"] = "gdr2";
    root["name"] = m.name;
    root["fps"] = m.fps;
    matjson::Value arr = matjson::Value::array();
    for (auto& e : m.events) {
        matjson::Value o = matjson::Value::object();
        o["frame"] = e.frame;
        o["down"] = e.down;
        o["2p"] = e.player2;
        arr.push(o);
    }
    root["inputs"] = arr;
    return root.dump(2);
}

// ---------- CML (XML con regex) ----------
Result<MacroData> MacroParser::parseCML(std::string const& s) {
    MacroData m;
    std::smatch fm;
    if (std::regex_search(s, fm, std::regex(R"(<macro[^>]*fps\s*=\s*\"([\d.]+)\")", std::regex::icase)) ||
        std::regex_search(s, fm, std::regex(R"(<fps>\s*([\d.]+)\s*</fps>)", std::regex::icase)))
        m.fps = std::strtod(fm[1].str().c_str(), nullptr);
    std::regex tag(R"(<input\s+([^>]*?)/?>)", std::regex::icase);
    std::regex attr(R"((\w+)\s*=\s*\"([^\"]*)\")");
    for (auto it = std::sregex_iterator(s.begin(), s.end(), tag); it != std::sregex_iterator(); ++it) {
        std::string body = (*it)[1].str();
        ClickEvent e; bool haveFrame = false;
        for (auto a = std::sregex_iterator(body.begin(), body.end(), attr); a != std::sregex_iterator(); ++a) {
            std::string k = (*a)[1].str(), v = (*a)[2].str();
            if (k == "frame") { e.frame = std::strtoll(v.c_str(), nullptr, 10); haveFrame = true; }
            else if (k == "down") e.down = (v == "1" || v == "true");
            else if (k == "p2") e.player2 = (v == "1" || v == "true");
        }
        if (haveFrame) m.events.push_back(e);
    }
    if (m.events.empty()) return Err("CML: no <input .../> entries found");
    return Ok(std::move(m));
}

std::string MacroParser::toCML(MacroData const& m) {
    std::string s = fmt::format("<?xml version=\"1.0\"?>\n<macro fps=\"{}\">\n", m.fps);
    for (auto& e : m.events)
        s += fmt::format("  <input frame=\"{}\" down=\"{}\" p2=\"{}\"/>\n", e.frame, e.down ? 1 : 0, e.player2 ? 1 : 0);
    return s + "</macro>\n";
}

// ---------- TTR3 (binario) ----------
// "TTR3" | u16 version=1 | f32 fps | u32 count | count * { u32 frame, u8 flags(bit0 down, bit1 p2) }
static constexpr size_t kTTR3Header = 14, kTTR3Entry = 5;

Result<MacroData> MacroParser::parseTTR3(std::vector<uint8_t> const& b) {
    if (b.size() < kTTR3Header || std::memcmp(b.data(), "TTR3", 4) != 0) return Err("TTR3: bad magic");
    uint16_t ver; float fps; uint32_t count;
    std::memcpy(&ver, &b[4], 2); std::memcpy(&fps, &b[6], 4); std::memcpy(&count, &b[10], 4);
    if (ver != 1) return Err("TTR3: unsupported version {}", ver);
    if (!(fps > 0.f)) return Err("TTR3: invalid fps");
    if ((b.size() - kTTR3Header) / kTTR3Entry != count || (b.size() - kTTR3Header) % kTTR3Entry != 0)
        return Err("TTR3: truncated or corrupt payload");
    MacroData m; m.fps = fps; m.events.reserve(count);
    for (uint32_t i = 0; i < count; ++i) {
        uint32_t f; std::memcpy(&f, &b[kTTR3Header + i * kTTR3Entry], 4);
        uint8_t fl = b[kTTR3Header + i * kTTR3Entry + 4];
        ClickEvent e; e.frame = f; e.down = fl & 1; e.player2 = fl & 2;
        m.events.push_back(e);
    }
    return Ok(std::move(m));
}

std::vector<uint8_t> MacroParser::toTTR3(MacroData const& m) {
    std::vector<uint8_t> b(kTTR3Header + m.events.size() * kTTR3Entry);
    std::memcpy(b.data(), "TTR3", 4);
    uint16_t ver = 1; float fps = static_cast<float>(m.fps); uint32_t count = static_cast<uint32_t>(m.events.size());
    std::memcpy(&b[4], &ver, 2); std::memcpy(&b[6], &fps, 4); std::memcpy(&b[10], &count, 4);
    for (size_t i = 0; i < m.events.size(); ++i) {
        uint32_t f = static_cast<uint32_t>(m.events[i].frame);
        std::memcpy(&b[kTTR3Header + i * kTTR3Entry], &f, 4);
        b[kTTR3Header + i * kTTR3Entry + 4] = (m.events[i].down ? 1 : 0) | (m.events[i].player2 ? 2 : 0);
    }
    return b;
}

}  // namespace fbfc
