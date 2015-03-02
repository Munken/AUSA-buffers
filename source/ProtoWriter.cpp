#include "buf/ProtoWriter.h"
#include "buf/EventBuilder.h"
#include "buf/HeaderBuilder.h"

#include <fcntl.h>
#include <capnp/serialize-packed.h>


#include <iostream>
using namespace std;

using namespace AUSA::Match;
using namespace AUSA::protobuf;
using namespace capnp;

ProtoWriter::ProtoWriter(std::string path, LZ4CompressionLevel compressionLevel, size_t chunkSize) :
        fd(open(path.c_str(), O_RDWR|O_CREAT, 0664)) {

    fdStream = new kj::FdOutputStream(fd);
    bufferedStream = new LZ4OutputStream(*fdStream, compressionLevel, chunkSize);
    buffer = kj::heapArray<word>(20);

    // Nasty hack to zero output array.
    std::fill(reinterpret_cast<uint64_t*>(buffer.begin()), reinterpret_cast<uint64_t*>(buffer.end()), 0);
}

ProtoWriter::~ProtoWriter() {

}

void ProtoWriter::setup(const CalibratedSetupOutput &output) {
    AbstractCalibratedAnalyzer::setup(output);

    MallocMessageBuilder builder;
    buildHeader(builder, output);
//    writePackedMessage(*bufferedStream, builder);
    writeMessage(*bufferedStream, builder);
}


void ProtoWriter::terminate() {
    try {
        bufferedStream ->flush();
        if (bufferedStream != nullptr) delete bufferedStream;
        if (fdStream != nullptr) delete fdStream;
        close(fd);
    }
    catch (...) {
        cerr << "Big trouble deleting ProtoWriter" << endl;
    }
}

void ProtoWriter::analyze() {
    MallocMessageBuilder builder{buffer};
    buildEvent(builder, output);
//    auto p = builder.getSegmentsForOutput();
    // Size of message can be determined from p[0].size() if message have only one segment.
//    writePackedMessage(*bufferedStream, builder);
    writeMessage(*bufferedStream, builder);
}



