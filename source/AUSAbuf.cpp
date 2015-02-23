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

void writeDummy(int fd);

void print(int fd);

using namespace AUSA::protobuf;
using namespace AUSA::Match;
using namespace std;

int main() {
        auto s = AUSA::JSON::readSetupFromJSON("setup/setup.json");
        MatchReader reader("matched/NaCla926_0m.root", s);
        reader.attach(make_shared<ProtoWriter>("test.buf"));

        reader.run();
}

