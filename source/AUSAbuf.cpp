#include "buf/Header.capn.h"
#include "match/MatchReader.h"
#include "json/IO.h"
#include <capnp/message.h>
#include <capnp/serialize-packed.h>
#include <iostream>
#include <fcntl.h>
#include <buf/ProtoWriter.h>

#include "buf/HeaderBuilder.h"
#include "match/output.h"

#include "buf/ProtoReader.h"

#include <ctime>
#include <buf/EventBuilder.h>
#include <buf/PackedEvent.capn.h>
#include <match/analyzer/SegmentSpectrumPlotter.h>

#include <lz4frame.h>
#include <lz4.h>
#include <buf/LZ4OutputStream.h>
#include <buf/LZ4InputStream.h>

void bufTest();

void lz4Test();

using namespace AUSA::protobuf;
using namespace AUSA::Match;
using namespace std;

int main() {
        bufTest();
//        lz4Test();
}

void lz4Test() {
        int fd = open("test.lz4", O_RDWR|O_CREAT, 0664);

//        string test = "HestPestPestKostMestMegaPestHest";
//        char* buffer = new char[test.length()];
//        memcpy(buffer, test.c_str(), test.size());
//
//        kj::FdOutputStream fdOutputStream(fd);
//        LZ4OutputStream lz4OutputStream(fdOutputStream, 0, 16);
//        lz4OutputStream.write(buffer, 16);
//        lz4OutputStream.write(buffer+8, 16);
//
//        lz4OutputStream.flush();
//
//        close(fd);
        kj::FdInputStream fdInputStream(fd);
        LZ4InputStream lz4(fdInputStream);


}

void bufTest() {
        auto s = AUSA::JSON::readSetupFromJSON("setup/setup.json");
        std::clock_t    start;

        start = std::clock();

        MatchReader reader("../Na23ap/matched/NaCla926_0m.root", s);
        reader.attach(make_shared<ProtoWriter>("test.buf"));
//        reader.attach(make_shared<SegmentSpectrumPlotter>(0, 5000));
        reader.run();

//        test("test.buf", s);
        double ms = (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000);
//        std::cout << "Time: " << ms << " ms" << std::endl;
        cout << ms << endl;
}