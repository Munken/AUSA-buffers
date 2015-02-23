#include "test.capn.h"
#include <capnp/message.h>
#include <capnp/serialize-packed.h>
#include <iostream>
#include <fcntl.h>

using namespace foo::bar::baz;

void writeAddressBook(int fd) {
    ::capnp::MallocMessageBuilder message;



    AddressBook::Builder addressBook = message.initRoot<AddressBook>();
    ::capnp::List<Person>::Builder people = addressBook.initPeople(2);

    Person::Builder alice = people[0];
    alice.setId(123);
    alice.setName("Alice");
    alice.setEmail("alice@example.com");
    // Type shown for explanation purposes; normally you'd use auto.
    ::capnp::List<Person::PhoneNumber>::Builder alicePhones =
            alice.initPhones(1);
    alicePhones[0].setNumber("555-1212");
    alicePhones[0].setType(Person::PhoneNumber::Type::MOBILE);
    alice.getEmployment().setSchool("MIT");

    Person::Builder bob = people[1];
    bob.setId(456);
    bob.setName("Bob");
    bob.setEmail("bob@example.com");
    auto bobPhones = bob.initPhones(2);
    bobPhones[0].setNumber("555-4567");
    bobPhones[0].setType(Person::PhoneNumber::Type::HOME);
    bobPhones[1].setNumber("555-7654");
    bobPhones[1].setType(Person::PhoneNumber::Type::WORK);
    bob.getEmployment().setUnemployed();

    writePackedMessageToFd(fd, message);
    writePackedMessageToFd(fd, message);
}

void writePerson(int fd) {
    ::capnp::MallocMessageBuilder message;
    Person::Builder person = message.initRoot<Person>();
    person.setId(2);
    person.setName("Hest");
    person.setEmail("Pest");

    auto phones = person.initPhones(3);
    phones[0].setNumber("1312");
    phones[0].setType(Person::PhoneNumber::Type::HOME);

    writePackedMessageToFd(fd, message);
}

void printPerson(Person::Reader& person) {
    std::cout << person.getName().cStr() << ": "
            << person.getEmail().cStr() << std::endl;
    for (Person::PhoneNumber::Reader phone: person.getPhones()) {
        const char* typeName = "UNKNOWN";
        switch (phone.getType()) {
            case Person::PhoneNumber::Type::MOBILE: typeName = "mobile"; break;
            case Person::PhoneNumber::Type::HOME: typeName = "home"; break;
            case Person::PhoneNumber::Type::WORK: typeName = "work"; break;
        }
        std::cout << "  " << typeName << " phone: "
                << phone.getNumber().cStr() << std::endl;
    }
    Person::Employment::Reader employment = person.getEmployment();
    switch (employment.which()) {
        case Person::Employment::UNEMPLOYED:
            std::cout << "  unemployed" << std::endl;
            break;
        case Person::Employment::EMPLOYER:
            std::cout << "  employer: "
                    << employment.getEmployer().cStr() << std::endl;
            break;
        case Person::Employment::SCHOOL:
            std::cout << "  student at: "
                    << employment.getSchool().cStr() << std::endl;
            break;
        case Person::Employment::SELF_EMPLOYED:
            std::cout << "  self-employed" << std::endl;
            break;
    }
}

void printAddressBook(int fd) {
    kj::FdInputStream fdStream(fd);
    kj::BufferedInputStreamWrapper bufferedStream(fdStream);

    if (bufferedStream.tryGetReadBuffer() != nullptr) {
        ::capnp::PackedMessageReader message(bufferedStream);

            auto person = message.getRoot<Person>();
            printPerson(person);
            std::cout << std::endl;
    }

    while (bufferedStream.tryGetReadBuffer() != nullptr) {
        ::capnp::PackedMessageReader message(bufferedStream);

        AddressBook::Reader addressBook = message.getRoot<AddressBook>();

        for (Person::Reader person : addressBook.getPeople()) {
            printPerson(person);
        }
    }
}

void printPerson(int fd) {
    ::capnp::PackedFdMessageReader message(fd);

    auto person = message.getRoot<Person>();
    printPerson(person);
}

int main() {
    int fd = open("myfile", O_RDWR|O_CREAT);
//    writePerson(fd);
//    writeAddressBook(fd);
    printAddressBook(fd);
}