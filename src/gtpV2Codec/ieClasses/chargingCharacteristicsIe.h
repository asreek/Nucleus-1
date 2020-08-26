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
#ifndef CHARGINGCHARACTERISTICSIE_H_
#define CHARGINGCHARACTERISTICSIE_H_

#include "manual/gtpV2Ie.h"



class ChargingCharacteristicsIe: public GtpV2Ie {
public:
    ChargingCharacteristicsIe();
    virtual ~ChargingCharacteristicsIe();

    bool encodeChargingCharacteristicsIe(MsgBuffer &buffer,
                 ChargingCharacteristicsIeData const &data);
    bool decodeChargingCharacteristicsIe(MsgBuffer &buffer,
                 ChargingCharacteristicsIeData &data, Uint16 length);
    void displayChargingCharacteristicsIe_v(ChargingCharacteristicsIeData const &data,
                 Debug &stream);
};

#endif /* CHARGINGCHARACTERISTICSIE_H_ */
