#include "buf/Header.capn.h"
#include "match/MatchReader.h"
#include "json/IO.h"

#include "buf/ProtoReader.h"

#include <match/analyzer/SegmentSpectrumPlotter.h>

void bufTest();


using namespace AUSA::protobuf;
using namespace AUSA::Match;
using namespace std;
using namespace capnp;


int main() {
        bufTest();
}

void bufTest() {
        auto s = AUSA::JSON::readSetupFromJSON("setup/setup.json");
//        std::clock_t    start;
//
//        start = std::clock();

//        MatchReader reader("../Na23ap/matched/NaCla926_0m.root", s);
//        reader.attach(make_shared<ProtoWriter>("test_926.buf", LZ4CompressionLevel::FAST, 20 << 20));
//////        reader.attach(make_shared<SegmentSpectrumPlotter>(0, 5000));
//        reader.run();
////        return;

//        test("test_953_float.buf", s);
        ProtoReader reader("test_926.buf", s);
        reader.attach(make_shared<SegmentSpectrumPlotter>(0,5000));
        reader.run();
        reader.saveToRootFile("test.root");
////        test("test_953_NP.buf", s);
//        int N = 8;
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

//        double ms = (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000);
//        std::cout << "Time: " << ms << " ms" << std::endl;
//        cout << ms << endl;
}