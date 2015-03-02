#include "buf/LZ4InputStream.h"

#include <iostream>
#include <xxhash.h>
#include <lz4.h>
#include <AUSA.h>

using namespace AUSA::protobuf;
using namespace AUSA::protobuf::LZ4;
using namespace kj;
using namespace std;

namespace {
    uint32_t readInt(byte* ptr, size_t offset) {
        return *((uint32_t*) (ptr + offset));
    }

    uint64_t readHash(byte* ptr, size_t offset) {
        return *((uint64_t*) (ptr + offset));
    }

    size_t readSize(size_t compressedSize) {
        return compressedSize + FRAME_HEADER_SIZE;
    }
}

class LZ4InputStream::StreamState {
public:
    LZ4_streamDecode_t* stream;
    LZ4::BufferSize_t nextFrameSize;
    LZ4::Hash_t lastHash;

    StreamState() {
        stream = LZ4_createStreamDecode();
    }

    ~StreamState() {
        if (stream != nullptr) LZ4_freeStreamDecode(stream);
    }
};

LZ4InputStream::LZ4InputStream(InputStream &inner) : inner(inner) {
    size_t initialHeaderSize = HEADER_SIZE + FRAME_HEADER_SIZE;
    auto tmpBuffer = heapArray<byte>(initialHeaderSize);
    auto out = inner.tryRead(tmpBuffer.begin(), initialHeaderSize, initialHeaderSize);

    if (out < HEADER_SIZE) {
        cerr << "Premature end of file !" << endl;
        throw;
    }

    uint32_t magic = readInt(tmpBuffer.begin(), MAGIC_WORD_OFFSET);
    if (magic != MAGIC_WORD) {
        cerr << "First 32 bits are not " << MAGIC_WORD << "." << endl
        << "Somthing is very wrong !" << endl;
        throw;
    }

    auto bufferSize = readInt(tmpBuffer.begin(), BUFFER_OFFSET);
    auto decompressedSize = LZ4_COMPRESSBOUND(bufferSize) + FRAME_HEADER_SIZE;

    state = make_unique<StreamState>();
    state-> nextFrameSize = readInt(tmpBuffer.begin(), FIRST_FRAME_OFFSET+FRAME_SIZE_OFFSET);
    state-> lastHash = readHash(tmpBuffer.begin(), FIRST_FRAME_OFFSET+FRAME_HASH_OFFSET);

    compressedBuffer = heapArray<byte>(bufferSize);
    decompressedBuffer = heapArray<byte>(decompressedSize);
}

size_t LZ4InputStream::readCompressed() {
    if (state-> nextFrameSize == 0) return 0;

    auto compressedSize = inner.tryRead(compressedBuffer.begin(), state-> nextFrameSize, readSize(state-> nextFrameSize));
    auto decompressedSize = LZ4_decompress_safe_continue(state-> stream, (char const *) compressedBuffer.begin(), (char *) decompressedBuffer.begin(), state-> nextFrameSize, (int) decompressedBuffer.size());

//    auto hash = XXH64(decompressedBuffer.begin(), decompressedSize, 0);

//    if (hash != state-> lastHash) {
//        cerr << "Hash value of chunk don't correspond to recorded value." << endl
//             << "This is most likely due to data corruption!" << endl;
//        throw;
//    }

    state-> lastHash = readHash(compressedBuffer.begin(), state-> nextFrameSize + FRAME_HASH_OFFSET);
    if (compressedSize == readSize(state-> nextFrameSize)) {
        state-> nextFrameSize = readInt(compressedBuffer.begin(), state-> nextFrameSize);
    } else {
        state-> nextFrameSize = 0;
    }

    return (size_t) decompressedSize;
}


size_t LZ4InputStream::tryRead(void *dst, size_t minBytes, size_t maxBytes) {
    if (minBytes <= bufferAvailable.size()) {
        // Serve from current buffer.
        size_t n = std::min(bufferAvailable.size(), maxBytes);
        memcpy(dst, bufferAvailable.begin(), n);
        bufferAvailable = bufferAvailable.slice(n, bufferAvailable.size());
        return n;
    } else {
        // Copy current available into destination.
        memcpy(dst, bufferAvailable.begin(), bufferAvailable.size());
        size_t fromFirstBuffer = bufferAvailable.size();
        dst = reinterpret_cast<byte*>(dst) + fromFirstBuffer;
        minBytes -= fromFirstBuffer;
        maxBytes -= fromFirstBuffer;
        if (maxBytes <= decompressedBuffer.size()) {
            // Read the next buffer-full.
            size_t n = readCompressed();

            size_t fromSecondBuffer = std::min(n, maxBytes);
            memcpy(dst, decompressedBuffer.begin(), fromSecondBuffer);
            bufferAvailable = decompressedBuffer.slice(fromSecondBuffer, n);
            return fromFirstBuffer + fromSecondBuffer;
        } else {
            // Forward large read to the underlying stream.
            cerr << "Currently unsupported!" << endl;
            exit(42);
//            bufferAvailable = nullptr;
//            return fromFirstBuffer + inner.read(dst, minBytes, maxBytes);
        }
    }
}

kj::ArrayPtr<kj::byte const> LZ4InputStream::tryGetReadBuffer() {
    if (bufferAvailable.size() == 0) {
        size_t n = readCompressed();
        bufferAvailable = decompressedBuffer.slice(0, n);
    }
    return bufferAvailable;
}


LZ4InputStream::~LZ4InputStream() {

}
