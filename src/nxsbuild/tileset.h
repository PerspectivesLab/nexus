#ifndef TILESET_H
#define TILESET_H

#include "deps/json.hpp"
#include "nexusbuilder.h"
#include <string>
#include <vector>

struct BoundingVolume
{
    float box[12];

    nlohmann::json toJson() {
        nlohmann::json j =
            {"BoundingVolume",
                            {{"box", box}}};

        return j;
    }
};

struct Content
{
    BoundingVolume boundingVolume;
    std::string uri;
    nlohmann::json toJson() {
        nlohmann::json j;
        return j;
    }
};

struct Tile
{
    float transform[16];
    BoundingVolume boundingVolume;
    int geometricError;
    std::string refine;
    std::vector<Tile> children ;
    std::vector<int> childrenIndex;
};

struct Tileset
{
    std::string asset = "1.0";
    int geometricError = 500;
    Tile root;
};

class TilesetBuilder
{
public:
    TilesetBuilder() = delete;
    TilesetBuilder(NexusBuilder &nexus) : _nexusStructure(nexus) {};
    void build();
private:
    void makeTile(int tile_index);
    NexusBuilder& _nexusStructure;
    std::vector<Tile> _tiles;
    Tileset _tileset;
};





#endif // TILESET_H
