#ifndef BUF_EVENT_BUILDER_H
#define BUF_EVENT_BUILDER_H
#include <ausa/match/output.h>
#include <capnp/message.h>

namespace AUSA {
    namespace protobuf {

        /**
        * Given a event. Construct a PackedEvent in the MessageBuilder.
        */
        void buildEvent(capnp::MessageBuilder& builder, const Match::SetupOutput& output);
    }
}

#endif