#include "BPSK_base.h"

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

    The following class functions are for the base class for the component class. To
    customize any of these functions, do not modify them here. Instead, overload them
    on the child class

******************************************************************************************/

BPSK_base::BPSK_base(const char *uuid, const char *label) :
    Resource_impl(uuid, label),
    ThreadedComponent()
{
    loadProperties();

    dataFloat_in = new bulkio::InFloatPort("dataFloat_in");
    addPort("dataFloat_in", dataFloat_in);
    clockFloat_in = new bulkio::InFloatPort("clockFloat_in");
    addPort("clockFloat_in", clockFloat_in);
    dataShort_out = new bulkio::OutShortPort("dataShort_out");
    addPort("dataShort_out", dataShort_out);
}

BPSK_base::~BPSK_base()
{
    delete dataFloat_in;
    dataFloat_in = 0;
    delete clockFloat_in;
    clockFloat_in = 0;
    delete dataShort_out;
    dataShort_out = 0;
}

/*******************************************************************************************
    Framework-level functions
    These functions are generally called by the framework to perform housekeeping.
*******************************************************************************************/
void BPSK_base::start() throw (CORBA::SystemException, CF::Resource::StartError)
{
    Resource_impl::start();
    ThreadedComponent::startThread();
}

void BPSK_base::stop() throw (CORBA::SystemException, CF::Resource::StopError)
{
    Resource_impl::stop();
    if (!ThreadedComponent::stopThread()) {
        throw CF::Resource::StopError(CF::CF_NOTSET, "Processing thread did not die");
    }
}

void BPSK_base::releaseObject() throw (CORBA::SystemException, CF::LifeCycle::ReleaseError)
{
    // This function clears the component running condition so main shuts down everything
    try {
        stop();
    } catch (CF::Resource::StopError& ex) {
        // TODO - this should probably be logged instead of ignored
    }

    Resource_impl::releaseObject();
}

void BPSK_base::loadProperties()
{
    addProperty(Output_Rate,
                0,
                "Output Rate",
                "",
                "readonly",
                "S/s",
                "external",
                "configure");

    addProperty(Symbol_Length,
                0,
                "Symbol Length",
                "",
                "readonly",
                "bits",
                "external",
                "configure");

}


