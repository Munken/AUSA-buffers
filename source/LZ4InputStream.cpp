#include "buf/LZ4InputStream.h"
#include "buf/AUSALZ4.h"

#include <iostream>

using namespace AUSA::protobuf;
using namespace kj;
using namespace std;

namespace {
    unsigned readInt(byte* ptr, size_t offset) {
        return *((unsigned*) (ptr + offset));
    }

    size_t readSize(size_t compressedSize) {
        return compressedSize + FRAME_HEADER_SIZE;
    }
}

LZ4InputStream::LZ4InputStream(InputStream &inner) : inner(inner) {
    size_t initialHeaderSize = HEADER_SIZE + FRAME_HEADER_SIZE;
    auto tmpBuffer = heapArray<byte>(initialHeaderSize);
    auto out = inner.tryRead(tmpBuffer.begin(), initialHeaderSize, initialHeaderSize);

    if (out < HEADER_SIZE) {
        cerr << "Premature end of file !" << endl;
        throw;
    }

    auto bufferSize = readInt(tmpBuffer.begin(), BUFFER_OFFSET);
    auto decompressedSize = LZ4_COMPRESSBOUND(bufferSize) + FRAME_HEADER_SIZE;

    nextFrameSize = readInt(tmpBuffer.begin(), FIRST_FRAME_OFFSET+FRAME_SIZE_OFFSET);

    stream = LZ4_createStreamDecode();

    compressedBuffer = heapArray<byte>(bufferSize);
    decompressedBuffer = heapArray<byte>(decompressedSize);
}

size_t LZ4InputStream::readCompressed() {
    if (nextFrameSize == 0) return 0;

    auto compressedSize = inner.tryRead(compressedBuffer.begin(), nextFrameSize, readSize(nextFrameSize));
    auto decompressedSize = LZ4_decompress_safe_continue(stream, (char const *) compressedBuffer.begin(), (char *) decompressedBuffer.begin(), nextFrameSize, (int) decompressedBuffer.size());

    if (compressedSize == readSize(nextFrameSize)) {
        nextFrameSize = readInt(compressedBuffer.begin(), nextFrameSize);
    } else {
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
        size_t n = readCompressed();
        bufferAvailable = decompressedBuffer.slice(0, n);
    }
    return bufferAvailable;
}


LZ4InputStream::~LZ4InputStream() {
    if (stream != nullptr) LZ4_freeStreamDecode(stream);
}


