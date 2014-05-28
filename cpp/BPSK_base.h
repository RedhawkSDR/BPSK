#ifndef BPSK_IMPL_BASE_H
#define BPSK_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Resource_impl.h>
#include <ossie/ThreadedComponent.h>

#include <bulkio/bulkio.h>

class BPSK_base : public Resource_impl, protected ThreadedComponent
{
    public:
        BPSK_base(const char *uuid, const char *label);
        ~BPSK_base();

        void start() throw (CF::Resource::StartError, CORBA::SystemException);

        void stop() throw (CF::Resource::StopError, CORBA::SystemException);

        void releaseObject() throw (CF::LifeCycle::ReleaseError, CORBA::SystemException);

        void loadProperties();

    protected:
        // Member variables exposed as properties
        float Output_Rate;
        CORBA::Long Symbol_Length;

        // Ports
        bulkio::InFloatPort *dataFloat_in;
        bulkio::InFloatPort *clockFloat_in;
        bulkio::OutShortPort *dataShort_out;

    private:
};
#endif // BPSK_IMPL_BASE_H
