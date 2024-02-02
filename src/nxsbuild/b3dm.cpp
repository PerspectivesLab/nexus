#include "b3dm.h"


void b3dm::writeToStream(std::ostream &os) {

    os.write((char *) &_header.magic[0], sizeof(char) * 4);
    os.write((char *) &_header.version, sizeof(uint32_t));
    os.write((char *) &_header.byteLength, sizeof(uint32_t));
    os.write((char *) &_header.featureTableJSONByteLength, sizeof(uint32_t));
    os.write((char *) &_header.featureTableBinaryByteLength, sizeof(uint32_t));
    os.write((char *) &_header.batchTableJSONByteLength, sizeof(uint32_t));
    os.write((char *) &_header.batchTableBinaryByteLength, sizeof(uint32_t));
}
