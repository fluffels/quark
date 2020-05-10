#include "FileSystem.h"
#include "PAKParser.h"

#define HEADER_LENGTH 4
#define FILE_ENTRY_LENGTH 64

void PAKParser::parseHeader() {
    readStruct(file, header);
    if (strncmp("PACK", header.id, HEADER_LENGTH) != 0) {
        throw runtime_error("this is not a PAK file");
    }
}

void PAKParser::parseEntries() {
    auto count = header.size / FILE_ENTRY_LENGTH;
    LOG(INFO) << "contains " << count << " entries";
    entries.resize(count);

    seek(file, header.offset);
    size_t read = fread_s(
        entries.data(),
        sizeof(PAKFileEntry) * count,
        sizeof(PAKFileEntry),
        count,
        file
    );
    if (read != count) {
        throw runtime_error("unexpected EOF while reading PAK entries");
    }
}

PAKParser::PAKParser(const char* path):
        file(NULL),
        map(nullptr) {
    errno_t error = fopen_s(&file, path, "rb");
    if (error != 0) {
        throw runtime_error("could not open PAK file");
    }

    parseHeader();
    parseEntries();
}

PAKParser::~PAKParser() {
    fclose(file);
    delete map;
}

BSPParser* PAKParser::loadMap(const string& name) {
    string entryName = "maps/" + name + ".bsp";
    for (auto entry: entries) {
        if (strcmp(entryName.c_str(), entry.name) == 0) {
            return new BSPParser(file, entry.offset);
        }
    }
    throw std::runtime_error("could not find map " + entryName);
}
