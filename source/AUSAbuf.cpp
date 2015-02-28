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

void messageTest();

void messageComp();

using namespace AUSA::protobuf;
using namespace AUSA::Match;
using namespace std;
using namespace capnp;

int main() {
        bufTest();
//        lz4Test();
//        messageComp();
//        messageTest();
}

void messageComp() {
        int fd = open("message.lz4", O_RDWR|O_CREAT, 0664);

        kj::FdInputStream fdOutputStream(fd);
        LZ4InputStream bufferedStream(fdOutputStream);

        if (bufferedStream.tryGetReadBuffer() != nullptr) {
                ::capnp::PackedMessageReader message(bufferedStream);

                auto header = message.getRoot<Header>();

                auto singles = header.getSingles();
                cout << singles.size() << endl;
                SingleSided::Reader r = singles[0];
                auto name = r.getName();
                cout << name.cStr() << endl;
        }

        int c = 0;
        while (bufferedStream.tryGetReadBuffer() != nullptr) {
                cout << ++c << endl;
                ::capnp::PackedMessageReader message(bufferedStream);

                auto event = message.getRoot<PackedEvent>();

                auto mul = event.getMul();
                cout << "size" << mul.size() << endl;
                auto data = event.getData();

                for (int i = 0; i < mul[0]; i++) {
                        uint16_t time = (uint16_t) data[i].getTime();
                        double energy = data[i].getEnergy();
                        uint8_t segment = (uint8_t) data[i].getStrip();

//                        cout << "Write: (T, E, S) " << time << " " << energy << " " << to_string(segment) << endl;
                }
        }
}

void messageTest() {
        auto s = AUSA::JSON::readSetupFromJSON("setup/setup.json");
        int fd = open("message.lz4", O_RDWR|O_CREAT, 0664);

        kj::FdOutputStream fdOutputStream(fd);
        LZ4OutputStream lz(fdOutputStream);

        SetupOutput output{};
        unique_ptr<UInt_t> p;
        CalibratedSingleOutput singleOutput(s ->getSingleSided(0), 3);
        output.addSingleOutput(singleOutput);

        MallocMessageBuilder builder;
        buildHeader(builder, output);
        writePackedMessage(lz, builder);

        singleOutput.setMultiplicity(2);
        singleOutput.setSegment(0, 1);
        singleOutput.setSegment(1, 2);

        singleOutput.setTime(0, 100);
        singleOutput.setTime(1, 200);

        singleOutput.setEnergy(0, 10);
        singleOutput.setEnergy(1, 20);

        for (int i = 0; i < 100; i++) {
                MallocMessageBuilder builder2;
                buildEvent(builder2, output);
                writePackedMessage(lz, builder2);
        }

        lz.flush();
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
//        kj::FdInputStream fdInputStream(fd);
//        LZ4InputStream lz4(fdInputStream);
}

void bufTest() {
        auto s = AUSA::JSON::readSetupFromJSON("setup/setup.json");
        std::clock_t    start;

        start = std::clock();

        MatchReader reader("../Na23ap/matched/NaCla953m.root", s);
        reader.attach(make_shared<ProtoWriter>("testHC.buf"));
//        reader.attach(make_shared<SegmentSpectrumPlotter>(0, 5000));
        reader.run();

//        test("test.buf", s);
        double ms = (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000);
//        std::cout << "Time: " << ms << " ms" << std::endl;
        cout << ms << endl;
}