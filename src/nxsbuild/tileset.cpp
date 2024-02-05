#include "tileset.h"
#include <QTextStream>
#include <fstream>

void TilesetBuilder::build() {
    for(int i = 0; i < _nexusStructure.nodes.size() - 1; i++) {
        makeTile(i);
    }

    _tileset.root = _tiles[0];
    nlohmann::json test = _tileset.root.boundingVolume.toJson();
    std::string test2 = test.dump();

    write(test);
}

void TilesetBuilder::makeTile(int tileIndex) {

    Tile tile;

    // 1. Extract bounding volume and convert it in 3D tiles boundingVolume specification.
    auto box = _nexusStructure.boxes[tileIndex].box;

    uint32_t index2 = _nexusStructure.nodes[tileIndex].last_patch();
    // 2. Extract children;
    nx::Node &node = _nexusStructure.nodes[tileIndex];

    // il y a quelques chose que l'on ne comprends pas sur le fonctionnement des patchs.
    // pourquoi node.last_patch() renvoie genre 127 ???
    // -- piste à suivre : --> fonction savetileset à comparer avec save() de nexusBuilder

    uint32_t index = node.last_patch();
    for(int i = node.first_patch; i < node.last_patch(); i++) {
        const nx::Patch &patch = _nexusStructure.patches[i];
        if(!(patch.node == _nexusStructure.nodes.size()-1)){
            tile.childrenIndex.push_back(patch.node);
        }
    }

    _tiles.push_back(tile);
}

void TilesetBuilder::write(const nlohmann::json &json) {

    std::ofstream file("output/tileset.json");
    file << json;
}
