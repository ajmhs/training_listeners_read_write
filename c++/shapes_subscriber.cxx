/*
* (c) Copyright, Real-Time Innovations, 2020.  All rights reserved.
* RTI grants Licensee a license to use, modify, compile, and create derivative
* works of the software solely for use with RTI Connext DDS. Licensee may
* redistribute copies of the software provided that all such copies are subject
* to this license. The software is provided "as is", with no warranty of any
* type, including any warranty for fitness for any purpose. RTI is under no
* obligation to maintain or support the software. RTI shall not be liable for
* any incidental or consequential damages arising out of the use or inability
* to use the software.
*/

#include <algorithm>
#include <iostream>

#include <dds/sub/ddssub.hpp>
#include <dds/core/ddscore.hpp>
#include <rti/config/Logger.hpp>  // for logging
// alternatively, to include all the standard APIs:
//  <dds/dds.hpp>
// or to include both the standard APIs and extensions:
//  <rti/rti.hpp>
//
// For more information about the headers and namespaces, see:
//    https://community.rti.com/static/documentation/connext-dds/7.2.0/doc/api/connext_dds/api_cpp2/group__DDSNamespaceModule.html
// For information on how to use extensions, see:
//    https://community.rti.com/static/documentation/connext-dds/7.2.0/doc/api/connext_dds/api_cpp2/group__DDSCpp2Conventions.html

#include "shapes.hpp"
#include "application.hpp"  // for command line parsing and ctrl-c
#include <rti/util/util.hpp> // for sleep

using std::cout;
using std::endl;
using dds::core::Duration;

static unsigned int samples_read = 0;

// https://community.rti.com/static/documentation/connext-dds/7.2.0/doc/api/connext_dds/api_cpp2/classListener.html
// CAUTION: Listeners are invoked by internal threads... 

class DRListener : public dds::sub::NoOpDataReaderListener<ShapeTypeExtended> {

    // This is the NoOp - where anything not overridden does nothing as a convenience (cleaner code)

    // There are other overrides available, but I don't need to use them here.
    // I could use them for logging events if I needed to know

    virtual void on_data_available(dds::sub::DataReader<ShapeTypeExtended> &reader) {

        dds::sub::LoanedSamples< ::ShapeTypeExtended> samples = reader.take();
        for (auto sample : samples) {
            if (sample.info().valid()) {
                ++samples_read;
                cout << sample.data() << endl;
            } 
            else { // Metadata received, do we want to do anything here?
                
            }
        }
    }

    virtual void on_subscription_matched(
        dds::sub::DataReader<ShapeTypeExtended> &writer,
        const dds::core::status::SubscriptionMatchedStatus &subscription_state) {

        // -1 for a subscription lost, else gained one
        cout << "Inside on_subscription_matched: " << 
            (subscription_state.current_count_change() < 0 ? "lost" : "found") <<
            " a publisher" << endl; 
    }
};

void run_subscriber_application(unsigned int domain_id, unsigned int sample_count)
{
    // DDS objects behave like shared pointers or value types
    // (see https://community.rti.com/best-practices/use-modern-c-types-correctly)

    // Start communicating in a domain, usually one participant per application
    dds::domain::DomainParticipant participant(domain_id);

    // Create a Topic with a name and a datatype
    dds::topic::Topic< ::ShapeTypeExtended> topic(participant, "Oblong");

    // Create a Subscriber and DataReader with default Qos
    dds::sub::Subscriber subscriber(participant);
    dds::sub::DataReader< ::ShapeTypeExtended> reader(subscriber, topic);

    auto listener = std::make_shared<DRListener>();
    reader.set_listener(listener);
    
    while (!application::shutdown_requested && samples_read < sample_count) {
        std::cout << "::ShapeTypeExtended subscriber sleeping up to 5 sec..." << std::endl;

            rti::util::sleep(Duration(5));
    }
}

int main(int argc, char *argv[])
{

    using namespace application;

    // Parse arguments and handle control-C
    auto arguments = parse_arguments(argc, argv);
    if (arguments.parse_result == ParseReturn::exit) {
        return EXIT_SUCCESS;
    } else if (arguments.parse_result == ParseReturn::failure) {
        return EXIT_FAILURE;
    }
    setup_signal_handlers();

    // Sets Connext verbosity to help debugging
    rti::config::Logger::instance().verbosity(arguments.verbosity);

    try {
        run_subscriber_application(arguments.domain_id, arguments.sample_count);
    } catch (const std::exception& ex) {
        // This will catch DDS exceptions
        std::cerr << "Exception in run_subscriber_application(): " << ex.what()
        << std::endl;
        return EXIT_FAILURE;
    }

    // Releases the memory used by the participant factory.  Optional at
    // application exit
    dds::domain::DomainParticipant::finalize_participant_factory();

    return EXIT_SUCCESS;
}
