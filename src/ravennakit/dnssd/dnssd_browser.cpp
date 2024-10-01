/*
 * Owllab License Agreement
 *
 * This software is provided by Owllab and may not be used, copied, modified,
 * merged, published, distributed, sublicensed, or sold without a valid and
 * explicit agreement with Owllab.
 *
 * Copyright (c) 2024 Owllab. All rights reserved.
 */

#include "ravennakit/dnssd/dnssd_browser.hpp"

std::unique_ptr<rav::dnssd::dnssd_browser> rav::dnssd::dnssd_browser::create() {
#if RAV_MACOS
    return std::make_unique<dnssd_browser_macos>();
#elif RAV_IOS
    return std::make_unique<dnssd_browser_ios>();
#endif
    return {};
}