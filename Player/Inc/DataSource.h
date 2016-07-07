#ifndef DATA_SOURCE_H_

#define DATA_SOURCE_H_

#include <sys/types.h>

#include "SIGM_Types.h"
#include <KeyedVector.h>
#include <List.h>
#include <RefBase.h>
#include <Threads.h>
#include <String8.h>

class DataSource : /*public VirtualLightRefBase*/ public virtual RefBase {
public:
    enum Flags {
        kWantsPrefetching      = 1,
        kStreamedFromLocalHost = 2,
        kIsCachingDataSource   = 4,
        kIsHTTPBasedSource     = 8,
    };

    static sp<DataSource> CreateFromURI( const char *uri);

   
    DataSource() {}

    virtual Error_Type_e initCheck() const = 0;

    virtual ssize_t readAt(off64_t offset, void *data, size_t size) = 0;

    // Convenience methods:
    bool getUInt16(off64_t offset, uint16_t *x);
    bool getUInt24(off64_t offset, uint32_t *x); // 3 byte int, returned as a 32-bit int
    bool getUInt32(off64_t offset, uint32_t *x);
    bool getUInt64(off64_t offset, uint64_t *x);

    // May return ERROR_UNSUPPORTED.
    virtual Error_Type_e getSize(off64_t *size);

    virtual uint32_t flags() {
        return 0;
    }

    ////////////////////////////////////////////////////////////////////////////

    bool sniff(String8 *mimeType, float *confidence);

    // The sniffer can optionally fill in "meta" with an AMessage containing
    // a dictionary of values that helps the corresponding extractor initialize
    // its state without duplicating effort already exerted by the sniffer.
    typedef bool (*SnifferFunc)(const sp<DataSource> &source, String8 *mimeType,float *confidence);

    static void RegisterDefaultSniffers();

    virtual String8 getUri() {
        return String8();
    }

    virtual String8 getMIMEType() const;

protected:
    virtual ~DataSource() {}

private:
    static Mutex gSnifferMutex;
    static List<SnifferFunc> gSniffers;
    static bool gSniffersRegistered;

    static void RegisterSniffer_l(SnifferFunc func);

    DataSource(const DataSource &);
    DataSource &operator=(const DataSource &);
};
#endif