#include <stdio.h>

#include <analyzer/plugins/plugin.hpp>
#include <analyzer/ras-data/ras-data-parser.hpp>
#include <hei_util.hpp>
#include <util/pdbg.hpp>
#include <util/trace.hpp>

#include "gtest/gtest.h"

using namespace analyzer;

static const auto nodeId =
    static_cast<libhei::NodeId_t>(libhei::hash<libhei::NodeId_t>("PLL_UNLOCK"));

// Sub-test #1 - single PLL unlock attention on proc 1, clock 1
TEST(PllUnlock, TestSet1)
{
    pdbg_targets_init(nullptr);

    // clang-format off
    libhei::Chip proc0{util::pdbg::getTrgt("/proc0"), P10_20};
    libhei::Chip proc1{util::pdbg::getTrgt("/proc1"), P10_20};
    libhei::Chip ocmb0{util::pdbg::getTrgt("/proc0/pib/perv12/mc0/mi0/mcc0/omi0/ocmb0"), EXPLORER_11};
    libhei::Chip ocmb1{util::pdbg::getTrgt("/proc0/pib/perv12/mc0/mi0/mcc0/omi1/ocmb0"), EXPLORER_11};
    libhei::Chip ocmb2{util::pdbg::getTrgt("/proc0/pib/perv12/mc0/mi0/mcc1/omi0/ocmb0"), EXPLORER_11};
    libhei::Chip ocmb3{util::pdbg::getTrgt("/proc0/pib/perv12/mc0/mi0/mcc1/omi1/ocmb0"), EXPLORER_11};

    libhei::Signature sig00{proc0, 0x9160, 0x02, 0x16, libhei::ATTN_TYPE_CHECKSTOP};
    libhei::Signature sig01{proc0, 0x9160, 0x02, 0x17, libhei::ATTN_TYPE_CHECKSTOP};
    libhei::Signature sig02{proc0, 0xbce5, 0x00, 0x16, libhei::ATTN_TYPE_UNIT_CS};
    libhei::Signature sig03{proc0, 0xbce5, 0x00, 0x17, libhei::ATTN_TYPE_UNIT_CS};
    libhei::Signature sig04{proc0, 0xbce5, 0x01, 0x16, libhei::ATTN_TYPE_UNIT_CS};
    libhei::Signature sig05{proc0, 0xbce5, 0x01, 0x17, libhei::ATTN_TYPE_UNIT_CS};
    libhei::Signature sig06{proc0, 0x9ecb, 0x00, 0x00, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig07{proc0, 0xf1ee, 0x00, 0x03, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig08{proc0, 0xf1ee, 0x00, 0x06, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig09{proc0, 0xf1ee, 0x00, 0x07, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig0a{proc0, 0xf1ee, 0x00, 0x09, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig0b{proc0, 0xf1ee, 0x00, 0x03, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig0c{proc0, 0xf1ee, 0x00, 0x06, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig0d{proc0, 0xf1ee, 0x00, 0x07, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig0e{proc0, 0xf1ee, 0x00, 0x09, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig0f{proc0, 0xf1ee, 0x00, 0x03, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig10{proc0, 0xf1ee, 0x00, 0x06, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig11{proc0, 0xf1ee, 0x00, 0x07, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig12{proc0, 0xf1ee, 0x00, 0x09, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig13{proc0, 0xf1ee, 0x00, 0x03, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig14{proc0, 0xf1ee, 0x00, 0x06, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig15{proc0, 0xf1ee, 0x00, 0x07, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig16{proc0, 0xf1ee, 0x00, 0x09, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig17{proc0, 0xf1ee, 0x01, 0x03, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig18{proc0, 0xf1ee, 0x01, 0x06, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig19{proc0, 0xf1ee, 0x01, 0x07, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig1a{proc0, 0xf1ee, 0x01, 0x09, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig1b{proc0, 0xf1ee, 0x01, 0x03, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig1c{proc0, 0xf1ee, 0x01, 0x06, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig1d{proc0, 0xf1ee, 0x01, 0x07, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig1e{proc0, 0xf1ee, 0x01, 0x09, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig1f{proc0, 0xf1ee, 0x01, 0x03, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig20{proc0, 0xf1ee, 0x01, 0x06, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig21{proc0, 0xf1ee, 0x01, 0x07, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig22{proc0, 0xf1ee, 0x01, 0x09, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig23{proc0, 0xf1ee, 0x01, 0x03, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig24{proc0, 0xf1ee, 0x01, 0x06, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig25{proc0, 0xf1ee, 0x01, 0x07, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig26{proc0, 0xf1ee, 0x01, 0x09, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig27{proc0, 0xf1ee, 0x02, 0x03, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig28{proc0, 0xf1ee, 0x02, 0x06, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig29{proc0, 0xf1ee, 0x02, 0x07, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig2a{proc0, 0xf1ee, 0x02, 0x09, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig2b{proc0, 0xf1ee, 0x02, 0x03, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig2c{proc0, 0xf1ee, 0x02, 0x06, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig2d{proc0, 0xf1ee, 0x02, 0x07, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig2e{proc0, 0xf1ee, 0x02, 0x09, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig2f{proc0, 0xf1ee, 0x02, 0x03, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig30{proc0, 0xf1ee, 0x02, 0x06, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig31{proc0, 0xf1ee, 0x02, 0x07, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig32{proc0, 0xf1ee, 0x02, 0x09, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig33{proc0, 0xf1ee, 0x02, 0x03, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig34{proc0, 0xf1ee, 0x02, 0x06, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig35{proc0, 0xf1ee, 0x02, 0x07, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig36{proc0, 0xf1ee, 0x02, 0x09, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig37{proc0, 0xf1ee, 0x03, 0x03, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig38{proc0, 0xf1ee, 0x03, 0x06, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig39{proc0, 0xf1ee, 0x03, 0x07, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig3a{proc0, 0xf1ee, 0x03, 0x09, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig3b{proc0, 0xf1ee, 0x03, 0x03, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig3c{proc0, 0xf1ee, 0x03, 0x06, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig3d{proc0, 0xf1ee, 0x03, 0x07, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig3e{proc0, 0xf1ee, 0x03, 0x09, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig3f{proc0, 0xf1ee, 0x03, 0x03, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig40{proc0, 0xf1ee, 0x03, 0x06, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig41{proc0, 0xf1ee, 0x03, 0x07, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig42{proc0, 0xf1ee, 0x03, 0x09, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig43{proc0, 0xf1ee, 0x03, 0x03, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig44{proc0, 0xf1ee, 0x03, 0x06, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig45{proc0, 0xf1ee, 0x03, 0x07, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig46{proc0, 0xf1ee, 0x03, 0x09, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig47{proc0, 0x62e9, 0x02, 0x06, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig48{proc0, 0x62e9, 0x02, 0x07, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig49{proc0, 0x62e9, 0x02, 0x0e, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig4a{proc0, 0x62e9, 0x02, 0x0f, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig4b{proc0, 0x62e9, 0x02, 0x10, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig4c{proc0, 0x62e9, 0x02, 0x11, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig4d{proc0, 0x62e9, 0x02, 0x38, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig4e{proc0, 0x62e9, 0x02, 0x39, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig4f{proc0, 0x62e9, 0x04, 0x06, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig50{proc0, 0x62e9, 0x04, 0x07, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig51{proc0, 0x62e9, 0x04, 0x0e, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig52{proc0, 0x62e9, 0x04, 0x0f, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig53{proc0, 0x62e9, 0x04, 0x10, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig54{proc0, 0x62e9, 0x04, 0x11, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig55{proc0, 0x62e9, 0x04, 0x38, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig56{proc0, 0x62e9, 0x04, 0x39, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig57{proc0, 0x62e9, 0x06, 0x06, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig58{proc0, 0x62e9, 0x06, 0x07, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig59{proc0, 0x62e9, 0x06, 0x0e, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig5a{proc0, 0x62e9, 0x06, 0x0f, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig5b{proc0, 0x62e9, 0x06, 0x10, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig5c{proc0, 0x62e9, 0x06, 0x11, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig5d{proc0, 0x62e9, 0x06, 0x38, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig5e{proc0, 0x62e9, 0x06, 0x39, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig5f{proc0, 0x62e9, 0x07, 0x06, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig60{proc0, 0x62e9, 0x07, 0x07, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig61{proc0, 0x62e9, 0x07, 0x0e, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig62{proc0, 0x62e9, 0x07, 0x0f, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig63{proc0, 0x62e9, 0x07, 0x10, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig64{proc0, 0x62e9, 0x07, 0x11, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig65{proc0, 0x62e9, 0x07, 0x38, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig66{proc0, 0x62e9, 0x07, 0x39, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig67{ocmb0, 0xc4f1, 0x00, 0x03, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig68{ocmb0, 0xc4f1, 0x00, 0x06, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig69{ocmb0, 0xc4f1, 0x00, 0x09, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig6a{ocmb0, 0xc4f1, 0x00, 0x03, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig6b{ocmb0, 0xc4f1, 0x00, 0x06, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig6c{ocmb0, 0xc4f1, 0x00, 0x09, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig6d{ocmb0, 0xc4f1, 0x00, 0x03, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig6e{ocmb0, 0xc4f1, 0x00, 0x06, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig6f{ocmb0, 0xc4f1, 0x00, 0x09, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig70{ocmb1, 0xc4f1, 0x00, 0x03, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig71{ocmb1, 0xc4f1, 0x00, 0x06, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig72{ocmb1, 0xc4f1, 0x00, 0x09, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig73{ocmb1, 0xc4f1, 0x00, 0x03, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig74{ocmb1, 0xc4f1, 0x00, 0x06, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig75{ocmb1, 0xc4f1, 0x00, 0x09, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig76{ocmb1, 0xc4f1, 0x00, 0x03, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig77{ocmb1, 0xc4f1, 0x00, 0x06, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig78{ocmb1, 0xc4f1, 0x00, 0x09, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig79{ocmb2, 0xbbd3, 0x00, 0x36, libhei::ATTN_TYPE_UNIT_CS};
    libhei::Signature sig7a{ocmb2, 0xc4f1, 0x00, 0x03, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig7b{ocmb2, 0xc4f1, 0x00, 0x06, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig7c{ocmb2, 0xc4f1, 0x00, 0x09, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig7d{ocmb2, 0xc4f1, 0x00, 0x03, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig7e{ocmb2, 0xc4f1, 0x00, 0x06, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig7f{ocmb2, 0xc4f1, 0x00, 0x09, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig80{ocmb2, 0xc4f1, 0x00, 0x03, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig81{ocmb2, 0xc4f1, 0x00, 0x06, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig82{ocmb2, 0xc4f1, 0x00, 0x09, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig83{ocmb3, 0xc4f1, 0x00, 0x03, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig84{ocmb3, 0xc4f1, 0x00, 0x06, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig85{ocmb3, 0xc4f1, 0x00, 0x09, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig86{ocmb3, 0xc4f1, 0x00, 0x03, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig87{ocmb3, 0xc4f1, 0x00, 0x06, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig88{ocmb3, 0xc4f1, 0x00, 0x09, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig89{ocmb3, 0xc4f1, 0x00, 0x03, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig8a{ocmb3, 0xc4f1, 0x00, 0x06, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig8b{ocmb3, 0xc4f1, 0x00, 0x09, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig8c{proc1, 0xdf2a, 0x00, 0x19, libhei::ATTN_TYPE_CHECKSTOP};
    libhei::Signature sig8d{proc1, 0x62e9, 0x01, 0x06, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig8e{proc1, 0x62e9, 0x01, 0x07, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig8f{proc1, 0x62e9, 0x01, 0x0e, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig90{proc1, 0x62e9, 0x01, 0x0f, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig91{proc1, 0x62e9, 0x01, 0x10, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig92{proc1, 0x62e9, 0x01, 0x11, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig93{proc1, 0x62e9, 0x01, 0x38, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig94{proc1, 0x62e9, 0x01, 0x39, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig95{proc1, 0x62e9, 0x04, 0x06, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig96{proc1, 0x62e9, 0x04, 0x07, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig97{proc1, 0x62e9, 0x04, 0x0e, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig98{proc1, 0x62e9, 0x04, 0x0f, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig99{proc1, 0x62e9, 0x04, 0x10, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig9a{proc1, 0x62e9, 0x04, 0x11, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig9b{proc1, 0x62e9, 0x04, 0x38, libhei::ATTN_TYPE_RECOVERABLE};
    libhei::Signature sig9c{proc1, 0x62e9, 0x04, 0x39, libhei::ATTN_TYPE_RECOVERABLE};
    // clang-format on

    libhei::IsolationData isoData{};
    isoData.addSignature(sig00);
    isoData.addSignature(sig01);
    isoData.addSignature(sig02);
    isoData.addSignature(sig03);
    isoData.addSignature(sig04);
    isoData.addSignature(sig05);
    isoData.addSignature(sig06);
    isoData.addSignature(sig07);
    isoData.addSignature(sig08);
    isoData.addSignature(sig09);
    isoData.addSignature(sig0a);
    isoData.addSignature(sig0b);
    isoData.addSignature(sig0c);
    isoData.addSignature(sig0d);
    isoData.addSignature(sig0e);
    isoData.addSignature(sig0f);
    isoData.addSignature(sig10);
    isoData.addSignature(sig11);
    isoData.addSignature(sig12);
    isoData.addSignature(sig13);
    isoData.addSignature(sig14);
    isoData.addSignature(sig15);
    isoData.addSignature(sig16);
    isoData.addSignature(sig17);
    isoData.addSignature(sig18);
    isoData.addSignature(sig19);
    isoData.addSignature(sig1a);
    isoData.addSignature(sig1b);
    isoData.addSignature(sig1c);
    isoData.addSignature(sig1d);
    isoData.addSignature(sig1e);
    isoData.addSignature(sig1f);
    isoData.addSignature(sig20);
    isoData.addSignature(sig21);
    isoData.addSignature(sig22);
    isoData.addSignature(sig23);
    isoData.addSignature(sig24);
    isoData.addSignature(sig25);
    isoData.addSignature(sig26);
    isoData.addSignature(sig27);
    isoData.addSignature(sig28);
    isoData.addSignature(sig29);
    isoData.addSignature(sig2a);
    isoData.addSignature(sig2b);
    isoData.addSignature(sig2c);
    isoData.addSignature(sig2d);
    isoData.addSignature(sig2e);
    isoData.addSignature(sig2f);
    isoData.addSignature(sig30);
    isoData.addSignature(sig31);
    isoData.addSignature(sig32);
    isoData.addSignature(sig33);
    isoData.addSignature(sig34);
    isoData.addSignature(sig35);
    isoData.addSignature(sig36);
    isoData.addSignature(sig37);
    isoData.addSignature(sig38);
    isoData.addSignature(sig39);
    isoData.addSignature(sig3a);
    isoData.addSignature(sig3b);
    isoData.addSignature(sig3c);
    isoData.addSignature(sig3d);
    isoData.addSignature(sig3e);
    isoData.addSignature(sig3f);
    isoData.addSignature(sig40);
    isoData.addSignature(sig41);
    isoData.addSignature(sig42);
    isoData.addSignature(sig43);
    isoData.addSignature(sig44);
    isoData.addSignature(sig45);
    isoData.addSignature(sig46);
    isoData.addSignature(sig47);
    isoData.addSignature(sig48);
    isoData.addSignature(sig49);
    isoData.addSignature(sig4a);
    isoData.addSignature(sig4b);
    isoData.addSignature(sig4c);
    isoData.addSignature(sig4d);
    isoData.addSignature(sig4e);
    isoData.addSignature(sig4f);
    isoData.addSignature(sig50);
    isoData.addSignature(sig51);
    isoData.addSignature(sig52);
    isoData.addSignature(sig53);
    isoData.addSignature(sig54);
    isoData.addSignature(sig55);
    isoData.addSignature(sig56);
    isoData.addSignature(sig57);
    isoData.addSignature(sig58);
    isoData.addSignature(sig59);
    isoData.addSignature(sig5a);
    isoData.addSignature(sig5b);
    isoData.addSignature(sig5c);
    isoData.addSignature(sig5d);
    isoData.addSignature(sig5e);
    isoData.addSignature(sig5f);
    isoData.addSignature(sig60);
    isoData.addSignature(sig61);
    isoData.addSignature(sig62);
    isoData.addSignature(sig63);
    isoData.addSignature(sig64);
    isoData.addSignature(sig65);
    isoData.addSignature(sig66);
    isoData.addSignature(sig67);
    isoData.addSignature(sig68);
    isoData.addSignature(sig69);
    isoData.addSignature(sig6a);
    isoData.addSignature(sig6b);
    isoData.addSignature(sig6c);
    isoData.addSignature(sig6d);
    isoData.addSignature(sig6e);
    isoData.addSignature(sig6f);
    isoData.addSignature(sig70);
    isoData.addSignature(sig71);
    isoData.addSignature(sig72);
    isoData.addSignature(sig73);
    isoData.addSignature(sig74);
    isoData.addSignature(sig75);
    isoData.addSignature(sig76);
    isoData.addSignature(sig77);
    isoData.addSignature(sig78);
    isoData.addSignature(sig79);
    isoData.addSignature(sig7a);
    isoData.addSignature(sig7b);
    isoData.addSignature(sig7c);
    isoData.addSignature(sig7d);
    isoData.addSignature(sig7e);
    isoData.addSignature(sig7f);
    isoData.addSignature(sig80);
    isoData.addSignature(sig81);
    isoData.addSignature(sig82);
    isoData.addSignature(sig83);
    isoData.addSignature(sig84);
    isoData.addSignature(sig85);
    isoData.addSignature(sig86);
    isoData.addSignature(sig87);
    isoData.addSignature(sig88);
    isoData.addSignature(sig89);
    isoData.addSignature(sig8a);
    isoData.addSignature(sig8b);
    isoData.addSignature(sig8c);
    isoData.addSignature(sig8d);
    isoData.addSignature(sig8e);
    isoData.addSignature(sig8f);
    isoData.addSignature(sig90);
    isoData.addSignature(sig91);
    isoData.addSignature(sig92);
    isoData.addSignature(sig93);
    isoData.addSignature(sig94);
    isoData.addSignature(sig95);
    isoData.addSignature(sig96);
    isoData.addSignature(sig97);
    isoData.addSignature(sig98);
    isoData.addSignature(sig99);
    isoData.addSignature(sig9a);
    isoData.addSignature(sig9b);
    isoData.addSignature(sig9c);
    ServiceData sd{sig06, AnalysisType::SYSTEM_CHECKSTOP, isoData};

    RasDataParser rasData{};
    rasData.getResolution(sig06)->resolve(sd);

    nlohmann::json j{};
    std::string s{};

    // Callout list
    j = sd.getCalloutList();
    s = R"([
    {
        "Deconfigured": false,
        "Guarded": false,
        "LocationCode": "P0",
        "Priority": "M"
    },
    {
        "Deconfigured": false,
        "Guarded": false,
        "LocationCode": "/proc1",
        "Priority": "M"
    }
])";
    EXPECT_EQ(s, j.dump(4));

    // Callout FFDC
    j = sd.getCalloutFFDC();
    s = R"([
    {
        "Callout Type": "Clock Callout",
        "Clock Type": "OSC_REF_CLOCK_1",
        "Priority": "medium"
    },
    {
        "Callout Type": "Hardware Callout",
        "Guard": false,
        "Priority": "medium",
        "Target": "/proc1"
    }
])";
    EXPECT_EQ(s, j.dump(4));
}

// Sub-test #2 - PLL unlock attention on multiple procs and clocks. Isolating
//               only to proc 1 clock 0 PLL unlock attentions.
TEST(PllUnlock, TestSet2)
{
    pdbg_targets_init(nullptr);

    libhei::Chip chip0{util::pdbg::getTrgt("/proc0"), P10_20};
    libhei::Chip chip1{util::pdbg::getTrgt("/proc1"), P10_20};

    // PLL unlock signatures for each clock per processor.
    libhei::Signature sig00{chip0, nodeId, 0, 0, libhei::ATTN_TYPE_CHECKSTOP};
    libhei::Signature sig01{chip0, nodeId, 0, 1, libhei::ATTN_TYPE_CHECKSTOP};
    libhei::Signature sig10{chip1, nodeId, 0, 0, libhei::ATTN_TYPE_CHECKSTOP};
    libhei::Signature sig11{chip1, nodeId, 0, 1, libhei::ATTN_TYPE_CHECKSTOP};

    // Plugins for each processor.
    auto plugin = PluginMap::getSingleton().get(chip1.getType(), "pll_unlock");

    libhei::IsolationData isoData{};
    isoData.addSignature(sig00);
    isoData.addSignature(sig01);
    isoData.addSignature(sig10);
    isoData.addSignature(sig11);
    ServiceData sd{sig10, AnalysisType::SYSTEM_CHECKSTOP, isoData};

    // Call the PLL unlock plugin.
    plugin(0, chip1, sd);

    nlohmann::json j{};
    std::string s{};

    // Callout list
    j = sd.getCalloutList();
    s = R"([
    {
        "Deconfigured": false,
        "Guarded": false,
        "LocationCode": "P0",
        "Priority": "H"
    },
    {
        "Deconfigured": false,
        "Guarded": false,
        "LocationCode": "/proc0",
        "Priority": "M"
    },
    {
        "Deconfigured": false,
        "Guarded": false,
        "LocationCode": "/proc1",
        "Priority": "M"
    }
])";
    EXPECT_EQ(s, j.dump(4));

    // Callout FFDC
    j = sd.getCalloutFFDC();
    s = R"([
    {
        "Callout Type": "Clock Callout",
        "Clock Type": "OSC_REF_CLOCK_0",
        "Priority": "high"
    },
    {
        "Callout Type": "Hardware Callout",
        "Guard": false,
        "Priority": "medium",
        "Target": "/proc0"
    },
    {
        "Callout Type": "Hardware Callout",
        "Guard": false,
        "Priority": "medium",
        "Target": "/proc1"
    }
])";
    EXPECT_EQ(s, j.dump(4));
}
