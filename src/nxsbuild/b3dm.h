#ifndef B3DM_H
#define B3DM_H

#include <stdint.h>
#include <ostream>

struct b3dmHeader {
    char magic[4] = {'b', '3', 'd', 'm'};
    uint32_t version = 1;
    uint32_t byteLength;
    uint32_t featureTableJSONByteLength = 0;
    uint32_t featureTableBinaryByteLength = 0;
    uint32_t batchTableJSONByteLength = 0;
    uint32_t batchTableBinaryByteLength = 0;

    inline uint32_t size() { return ( sizeof(char) * 4 + 6 * sizeof(uint32_t) ); }
};

class b3dm
{
public:
    b3dm() = delete;
    b3dm(uint32_t binaryByteLength) { _header.byteLength = _header.size() + binaryByteLength; };
    void writeToStream(std::ostream &os);
private:
    b3dmHeader _header;
};
#endif // B3DM_H
