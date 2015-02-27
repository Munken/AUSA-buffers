#include "buf/LZ4OutputStream.h"

using namespace AUSA::protobuf;
using namespace kj;

LZ4OutputStream::LZ4OutputStream(OutputStream &inner, unsigned compressionLevel, size_t chunkSize) : BUFFER_SIZE(chunkSize), inner(inner) {
    LZ4F_frameInfo_t& fInfo = preferences.frameInfo;
    fInfo.contentChecksumFlag = contentChecksumEnabled;
    fInfo.blockSizeID = LZ4F_default;
    fInfo.blockMode = blockLinked;

    preferences.compressionLevel = compressionLevel;
    preferences.autoFlush = 1;

    OUTPUT_SIZE = LZ4F_compressFrameBound(BUFFER_SIZE, &preferences);

    writeBuffer = heapArray<byte>(BUFFER_SIZE);
    outputBuffer = heapArray<byte>(OUTPUT_SIZE);

    bufferPos = writeBuffer.begin();
}


void LZ4OutputStream::write(const void *src, size_t size) {
    if (src == bufferPos) {
        // Oh goody, the caller wrote directly into our buffer.
        bufferPos += size;
    }
    else { // MessageWriter using it's internal slow buffer.
        size_t available = writeBuffer.end() - bufferPos;
        if (size <= available) {
            memcpy(bufferPos, src, size);
            bufferPos += size;
        } else if (size <= writeBuffer.size()) {
            // Too much for this buffer, but not a full buffer's worth, so we'll go ahead and copy.
            memcpy(bufferPos, src, available);

            compressAndWrite();

            size -= available;
            src = reinterpret_cast<const byte*>(src) + available;
            memcpy(writeBuffer.begin(), src, size);
            bufferPos = writeBuffer.begin() + size;
        } else {
            // Fill current buffer and write out.
            memcpy(bufferPos, src, available);
            compressAndWrite();

            size-=available;
            src = reinterpret_cast<const byte*>(src) + available;

            // Write out big array in chunks of size BUFFER_SIZE
            for (size_t i = 0, N = size/writeBuffer.size(); i < N; i++) {
                compressAndWrite(src, BUFFER_SIZE);
                src = reinterpret_cast<const byte*>(src) + BUFFER_SIZE;
                size-=writeBuffer.size();
            }

            // Put the rest in the buffer.
            bufferPos = writeBuffer.begin();
            write(src, size);
        }
    }
}

kj::ArrayPtr<kj::byte> LZ4OutputStream::getWriteBuffer() {
    return arrayPtr(bufferPos, writeBuffer.end());
}

void LZ4OutputStream::flush() {
    if (bufferPos > writeBuffer.begin()) {
        compressAndWrite(writeBuffer.begin(), bufferPos - writeBuffer.begin());
        bufferPos = writeBuffer.begin();
    }
}

void LZ4OutputStream::compressAndWrite(const void *src, size_t size) {
    auto compressedSize = LZ4F_compressFrame(outputBuffer.begin(), OUTPUT_SIZE, src, size, &preferences);
    inner.write(outputBuffer.begin(), compressedSize);
}

void LZ4OutputStream::compressAndWrite() {
    compressAndWrite(writeBuffer.begin(), BUFFER_SIZE);
}

LZ4OutputStream::~LZ4OutputStream() {
    unwindDetector.catchExceptionsIfUnwinding([&]() {
        flush();
    });
}



