#ifndef BUF_PROTO_WRITER_H
#define BUF_PROTO_WRITER_H

#include <match/analyzer/AbstractCalibratedAnalyzer.h>
#include <capnp/serialize-packed.h>
#include "LZ4OutputStream.h"


namespace AUSA {
    namespace protobuf {
        class
        ProtoWriter : public Match::AbstractCalibratedAnalyzer {
        public:
            ProtoWriter(std::string path);
            ~ProtoWriter();

            virtual void setup(Match::CalibratedSetupOutput const &output) override;

            virtual void terminate() override;

            virtual void analyze() override;

        private:
            int fd;

            kj::FdOutputStream* fdStream;
            LZ4OutputStream* bufferedStream;
        };
    }
}

#endif