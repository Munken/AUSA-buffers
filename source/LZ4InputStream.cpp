#include <glob.h>
#include <kj/common.h>
#include <lz4frame.h>
#include "buf/LZ4InputStream.h"

#include <iostream>

using namespace AUSA::protobuf;
using namespace kj;
using namespace std;

LZ4InputStream::LZ4InputStream(InputStream &inner) : inner(inner) {
    LZ4F_decompressionContext_t* context = new LZ4F_decompressionContext_t();
    LZ4F_frameInfo_t* frame = new LZ4F_frameInfo_t();
    auto err = LZ4F_createDecompressionContext(context, LZ4F_VERSION);
    if (LZ4F_isError(err)) {
        cerr << "Failed to create decompression context due to: " << LZ4F_getErrorName(err) << endl;
        throw;
    }

    auto buffer = heapArray<byte>(8 << 10);
    size_t size;
    auto readRet = inner.read(buffer.begin(), 1, buffer.size());
    cout << readRet << endl;

    for (int i = 0; i < readRet; i++) cout << buffer[i];
    cout << endl;

    auto lzErr = LZ4F_getFrameInfo(context, frame, buffer.begin(), &size);

    cout << lzErr << endl;
    cout << LZ4F_getErrorName(lzErr) << endl;
    cout << size << endl;
}

size_t LZ4InputStream::tryRead(void *buffer, size_t minBytes, size_t maxBytes) {
    return 0;
}

kj::ArrayPtr<kj::byte const> LZ4InputStream::tryGetReadBuffer() {

}


