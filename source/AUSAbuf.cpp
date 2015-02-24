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


using namespace AUSA::protobuf;
using namespace AUSA::Match;
using namespace std;

int main() {
        auto s = AUSA::JSON::readSetupFromJSON("setup/setup.json");
        std::clock_t    start;

        start = std::clock();

        MatchReader reader("matched/NaCla926_0m.root", s);
//        reader.attach(make_shared<ProtoWriter>("test.buf"));
        reader.run();



//        test("test.buf", s);
        std::cout << "Time: " << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000) << " ms" << std::endl;
}

