#include <iostream>
#include <fstream>
#include <fcntl.h>
#include "addressbook.pb.h"
#include "test.pb.h"
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>
#include <string>
#include <unistd.h>

using namespace std;
using namespace google::protobuf::io;

//// This function fills in a Person message based on user input.
//void PromptForAddress(tutorial::Person* person) {
//    cout << "Enter person ID number: ";
//    int id;
//    cin >> id;
//    person->set_id(id);
//    cin.ignore(256, '\n');
//
//    cout << "Enter name: ";
//    getline(cin, *person->mutable_name());
//
//    cout << "Enter email address (blank for none): ";
//    string email;
//    getline(cin, email);
//    if (!email.empty()) {
//        person->set_email(email);
//    }
//
//    while (true) {
//        cout << "Enter a phone number (or leave blank to finish): ";
//        string number;
//        getline(cin, number);
//        if (number.empty()) {
//            break;
//        }
//
//        tutorial::Person::PhoneNumber* phone_number = person->add_phone();
//        phone_number->set_number(number);
//
//        cout << "Is this a mobile, home, or work phone? ";
//        string type;
//        getline(cin, type);
//        if (type == "mobile") {
//            phone_number->set_type(tutorial::Person::MOBILE);
//        } else if (type == "home") {
//            phone_number->set_type(tutorial::Person::HOME);
//        } else if (type == "work") {
//            phone_number->set_type(tutorial::Person::WORK);
//        } else {
//            cout << "Unknown phone type.  Using default." << endl;
//        }
//    }
//}
//
//// Main function:  Reads the entire address book from a file,
////   adds one person based on user input, then writes it back out to the same
////   file.
//int main(int argc, char* argv[]) {
//    // Verify that the version of the library that we linked against is
//    // compatible with the version of the headers we compiled against.
//    GOOGLE_PROTOBUF_VERIFY_VERSION;
//
//    if (argc != 2) {
//        cerr << "Usage:  " << argv[0] << " ADDRESS_BOOK_FILE" << endl;
//        return -1;
//    }
//
//    tutorial::AddressBook address_book;
//
//    {
//        // Read the existing address book.
//        fstream input(argv[1], ios::in | ios::binary);
//        if (!input) {
//            cout << argv[1] << ": File not found.  Creating a new file." << endl;
//        } else if (!address_book.ParseFromIstream(&input)) {
//            cerr << "Failed to parse address book." << endl;
//            return -1;
//        }
//    }
//
//    // Add an address.
//    PromptForAddress(address_book.add_person());
//
//    {
//        // Write the new address book back to disk.
//        fstream output(argv[1], ios::out | ios::trunc | ios::binary);
//        if (!address_book.SerializeToOstream(&output)) {
//            cerr << "Failed to write address book." << endl;
//            return -1;
//        }
//        if (!address_book.SerializeToOstream(&output)) {
//            cerr << "Failed to write address book." << endl;
//            return -1;
//        }
//    }
//
//    // Optional:  Delete all global objects allocated by libprotobuf.
//    google::protobuf::ShutdownProtobufLibrary();
//
//    return 0;
//}

int main(int argc, char* argv[]) {
    // Verify that the version of the library that we linked against is
    // compatible with the version of the headers we compiled against.
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    A a;
    B b;
    B b2;

    a.set_a("A");
    b.set_t("b");
    b2.set_t("b2");
//    int fd = open("myfile", O_WRONLY|O_CREAT);
//    cout << fd << endl;
//    ZeroCopyOutputStream* raw_output = new FileOutputStream(fd);
//    CodedOutputStream* coded_output = new CodedOutputStream(raw_output);

    google::protobuf::uint32 magic_number = 1234;
//    const char* text =  "Hello world!";
//    coded_output->WriteLittleEndian32(magic_number);
//    coded_output->WriteVarint32(strlen(text));
//    coded_output->WriteRaw((void const *) text, strlen(text));
//
//    delete coded_output;
//    delete raw_output;
//    close(fd);

    int fd = open("myfile", O_RDONLY);
    ZeroCopyInputStream* raw_input = new FileInputStream(fd);
    CodedInputStream* coded_input = new CodedInputStream(raw_input);

    coded_input->ReadLittleEndian32(&magic_number);
    if (magic_number != 1234) {
        cerr << "File not in expected format." << endl;
        return 1;
    }



    google::protobuf::uint32 size;
    coded_input->ReadVarint32(&size);

    char* text = new char[size + 1];
    coded_input->ReadRaw(text, size);
    text[size] = '\0';

        delete coded_input;
        delete raw_input;
        close(fd);

        cout << "Text is: " << text << endl;
//
//    fstream input(argv[1], ios::in | ios::binary);
//    if (!a.ParseFromIstream(&input)) {
//        cerr << "Failed to parse a" << endl;
//        return -1;
//    }
//    cout << a.a() << endl;
//    if (!b2.ParseFromIstream(&input)) {
//        cerr << "Failed to parse b2." << endl;
//        return -1;
//    }
//    cout << b2.t() << endl;
//
//
//    if (!b.ParseFromIstream(&input)) {
//        cerr << "Failed to parse b." << endl;
//        return -1;
//    }




    cout << a.a() << endl;
    cout << b.t() << endl;

    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}
