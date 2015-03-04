#ifndef BUF_PROTO_READER_H
#define BUF_PROTO_READER_H

#include <setup/Setup.h>
#include <string>
#include <match/analyzer/AbstractCalibratedProvider.h>
#include <kj/io.h>

namespace AUSA {
    namespace protobuf {
        class ProtoReader : public Match::AbstractCalibratedProvider {
        public:
            ProtoReader(std::string path, std::shared_ptr<Setup> setup);

//            void add(std::string path);

            void run();

            /**
            * Save the output from all the attached analyzers to a ROOT file.
            * @param filename Path of the file.
            * @param options Option string to pass to the <a href="http://root.cern.ch/root/html/TFile.html">TFile</a> constructor.
            */
            void saveToRootFile(std::string filename, std::string options = "RECREATE");

        private:
            std::string path;
            const int dCount, sCount, sigCount;
            Match::SetupOutput output;
        };
    }
}

#endif
