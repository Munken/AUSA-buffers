#include "buf/ProtoWriter.h"
#include "buf/EventBuilder.h"
#include "buf/HeaderBuilder.h"

#include <fcntl.h>
#include <capnp/serialize-packed.h>


using namespace std;

using namespace AUSA::Match;
using namespace AUSA::protobuf;
using namespace capnp;

ProtoWriter::ProtoWriter(std::string path, LZ4CompressionLevel compressionLevel, size_t chunkSize) :
        fd(open(path.c_str(), O_RDWR|O_CREAT|O_TRUNC, 0664)) {

    fdStream = new kj::FdOutputStream(fd);
    bufferedStream = new LZ4OutputStream(*fdStream, compressionLevel, chunkSize);
    buffer = kj::heapArray<word>(SUGGESTED_FIRST_SEGMENT_WORDS);

    // Fill buffer with 0's
    std::fill(buffer.asBytes().begin(), buffer.asBytes().end(), 0);

}

ProtoWriter::~ProtoWriter() {
    try {
        if (bufferedStream != nullptr) delete bufferedStream;
        if (fdStream != nullptr) delete fdStream;
        close(fd);
    }
    catch (...) {
        cerr << "Big trouble deleting ProtoWriter" << endl;
    }
}

void ProtoWriter::setup(const CalibratedSetupOutput &output) {
    AbstractCalibratedAnalyzer::setup(output);

    MallocMessageBuilder builder;
    buildHeader(builder, output);
//    writePackedMessage(*bufferedStream, builder);
    writeMessage(*bufferedStream, builder);
}

void ProtoWriter::terminate() {
    bufferedStream ->flush();
}

void ProtoWriter::analyze() {
    MallocMessageBuilder builder{buffer};
    buildEvent(builder, output);
    writeMessage(*bufferedStream, builder);
}



