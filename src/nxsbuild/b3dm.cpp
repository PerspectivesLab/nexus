#include "b3dm.h"


b3dm::b3dm(int binaryByteLength) {

    nlohmann::json j;
    j["BATCH_LENGTH"] = 0;
    _featureTableHeader = j.dump();
    _header.featureTableJSONByteLength = std::streamsize(j.dump().size());

    int byteOffset = _header.size() + _header.featureTableJSONByteLength;
    int paddingSize = byteOffset % 8;
    if (paddingSize != 0) {
        paddingSize = 8 - paddingSize;
        _padding = std::string(size_t(paddingSize), ' ');
        _featureTableHeader.append(_padding);
        _header.featureTableJSONByteLength = std::streamsize(_featureTableHeader.size());
    }

    int endPaddingSize = (_header.size() + _header.featureTableJSONByteLength + binaryByteLength) % 8;
    if(endPaddingSize !=0) {
        endPaddingSize = 8 - endPaddingSize;
        _endPadding = std::string(size_t(endPaddingSize), ' ');
    }

     _header.byteLength = _header.size() + _header.featureTableJSONByteLength + binaryByteLength + endPaddingSize;
    verify();
}

void b3dm::writeToStream(std::ostream &os) {

    os.write((char *) &_header.magic[0], sizeof(char) * 4);
    os.write((char *) &_header.version, sizeof(uint32_t));
    os.write((char *) &_header.byteLength, sizeof(uint32_t));
    os.write((char *) &_header.featureTableJSONByteLength, sizeof(uint32_t));
    os.write((char *) &_header.featureTableBinaryByteLength, sizeof(uint32_t));
    os.write((char *) &_header.batchTableJSONByteLength, sizeof(uint32_t));
    os.write((char *) &_header.batchTableBinaryByteLength, sizeof(uint32_t));
    os.write(_featureTableHeader.c_str(), _header.featureTableJSONByteLength);
    //os.write(_padding.c_str(), std::streamsize(_padding.size()));
}

bool b3dm::verify() {

    // A tile's byteLength must be aligned to an 8-byte boundary.
    assert(_header.byteLength % 8 == 0);

    // The binary glTF (b3dm and i3dm only) (if present) must start and end on an 8-byte alignment
    assert((_header.size() + _featureTableHeader.size()) % 8 == 0);
    return true;
};
