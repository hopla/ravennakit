#include "ravennakit/dnssd/bonjour/bonjour_shared_connection.hpp"

#include "ravennakit/core/log.hpp"
#include "ravennakit/dnssd/result.hpp"
#include "ravennakit/dnssd/bonjour/bonjour.hpp"

rav::dnssd::bonjour_shared_connection::bonjour_shared_connection() {
    DNSServiceRef ref = nullptr;
    auto r = result(DNSServiceCreateConnection(&ref));
    if (r.has_error())
        RAV_ERROR(r.description());
    else
        service_ref_ = ref;  // From here on the ref is under RAII inside a ScopedDnsServiceRef class
}
