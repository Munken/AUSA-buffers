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
#include <thread>

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
}

void bufTest() {
        auto s = AUSA::JSON::readSetupFromJSON("setup/setup.json");
        std::clock_t    start;

        start = std::clock();

//        MatchReader reader("../Na23ap/matched/NaCla953m.root", s);
//        reader.attach(make_shared<ProtoWriter>("test_953_F.buf", LZ4CompressionLevel::FAST, 50 << 20));
////        reader.attach(make_shared<SegmentSpectrumPlotter>(0, 5000));
//        reader.run();
//
        test("test_953_F.buf", s);
//        test("test_926_HC.buf", s);
//        test("test_953_NP.buf", s);
//        int N = 4;
//////        for (int i = 0; i < N; i++)
////                test("test_953_" + to_string(0)+".buf", s);
//
//        vector<thread> t;
////
//        //Launch a group of threads
//        for (int i = 0; i < N-1; ++i) {
//                t.push_back(thread(test, "test_953_F_" + to_string(i) + ".buf", s));
//        }
//        test("test_953_F_" + to_string(N-1) +".buf", s);
//
//        //Join the threads with the main thread
//        for (int i = 0; i < t.size(); ++i) {
//                t[i].join();
//        }

        double ms = (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000);
//        std::cout << "Time: " << ms << " ms" << std::endl;
        cout << ms << endl;
}