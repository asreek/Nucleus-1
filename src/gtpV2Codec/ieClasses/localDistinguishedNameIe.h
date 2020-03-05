/*
 * Copyright 2019-present, Infosys Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 /******************************************************************************
 *
 * This is an auto generated file.
 * Please do not edit this file.
 * All edits to be made through template source file
 * <TOP-DIR/scripts/GtpV2StackCodeGen/tts/ietemplate.h.tt>
 ******************************************************************************/
#ifndef LOCALDISTINGUISHEDNAMEIE_H_
#define LOCALDISTINGUISHEDNAMEIE_H_

#include "manual/gtpV2Ie.h"



class LocalDistinguishedNameIe: public GtpV2Ie {
public:
    LocalDistinguishedNameIe();
    virtual ~LocalDistinguishedNameIe();

    bool encodeLocalDistinguishedNameIe(MsgBuffer &buffer,
                 LocalDistinguishedNameIeData const &data);
    bool decodeLocalDistinguishedNameIe(MsgBuffer &buffer,
                 LocalDistinguishedNameIeData &data, Uint16 length);
    void displayLocalDistinguishedNameIe_v(LocalDistinguishedNameIeData const &data,
                 Debug &stream);
};

#endif /* LOCALDISTINGUISHEDNAMEIE_H_ */