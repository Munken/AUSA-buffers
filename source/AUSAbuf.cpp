#include "buf/Header.capn.h"
#include "buf/Event.capn.h"
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


using namespace AUSA::protobuf;
using namespace AUSA::Match;
using namespace std;

void write(shared_ptr<AUSA::Setup> &s);
void read();

int main() {
        auto s = AUSA::JSON::readSetupFromJSON("setup/setup.json");
        std::clock_t    start;

        start = std::clock();

        MatchReader reader("../Na23ap/matched/NaCla953m.root", s);
//        reader.attach(make_shared<ProtoWriter>("test.buf"));
        reader.attach(make_shared<SegmentSpectrumPlotter>(0, 5000));
        reader.run();

//        test("test.buf", s);
        double ms = (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000);
//        std::cout << "Time: " << ms << " ms" << std::endl;
        cout << ms << endl;




//        write(s);
//        read();

}

void read() {
        int fd = open("test.buf", O_RDWR|O_CREAT, 0777);
        capnp::PackedFdMessageReader message(fd);

        auto evt = message.getRoot<PackedEvent>();

        auto mul = evt.getMul();
        auto data = evt.getData();

        cout << "Size " << mul.size() << endl;
        cout << "Data size " << data.size() << endl;
        auto m = mul[0];

        cout << "Mul" << to_string(int(m)) << endl;

        for (int i = 0; i < 2; i++) {
                auto E0 = data[i];
                cout << "Strip  " << to_string(int(E0.getStrip())) << endl;
                cout << "TDC    " << to_string(int(E0.getTime())) << endl;
                cout << "Energy " <<to_string(double(E0.getEnergy())) << endl;
        }
}

void write(shared_ptr<AUSA::Setup> &s) {
        int fd = open("test.buf", O_RDWR|O_CREAT, 0777);

        auto S3 = s ->getDSSD("S3");
        SetupOutput out;
        CalibratedOutput f(S3, 32);
        CalibratedOutput b(S3, 32);
        CalibratedDoubleOutput dOut(S3, f, b);
        out.addDssdOutput(dOut);

        f.setMultiplicity(1);
        b.setMultiplicity(1);

        f.setEnergy(0, 1000);
        b.setEnergy(0, 2000);

        f.setSegment(0, 1);
        b.setSegment(0, 3);

        f.setTime(0, 68);
        b.setTime(0, 70);

        capnp::MallocMessageBuilder builder;
        buildHeader(builder, out);
//        writePackedMessageToFd(fd, builder);

        capnp::MallocMessageBuilder builder2;
        buildEvent(builder2, out);

        writePackedMessageToFd(fd, builder2);

        close(fd);
}

