#include "MacroParser.hpp"
#include <fstream>
#include <sstream>
#include <cstring>
#include <regex>
#include <algorithm>

// ============================================================
//  DETECCIÓN DE FORMATO
// ============================================================

MacroFormat MacroParser::detectFormat(const std::filesystem::path& path) {
    std::string ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if (ext == ".cml")  return MacroFormat::CML;
    if (ext == ".ttr3") return MacroFormat::TTR3;
    if (ext == ".gdr")  return MacroFormat::GDR;
    if (ext == ".gdr2") return MacroFormat::GDR2;

    if (ext == ".json") {
        std::string stem = path.stem().string();
        if (stem.find(".gdr") != std::string::npos)
            return MacroFormat::GDR_JSON;
        return MacroFormat::JSON;
    }

    return MacroFormat::UNKNOWN;
}

// ============================================================
//  PARSEO GENÉRICO
// ============================================================

Result<std::vector<ClickData>> MacroParser::parse(
    const std::filesystem::path& path,
    MacroFormat format) {

    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return Err("No se pudo abrir el archivo: {}", path.string());
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    switch (format) {
        case MacroFormat::CML:      return parseCML(content);
        case MacroFormat::TTR3:     return parseTTR3(content);
        case MacroFormat::GDR:      return parseGDR(content);
        case MacroFormat::GDR2:     return parseGDR2(content);
        case MacroFormat::JSON:
        case MacroFormat::GDR_JSON: return parseJSON(content);
        default:
            return Err("Formato no soportado");
    }
}

// ============================================================
//  PARSER CML (XML con regex)
// ============================================================

