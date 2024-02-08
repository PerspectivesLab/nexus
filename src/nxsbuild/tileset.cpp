#include "tileset.h"
#include <QTextStream>
#include <fstream>



std::array<float, 12> convertBoundingBox (const vcg::Point3f &X, const vcg::Point3f &Y, const vcg::Point3f &Z,
                                          const vcg::Point3f &min, const vcg::Point3f &max)
{
    vcg::Point3f centre = (min + max) * 0.5;
    float halfLengthX = (max.X() - min.X())/2;
    vcg::Point3f halfAxeX = X * halfLengthX;
    float halfLengthY = (max.Y() - min.Y())/2;
    vcg::Point3f halfAxeY = Y * halfLengthY;
    float halfLengthZ = (max.Z() - min.Z())/2;
    vcg::Point3f halfAxeZ = Z * halfLengthZ;

    std::array<float, 12> bbox = { centre.X(),    centre.Y(),   centre.Z(),
                                   halfAxeX.X(),  halfAxeX.Y(), halfAxeX.Z(),
                                   halfAxeY.X(),  halfAxeY.Y(), halfAxeY.Z(),
                                   halfAxeZ.X(),  halfAxeZ.Y(), halfAxeZ.Z()
    };

    return bbox;
}

void TilesetBuilder::build() {
    for(int i = 0; i < _nexusStructure.nodes.size() - 1; i++) {
        makeTile(i);
    }

    fillTileset();

    Tileset tileset;
    tileset.root = _tiles[0];
    write(tileset.toJson(), "output/tileset.json");
}

void TilesetBuilder::makeTile(int tileIndex) {

    Tile tile;

    // 1. Extract bounding volume and convert it in 3D tiles boundingVolume specification.
    auto axes = _nexusStructure.boxes[tileIndex].axes;
    auto box = _nexusStructure.boxes[tileIndex].box;
    auto bbox = convertBoundingBox(axes[0], axes[1], axes[2], box.min, box.max);
    tile.boundingVolume.box = bbox;

    // 2. GeometricError
    nx::Node &node = _nexusStructure.nodes[tileIndex];
    tile.geometricError = node.error;
    // are they the same values ??

    // 3. Uri
    tile.content.uri = std::to_string(tileIndex) + ".b3dm";

    // 4. Extract children;

    for(int i = node.first_patch; i < node.last_patch(); i++) {
        const nx::Patch &patch = _nexusStructure.patches[i];
        if(!(patch.node == _nexusStructure.nodes.size()-1)){
            tile.childrenIndex.push_back(patch.node);
        }
    }

    _tiles.push_back(tile);
}

void TilesetBuilder::fillTileset() {

    for( int i = _tiles.size() - 1; i >= 0; i-- ) {
        for(int child_index : _tiles[i].childrenIndex) {
            _tiles[i].children.push_back(_tiles[child_index]);
        }
    }
}

nlohmann::json  TilesetBuilder::getTile(int index) {
    return _tiles[index].toJson();
}

void TilesetBuilder::write(const nlohmann::json &json, const QString &path) {

    std::ofstream file(path.toStdString());
    file << json;
}

void TilesetBuilder::writeMinimalTileset(int node_index) {
    Tile simpleTile;

    auto axes = _nexusStructure.boxes[node_index].axes;
    auto box = _nexusStructure.boxes[node_index].box;
    simpleTile.boundingVolume.box = convertBoundingBox(axes[0], axes[1], axes[2], box.min, box.max);
    nx::Node &node = _nexusStructure.nodes[node_index];
    simpleTile.geometricError = node.error;

    simpleTile.content.uri = std::to_string(node_index) + ".b3dm";

    Tileset tileset;
    tileset.root = simpleTile;

    write(tileset.toJson(), "output/tileset2.json");
}
