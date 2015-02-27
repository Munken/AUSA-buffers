#include <glob.h>
#include <kj/common.h>
#include <kj/array.h>
#include <lz4frame.h>
#include "buf/LZ4OutputStream.h"

#include <iostream>
#include <G__ci_fproto.h>

using namespace std;

using namespace AUSA::protobuf;
using namespace kj;

LZ4OutputStream::LZ4OutputStream(OutputStream &inner) : BUFFER_SIZE(8 << 20), inner(inner) {
    LZ4F_frameInfo_t& fInfo = preferences.frameInfo;
    fInfo.contentChecksumFlag = contentChecksumEnabled;
    fInfo.blockSizeID = LZ4F_default;
    fInfo.blockMode = blockLinked;

    preferences.compressionLevel = 0;
    preferences.autoFlush = 1;

    OUTPUT_SIZE = LZ4F_compressFrameBound(BUFFER_SIZE, &preferences);

    writeBuffer = heapArray<byte>(BUFFER_SIZE);
    outputBuffer = heapArray<byte>(OUTPUT_SIZE);

//    context = new LZ4F_compressionContext_t;
//    auto err = LZ4F_createCompressionContext(context, LZ4F_VERSION);


//    auto oSize = LZ4F_compressBegin(context, outputBuffer.begin(), BUFFER_SIZE, &preferences);

    bufferPos = writeBuffer.begin();
}

int bufEqSrc = 0, sizeAvail = 0, sizeLessThan = 0, sizeToBig = 0;


void LZ4OutputStream::write(const void *src, size_t size) {
    if (src == bufferPos) {
        // Oh goody, the caller wrote directly into our buffer.
        bufferPos += size;
        bufEqSrc++;
    }
    else { // MessageWriter using it's internal slow buffer.
        size_t available = writeBuffer.end() - bufferPos;
        if (size <= available) {
            memcpy(bufferPos, src, size);
            bufferPos += size;
            sizeAvail++;
        } else if (size <= writeBuffer.size()) {
            // Too much for this buffer, but not a full buffer's worth, so we'll go ahead and copy.
            memcpy(bufferPos, src, available);

            auto compressedSize = LZ4F_compressFrame(outputBuffer.begin(), OUTPUT_SIZE, writeBuffer.begin(), BUFFER_SIZE, &preferences);

            inner.write(outputBuffer.begin(), compressedSize);
            size -= available;
            src = reinterpret_cast<const byte*>(src) + available;
            memcpy(writeBuffer.begin(), src, size);
            bufferPos = writeBuffer.begin() + size;
        } else {
            cout << size / writeBuffer.size() << endl;
            cerr << "Not supported for LZ4 buffer" << endl;
            exit(42);
            // Writing so much data that we might as well write directly to avoid a copy.
            /*inner.write(writeBuffer.begin(), bufferPos - writeBuffer.begin());
            bufferPos = writeBuffer.begin();
            inner.write(src, size);
            sizeToBig++;*/
        }
    }
}

kj::ArrayPtr<kj::byte> LZ4OutputStream::getWriteBuffer() {
    return arrayPtr(bufferPos, writeBuffer.end());
}

int flushCount = 0;

void LZ4OutputStream::flush() {
    flushCount++;
    bufferPos = writeBuffer.begin();
}

LZ4OutputStream::~LZ4OutputStream() {
    flush();
    cout << bufEqSrc << "\t" << sizeAvail << "\t" << sizeLessThan << "\t" << sizeToBig << "\t" << flushCount << endl;
}
