#include <glob.h>
#include <kj/common.h>
#include <lz4frame.h>
#include "buf/LZ4InputStream.h"

#include <iostream>

using namespace AUSA::protobuf;
using namespace kj;
using namespace std;

namespace {
    unsigned readInt(byte* ptr) {
        return *((unsigned*) ptr);
    }

    auto UNSIGNED_SIZE = 4U;
    auto HEADER_SIZE = 2U*UNSIGNED_SIZE;
    auto FRAME_HEADER_SIZE = 1U*UNSIGNED_SIZE;

    size_t readSize(size_t compressedSize) {
        return compressedSize + FRAME_HEADER_SIZE;
    }

}

LZ4InputStream::LZ4InputStream(InputStream &inner) : inner(inner) {
    auto tmpBuffer = heapArray<byte>(HEADER_SIZE);
    auto out = inner.tryRead(tmpBuffer.begin(), HEADER_SIZE, HEADER_SIZE);

    if (out < HEADER_SIZE) {
        cerr << "Premature end of file !" << endl;
        throw;
    }

    auto bufferSize = readInt(tmpBuffer.begin()) + FRAME_HEADER_SIZE;
    auto decompressedSize = LZ4_COMPRESSBOUND(bufferSize);

    nextFrameSize = readInt(tmpBuffer.begin() + FRAME_HEADER_SIZE);

    stream = LZ4_createStreamDecode();

    compressedBuffer = heapArray<byte>(bufferSize);
    decompressedBuffer = heapArray<byte>(decompressedSize);
}

size_t LZ4InputStream::readCompressed() {
    if (nextFrameSize == 0) return 0;

    auto compressedSize = inner.tryRead(compressedBuffer.begin(), nextFrameSize, readSize(nextFrameSize));
    auto decompressedSize = LZ4_decompress_safe_continue(stream, (char const *) compressedBuffer.begin(), (char *) decompressedBuffer.begin(), nextFrameSize, (int) decompressedBuffer.size());

    if (compressedSize == readSize(nextFrameSize)) {
        nextFrameSize = readInt(compressedBuffer.begin() + nextFrameSize);
    }
    else {
        nextFrameSize = 0;
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
//        size_t n = inner.tryRead(decompressedBuffer.begin(), 1, decompressedBuffer.size());
        size_t n = readCompressed();
        bufferAvailable = decompressedBuffer.slice(0, n);
    }
    return bufferAvailable;
}


LZ4InputStream::~LZ4InputStream() {
    if (stream != nullptr) LZ4_freeStreamDecode(stream);
}