Result<std::vector<ClickData>> MacroParser::parseCML(const std::string& content) {
    std::vector<ClickData> clicks;

    std::regex clickRegex(R"(<click\s+([^>]+)/>)");
    std::regex attrRegex(R"rx((\w+)="([^"]*)")rx");

    auto begin = std::sregex_iterator(content.begin(), content.end(), clickRegex);
    auto end = std::sregex_iterator();

    for (auto it = begin; it != end; ++it) {
        std::string attrs = (*it)[1].str();
        ClickData click;

        auto attrBegin = std::sregex_iterator(attrs.begin(), attrs.end(), attrRegex);
        for (auto a = attrBegin; a != std::sregex_iterator(); ++a) {
            std::string key = (*a)[1].str();
            std::string val = (*a)[2].str();

            if (key == "frame")  click.frame = std::stoi(val);
            if (key == "player") click.player2 = (val == "2");
            if (key == "hold")   click.isHold = (val == "true" || val == "1");
        }

        click.time = static_cast<float>(click.frame) / 240.0f;
        clicks.push_back(click);
    }

    return Ok(clicks);
}

// ============================================================
//  PARSER TTR3 (binario)
// ============================================================

Result<std::vector<ClickData>> MacroParser::parseTTR3(const std::string& content) {
    std::vector<ClickData> clicks;

    if (content.size() < 12) {
        return Err("TTR3 demasiado corto");
    }

    if (content.substr(0, 4) != "TTR3") {
        return Err("Magic number inválido");
    }

    uint32_t version;
    std::memcpy(&version, content.data() + 4, 4);

    if (version != 3) {
        return Err("Versión TTR3 no soportada: {}", version);
    }

    size_t offset = 12;
    while (offset + 16 <= content.size()) {
        ClickData click;

        int32_t frame;
        std::memcpy(&frame, content.data() + offset, 4);
        click.frame = frame;

        float time;
        std::memcpy(&time, content.data() + offset + 4, 4);
        click.time = time;

        uint8_t type = content[offset + 8];
        click.isHold = (type == 0);

        uint8_t player = content[offset + 9];
        click.player2 = (player == 2);

        clicks.push_back(click);
        offset += 16;
    }

    return Ok(clicks);
}

// ============================================================
//  PARSER GDR (CSV: "frame,hold,player")
// ============================================================

Result<std::vector<ClickData>> MacroParser::parseGDR(const std::string& content) {
    std::vector<ClickData> clicks;
    std::istringstream stream(content);
    std::string line;

    while (std::getline(stream, line)) {
        if (line.empty()) continue;

        std::vector<std::string> parts;
        std::stringstream ss(line);
        std::string part;
        while (std::getline(ss, part, ',')) {
            parts.push_back(part);
        }

        if (parts.size() < 2) continue;

        ClickData click;
        try {
            click.frame = std::stoi(parts[0]);
        } catch (...) {
            continue;
        }
        click.isHold = (parts[1] == "1" || parts[1] == "true");
        click.player2 = (parts.size() > 2 && (parts[2] == "2" || parts[2] == "true"));
        click.time = static_cast<float>(click.frame) / 240.0f;

        clicks.push_back(click);
    }

    return Ok(clicks);
}

// ============================================================
//  PARSER GDR2 (JSON extendido)
// ============================================================

Result<std::vector<ClickData>> MacroParser::parseGDR2(const std::string& content) {
    std::vector<ClickData> clicks;

    auto parseResult = matjson::parse(content);
    if (parseResult.isErr()) {
        return parseGDR(content);
    }

    auto json = parseResult.unwrap();

    if (!json.isObject()) {
        return parseGDR(content);
    }

    if (!json.contains("clicks")) {
        return parseGDR(content);
    }

    for (auto& c : json["clicks"]) {
        ClickData click;
        click.frame = static_cast<int>(c["frame"].asInt().unwrapOr(0));
        click.time = c.contains("time")
            ? static_cast<float>(c["time"].asDouble().unwrapOr(click.frame / 240.0))
            : (click.frame / 240.0f);
        click.isHold = c.contains("hold") && c["hold"].asBool().unwrapOr(false);
        click.player2 = c.contains("player") && c["player"].asInt().unwrapOr(1) == 2;
        clicks.push_back(click);
    }

    return Ok(clicks);
}

// ============================================================
//  PARSER JSON (genérico)
// ============================================================

Result<std::vector<ClickData>> MacroParser::parseJSON(const std::string& content) {
    std::vector<ClickData> clicks;

    auto parseResult = matjson::parse(content);
    if (parseResult.isErr()) {
        return Err("JSON inválido");
    }

    auto json = parseResult.unwrap();

    bool found = false;
    auto processArray = [&](matjson::Value& arr) {
        if (!arr.isArray()) return;
        found = true;
        for (auto& item : arr) {
            ClickData click;
            if (item.isObject()) {
                click.frame = static_cast<int>(item["frame"].asInt().unwrapOr(0));
                click.time = item.contains("time")
                    ? static_cast<float>(item["time"].asDouble().unwrapOr(click.frame / 240.0))
                    : (click.frame / 240.0f);
                click.isHold = item.contains("hold") && item["hold"].asBool().unwrapOr(false);
                click.player2 = item.contains("player") && item["player"].asInt().unwrapOr(1) == 2;
            } else if (item.isNumber()) {
                click.frame = static_cast<int>(item.asInt().unwrapOr(0));
                click.time = click.frame / 240.0f;
            }
            clicks.push_back(click);
        }
    };

    if (json.isArray()) {
        processArray(json);
    } else if (json.isObject()) {
        if (json.contains("inputs"))       processArray(json["inputs"]);
        else if (json.contains("clicks"))  processArray(json["clicks"]);
        else if (json.contains("frames"))  processArray(json["frames"]);
    }

    if (!found) {
        return Err("No se encontró array de inputs en el JSON");
    }

    return Ok(clicks);
}

// ============================================================
//  EXPORTACIÓN
// ============================================================

Result<> MacroParser::exportTo(
    const std::vector<ClickData>& clicks,
    const std::filesystem::path& path,
    MacroFormat format) {

    std::ofstream file(path);
    if (!file.is_open()) return Err("No se pudo crear el archivo");

    switch (format) {
        case MacroFormat::CML: {
            file << "<macro>\n";
            for (const auto& c : clicks) {
                file << "  <click frame=\"" << c.frame
                     << "\" player=\"" << (c.player2 ? 2 : 1)
                     << "\" hold=\"" << (c.isHold ? "true" : "false") << "\"/>\n";
            }
            file << "</macro>\n";
            break;
        }
        case MacroFormat::GDR: {
            for (const auto& c : clicks) {
                file << c.frame << "," << (c.isHold ? 1 : 0)
                     << "," << (c.player2 ? 2 : 1) << "\n";
            }
            break;
        }
        case MacroFormat::JSON:
        case MacroFormat::GDR_JSON: {
            matjson::Value root = matjson::Value::object();
            root["version"] = 1;
            matjson::Value arr = matjson::Value::array();
            for (const auto& c : clicks) {
                matjson::Value item = matjson::Value::object();
                item["frame"] = c.frame;
                item["time"] = c.time;
                item["hold"] = c.isHold;
                item["player"] = c.player2 ? 2 : 1;
                arr.push(item);
            }
            root["inputs"] = arr;
            file << root.dump(2);
            break;
        }
        default:
            return Err("Exportación no soportada para este formato");
    }

    return Ok();
}
