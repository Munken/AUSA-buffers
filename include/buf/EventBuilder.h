#ifndef BUF_EVENT_BUILDER_H
#define BUF_EVENT_BUILDER_H
#include <match/output.h>
#include <capnp/message.h>

namespace AUSA {
    namespace protobuf {
        void buildEvent(capnp::MessageBuilder& builder, const Match::SetupOutput& output);
    }
}

#endif