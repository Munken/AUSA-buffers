#include "buf/LZ4OutputStream.h"
#include "iostream"
using namespace std;

using namespace AUSA::protobuf;
using namespace kj;

namespace {
    void writeInt(byte* ptr, size_t number) {
        *((unsigned*) ptr) = static_cast<unsigned>(number);
    }
}

LZ4OutputStream::LZ4OutputStream(OutputStream &inner, unsigned compressionLevel, size_t chunkSize) : BUFFER_SIZE(chunkSize), inner(inner) {
    stream = LZ4_createStream();

    OUTPUT_SIZE = LZ4_COMPRESSBOUND(BUFFER_SIZE) + sizeof(unsigned);

    writeBuffer[0] = heapArray<byte>(BUFFER_SIZE);
    writeBuffer[1] = heapArray<byte>(BUFFER_SIZE);
    outputBuffer = heapArray<byte>(OUTPUT_SIZE);

    activeBuffer = writeBuffer[0];
    bufferPos = activeBuffer.begin();

    writeInt(bufferPos, BUFFER_SIZE+4);
    inner.write(bufferPos, sizeof(unsigned));
}


void LZ4OutputStream::write(const void *src, size_t size) {
    if (src == bufferPos) {
        // Oh goody, the caller wrote directly into our buffer.
        bufferPos += size;
    }
    else { // MessageWriter using it's internal slow buffer.
        size_t available = activeBuffer.end() - bufferPos;
        if (size <= available) {
            memcpy(bufferPos, src, size);
            bufferPos += size;
        } else if (size <= activeBuffer.size()) {
            // Too much for this buffer, but not a full buffer's worth, so we'll go ahead and copy.
            memcpy(bufferPos, src, available);

            compressAndWrite();

            size -= available;
            src = reinterpret_cast<const byte*>(src) + available;
            memcpy(activeBuffer.begin(), src, size);
            bufferPos = activeBuffer.begin() + size;
        } else {
            // Fill current buffer and write out.
            memcpy(bufferPos, src, available);
            compressAndWrite();

            size-=available;
            src = reinterpret_cast<const byte*>(src) + available;

            // Write out big array in chunks of size BUFFER_SIZE
            for (size_t i = 0, N = size/activeBuffer.size(); i < N; i++) {
                compressAndWrite(src, BUFFER_SIZE);
                src = reinterpret_cast<const byte*>(src) + BUFFER_SIZE;
                size-=activeBuffer.size();
            }

            // Put the rest in the buffer.
            bufferPos = activeBuffer.begin();
            write(src, size);
        }
    }
}

kj::ArrayPtr<kj::byte> LZ4OutputStream::getWriteBuffer() {
    return arrayPtr(bufferPos, activeBuffer.end());
}

void LZ4OutputStream::flush() {
    if (bufferPos > activeBuffer.begin()) {
        compressAndWrite(activeBuffer.begin(), bufferPos - activeBuffer.begin());
        bufferPos = activeBuffer.begin();
    }
}

void LZ4OutputStream::compressAndWrite(const void *src, size_t size) {
    auto compressedSize = LZ4_compress_continue(stream, static_cast<const char*>(src), (char*)(outputBuffer.begin() + sizeof(unsigned)), (int) size);
    writeInt(outputBuffer.begin(), compressedSize);
    inner.write(outputBuffer.begin(), compressedSize+sizeof(unsigned));

    if (activeBuffer == writeBuffer[0])
        activeBuffer = writeBuffer[1];
    else
        activeBuffer = writeBuffer[0];
}

void LZ4OutputStream::compressAndWrite() {
    compressAndWrite(activeBuffer.begin(), BUFFER_SIZE);
}

LZ4OutputStream::~LZ4OutputStream() {
    unwindDetector.catchExceptionsIfUnwinding([&]() {
        flush();
    });
}



