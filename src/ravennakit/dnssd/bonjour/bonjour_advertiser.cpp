#include "ravennakit/dnssd/bonjour/bonjour.hpp"

#if RAV_HAS_APPLE_DNSSD

    #include "ravennakit/dnssd/bonjour/bonjour_advertiser.hpp"
    #include "ravennakit/dnssd/service_description.hpp"
    #include "ravennakit/dnssd/bonjour/bonjour_txt_record.hpp"

    #include <iostream>
    #include <thread>

void rav::dnssd::bonjour_advertiser::register_service(
    const std::string& reg_type, const char* name, const char* domain, uint16_t port, const txt_record& txt_record
) {
    DNSServiceRef serviceRef = nullptr;
    const auto record = bonjour_txt_record(txt_record);

    DNSSD_THROW_IF_ERROR(DNSServiceRegister(
        &serviceRef, 0, 0, name, reg_type.c_str(), domain, nullptr, htons(port), record.length(), record.bytesPtr(),
        register_service_callback, this
    ));

    service_ref_ = serviceRef;
    DNSSD_THROW_IF_ERROR(DNSServiceProcessResult(service_ref_.service_ref()));
}

void rav::dnssd::bonjour_advertiser::unregister_service() noexcept {
    service_ref_ = nullptr;
}

void rav::dnssd::bonjour_advertiser::register_service_callback(
    DNSServiceRef service_ref, DNSServiceFlags flags, DNSServiceErrorType error_code, const char* service_name,
    const char* reg_type, const char* reply_domain, void* context
) {
    (void)service_ref;
    (void)flags;
    (void)service_name;
    (void)reg_type;
    (void)reply_domain;

    if (error_code != kDNSServiceErr_NoError) {
        auto* owner = static_cast<bonjour_advertiser*>(context);
        owner->emit<events::advertiser_error>(events::advertiser_error {dns_service_error_to_string(error_code)});
        owner->unregister_service();
    }
}

void rav::dnssd::bonjour_advertiser::update_txt_record(const txt_record& txt_record) {
    auto const record = bonjour_txt_record(txt_record);

    // Second argument's nullptr tells us that we are updating the primary record.
    DNSSD_THROW_IF_ERROR(
        DNSServiceUpdateRecord(service_ref_.service_ref(), nullptr, 0, record.length(), record.bytesPtr(), 0)
    );
}

#endif
