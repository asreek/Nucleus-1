
/*
 * Copyright 2020-present Infosys Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */
 
/**************************************
 * detachStart.cpp
 * This is an auto generated file.
 * Please do not edit this file.
 * All edits to be made through template source file
 * <TOP-DIR/scripts/SMCodeGen/templates/stateMachineTmpls/state.cpp.tt>
 **************************************/


#include "actionTable.h"
#include "actionHandlers/actionHandlers.h"
#include "mmeSmDefs.h"
#include "utils/mmeStatesUtils.h"
#include "utils/mmeTimerTypes.h"

#include "mmeStates/detachStart.h"	
#include "mmeStates/detachWfPurgeRespDelSessionResp.h"

using namespace mme;
using namespace SM;

/******************************************************************************
* Constructor
******************************************************************************/
DetachStart::DetachStart():State(detach_start)
{
        stateEntryAction = &MmeStatesUtils::on_state_entry;
        stateExitAction = &MmeStatesUtils::on_state_exit;
        eventValidator = &MmeStatesUtils::validate_event;
}

/******************************************************************************
* Destructor
******************************************************************************/
DetachStart::~DetachStart()
{
}

/******************************************************************************
* creates and returns static instance
******************************************************************************/
DetachStart* DetachStart::Instance()
{
        static DetachStart state;
        return &state;
}

/******************************************************************************
* initializes eventToActionsMap
******************************************************************************/
void DetachStart::initialize()
{
        {
                ActionTable actionTable;
                actionTable.addAction(&ActionHandlers::del_session_req);
                actionTable.addAction(&ActionHandlers::purge_req);
                actionTable.setNextState(DetachWfPurgeRespDelSessionResp::Instance());
                eventToActionsMap.insert(pair<uint16_t, ActionTable>(DETACH_REQ_FROM_UE, actionTable));
        }
        {
                ActionTable actionTable;
                actionTable.addAction(&ActionHandlers::detach_accept_to_ue);
                eventToActionsMap.insert(pair<uint16_t, ActionTable>(STATE_GUARD_TIMEOUT, actionTable));
        }
}
