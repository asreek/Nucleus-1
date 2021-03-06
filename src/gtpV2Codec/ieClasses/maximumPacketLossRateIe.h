/*
 * Copyright (c) 2020, Infosys Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
 /******************************************************************************
 *
 * This is an auto generated file.
 * Please do not edit this file.
 * All edits to be made through template source file
 * <TOP-DIR/scripts/GtpV2StackCodeGen/tts/ietemplate.h.tt>
 ******************************************************************************/
#ifndef MAXIMUMPACKETLOSSRATEIE_H_
#define MAXIMUMPACKETLOSSRATEIE_H_

#include "manual/gtpV2Ie.h"



class MaximumPacketLossRateIe: public GtpV2Ie {
public:
    MaximumPacketLossRateIe();
    virtual ~MaximumPacketLossRateIe();

    bool encodeMaximumPacketLossRateIe(MsgBuffer &buffer,
                 MaximumPacketLossRateIeData const &data);
    bool decodeMaximumPacketLossRateIe(MsgBuffer &buffer,
                 MaximumPacketLossRateIeData &data, Uint16 length);
    void displayMaximumPacketLossRateIe_v(MaximumPacketLossRateIeData const &data,
                 Debug &stream);
};

#endif /* MAXIMUMPACKETLOSSRATEIE_H_ */
