/*
 * Copyright (C) 2022-2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 *
 * libinit for OnePlus Pad 3 (OPD2415) / OnePlus Pad 2 Pro (OPD2413),
 * codename "erhai" (SM8750). Based on the OPLUS sm87xx libinit by kmiit.
 */

#include <android-base/logging.h>
#include <android-base/parseint.h>
#include <android-base/properties.h>
#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>

#include <fs_mgr.h>
#include <unordered_map>

using android::base::GetProperty;
using android::fs_mgr::GetKernelCmdline;

const std::unordered_map<int, std::string> kRegionSuffixMap = {
    {27,    "IN"},
    {55,    "RU"},
    {68,    "EEA"},
    {151,   ""},    // CN
    {161,   "NA"},
    {167,   ""},    // GLO
    {0,     ""},    // Default
};

struct ModelInfo {
    const char* brand;              // ro.product.brand
    const char* device;             // ro.product.device
    const char* manufacturer;       // ro.product.manufacturer
    const char* model;              // ro.product.model
    const char* base_name;          // ro.product.name  w/o region suffix
    const char* twversion;          // ro.twrp.device_version
    const char* supportSpr;         // vendor.display.enable_spr
};

const std::unordered_map<int, ModelInfo> kModelInfoMap = {
    // erhai: OnePlus Pad 2 Pro (CN) / OnePlus Pad 3 (IN/EEA/NA/GLO)
    {24926, {"OnePlus", "OP615EL1", "OnePlus", "OPD2413", "OPD2413", "OnePlus_Pad2Pro", "0"}}, // OPD2413 CN
    // TODO: add OPD2415 (OP6190L1) prjname once known; falls back to 24926 mapping otherwise
    {0,     {"OPLUS",   "SM87XX",   "OPLUS",   "SM87XX",  "SM87XX",  "SM87XX",              "0"}}, // Default
};

/*
 * SetProperty does not allow updating read only properties and as a result
 * does not work for our use case. Write "OverrideProperty" to do practically
 * the same thing as "SetProperty" without this restriction.
 */
void OverrideProperty(const char* name, const char* value) {
    size_t valuelen = strlen(value);

    prop_info* pi = (prop_info*)__system_property_find(name);
    if (pi != nullptr) {
        __system_property_update(pi, value, valuelen);
    } else {
        __system_property_add(name, strlen(name), value, valuelen);
    }
}

void SetupModelProperties(const ModelInfo& info, const std::string& region) {
    std::string name = info.base_name + region;
    struct PropPair {
        const char* key;
        const char* value;
    } props[] = {
        {"ro.product.brand",            info.brand},
        {"ro.product.device",           info.device},
        {"ro.product.manufacturer",     info.manufacturer},
        {"ro.product.model",            info.model},
        {"ro.product.name",             name.c_str()},
        {"vendor.display.enable_spr",   info.supportSpr},
        {"ro.twrp.device_version",      info.twversion},
        {"ro.build.date.utc",           "0"},
    };
    for (const auto& p : props) {
        OverrideProperty(p.key, p.value);
    }
}

void vendor_load_properties() {
    std::string buf = "0";
    GetKernelCmdline("oplus_region", &buf);

    auto region = std::stoi(buf);
    auto region_suffix_iter = kRegionSuffixMap.find(region);

    // Handle unknown regions gracefully
    if (region_suffix_iter == kRegionSuffixMap.end()) {
        LOG(WARNING) << "Unknown oplus_region: " << region << ", using default";
        region_suffix_iter = kRegionSuffixMap.find(0);
    }

    auto prjname = std::stoi(GetProperty("ro.boot.prjname", "0"));
    auto model_info = kModelInfoMap.find(prjname);

    // Handle unknown device models gracefully (fall back to the erhai entry
    // so OPD2415 still gets sane properties even if its prjname is unknown)
    if (model_info == kModelInfoMap.end()) {
        LOG(ERROR) << "Unknown prjname: " << prjname << ", using erhai defaults";
        model_info = kModelInfoMap.find(24926);
    }

    SetupModelProperties(model_info->second, region_suffix_iter->second);

    // The erhai panel is natively portrait (2400x3392); rotate the TWRP UI
    // so it appears landscape, matching how the tablet is held/used.
    OverrideProperty("persist.twrp.rotation", "270");

    // Strongbox (NXP) is not disabled on erhai
    OverrideProperty("twrp.se.no_sb", "false");
}
