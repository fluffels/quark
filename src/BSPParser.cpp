#include <cmath>

#include "BSPParser.h"
#include "FileSystem.h"

using std::runtime_error;

void fixCoords(vec3& v) {
    /* NOTE(jan): Translate from BSP coordinate system.
       See: http://www.gamers.org/dEngine/quake/spec/quake-spec34/qkspec_2.htm#2.1.1 */
    auto y = -v.z;
    auto z = -v.y;
    v.y = y;
    v.z = z;
}

void parseOrigin(char *buffer, vec3 &origin) {
    char *s = strstr(buffer, " ");
    *s = '\0';
    origin.x = (float)atoi(buffer);

    char *n = s + 1;
    s = strstr(n, " ");
    *s = '\0';
    origin.z = (float)-atoi(n);

    n = s + 1;
    origin.y = (float)-atoi(n);
}

void BSPParser::parseHeader() {
    seek(file, fileOffset);
    readStruct(file, header);
    if (header.version != 29) {
        throw runtime_error("BSP is not version 29");
    }
}

void BSPParser::parseFaces() {
    auto offset = fileOffset + header.faces.offset;
    auto size = header.faces.size;

    auto count = size / sizeof(Face);
    faces.resize(count);

    seek(file, offset);
    size_t readCount = fread_s(faces.data(), size, sizeof(Face), count, file);
    if (readCount != count) {
        throw runtime_error("unexpected eof");
    }
}

void BSPParser::parseEntities() {
    auto offset = fileOffset + header.entities.offset;
    auto size = header.entities.size;

    seek(file, offset);
    char* entityBuffer = new char[size];
    fread_s(entityBuffer, size, size, 1, file);
    char* ePos = entityBuffer;

    char buffer[255];
    char* bPos = buffer;

    enum KEY {
        CLASS_NAME,
        ORIGIN,
        ANGLE,
        UNKNOWN
    };
    KEY key = UNKNOWN;

    enum STATE {
        OUTSIDE_ENTITY,
        INSIDE_ENTITY,
        INSIDE_STRING,
    };
    STATE state = OUTSIDE_ENTITY;
    Entity entity;

    while(ePos < entityBuffer + size) {
        char c = *ePos;
        switch (state) {
            case OUTSIDE_ENTITY:
                if (c == '{') {
                    state = INSIDE_ENTITY;
                    entity = {};
                }
                ePos++;
                break;
            case INSIDE_ENTITY:
                if (c == '"') {
                    bPos = buffer;
                    state = INSIDE_STRING;
                } else if (*ePos == '}') {
                    state = OUTSIDE_ENTITY;
                    entities.push_back(entity);
                }
                ePos++;
                break;
            case INSIDE_STRING:
                if (*ePos == '"') {
                    state = INSIDE_ENTITY;
                    *bPos = '\0';
                    switch (key) {
                        case CLASS_NAME:
                            strncpy_s(entity.className, buffer, 255);
                            break;
                        case ORIGIN:
                            parseOrigin(buffer, entity.origin);
                            break;
                        case ANGLE:
                            entity.angle = atoi(buffer);
                            break;
                        default:
                            break;
                    }
                    if (strcmp("classname", buffer) == 0) key = CLASS_NAME;
                    else if (strcmp("origin", buffer) == 0) key = ORIGIN;
                    else if (strcmp("angle", buffer) == 0) key = ANGLE;
                    else key = UNKNOWN;
                } else {
                    *bPos = c;
                    bPos++;
                }
                ePos++;
                break;
        }
    }
}

void BSPParser::parseModels() {
    auto offset = fileOffset + header.models.offset;
    auto size = header.models.size;
    auto elementSize = sizeof(Model);

    const auto count = size / elementSize;
    models.resize(count);

    seek(file, offset);
    auto readCount = fread_s(
        models.data(),
        count * elementSize,
        elementSize,
        count,
        file
    );
    if (readCount != count) {
        throw runtime_error("unexpected EOF");
    }
}

void BSPParser::parseLightMap() {
    auto offset = fileOffset + header.lightmaps.offset;
    auto size = header.lightmaps.size;
    auto elementSize = sizeof(uint8_t);

    const auto count = size / elementSize;
    lightMap.resize(count);

    seek(file, offset);
    auto readCount = fread_s(
        lightMap.data(),
        count * elementSize,
        elementSize,
        count,
        file
    );
    if (readCount != count) {
        throw runtime_error("unexpected EOF");
    }
}

void BSPParser::parsePlanes() {
    auto offset = fileOffset + header.planes.offset;
    auto size = header.planes.size;
    auto elementSize = sizeof(Plane);

    const auto count = size / elementSize;
    planes.resize(count);

    seek(file, offset);
    auto readCount = fread_s(
        planes.data(),
        count * elementSize,
        elementSize,
        count,
        file
    );
    if (readCount != count) {
        throw runtime_error("unexpected EOF");
    }
}

void BSPParser::parseTexInfos() {
    auto offset = fileOffset + header.texinfo.offset;
    auto size = header.texinfo.size;
    auto elementSize = sizeof(TexInfo);

    auto count = size / elementSize;
    texInfos.resize(count);

    seek(file, offset);
    auto readCount = fread_s(
        texInfos.data(),
        size,
        elementSize,
        count,
        file
    );
    if (readCount != count) {
        throw runtime_error("unexpected EOF");
    }

    /* NOTE(jan): Translate from BSP coordinate system.
       See: http://www.gamers.org/dEngine/quake/spec/quake-spec34/qkspec_2.htm#2.1.1 */
    for (auto& texInfo: texInfos) {
        fixCoords(texInfo.uVector);
        fixCoords(texInfo.vVector);
    }
}

void BSPParser::parseVertices() {
    auto offset = fileOffset + header.vertices.offset;
    auto size = header.vertices.size;

    const int count = size / sizeof(vec3);
    vertices.resize(count);

    seek(file, offset);
    int32_t bytes = sizeof(vec3) * count;
    fread_s(vertices.data(), bytes, bytes, 1, file);

    for (vec3& vertex: vertices) {
        fixCoords(vertex);
    }
}

void BSPParser::parseEdgeList() {
    auto offset = fileOffset + header.ledges.offset;
    auto size = header.ledges.size;
    auto elementSize = (int32_t)sizeof(int32_t);

    auto count = size / elementSize;
    edgeList.resize(count);

    seek(file, offset);
    auto readCount = fread_s(edgeList.data(), size, elementSize, count, file);
    if (readCount != count) {
        throw runtime_error("unexpected EOF");
    }
}

void BSPParser::parseEdges() {
    auto offset = fileOffset + header.edges.offset;
    auto size = header.edges.size;

    const int count = size / sizeof(Edge);
    edges.resize(count);

    seek(file, offset);
    int32_t bytes = sizeof(Edge) * count;
    fread_s(edges.data(), bytes, bytes, 1, file);
}

Entity& BSPParser::findEntityByName(char* name) {
    for (Entity& entity: entities) {
        if (strcmp(name, entity.className) == 0) {
            return entity;
        }
    }
    throw runtime_error("could not find entity " + string(name));
}

BSPParser::BSPParser(FILE* file, int32_t offset, Palette& palette):
        atlas(nullptr),
        file(file),
        fileOffset(offset)
{
    parseHeader();

    atlas = new Atlas(file, fileOffset + header.miptex.offset, palette);

    parseModels();
    parseEntities();
    parseVertices();
    parseEdgeList();
    parseEdges();
    parseFaces();
    parseLightMap();
    parsePlanes();
    parseTexInfos();
}

BSPParser::~BSPParser() {
    delete atlas;
}
