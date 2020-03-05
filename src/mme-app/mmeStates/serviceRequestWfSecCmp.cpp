  
/*
 * Copyright 2019-present Infosys Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */
 
/**************************************
 * serviceRequestWfSecCmp.cpp
 * This is an auto generated file.
 * Please do not edit this file.
 * All edits to be made through template source file
 * <TOP-DIR/scripts/SMCodeGen/templates/stateMachineTmpls/state.cpp.tt>
 **************************************/

#include "smEnumTypes.h"
#include "actionTable.h"
#include "actionHandlers/actionHandlers.h"

#include "mmeStates/serviceRequestWfSecCmp.h"	
#include "mmeStates/serviceRequestWfInitCtxtResp.h"

using namespace mme;
using namespace SM;

/******************************************************************************
* Constructor
******************************************************************************/
ServiceRequestWfSecCmp::ServiceRequestWfSecCmp():State(State_e::service_request_wf_sec_cmp)
{
}

/******************************************************************************
* Destructor
******************************************************************************/
ServiceRequestWfSecCmp::~ServiceRequestWfSecCmp()
{
}

/******************************************************************************
* creates and returns static instance
******************************************************************************/
ServiceRequestWfSecCmp* ServiceRequestWfSecCmp::Instance()
{
        static ServiceRequestWfSecCmp state;
        return &state;
}

/******************************************************************************
* initializes eventToActionsMap
******************************************************************************/
void ServiceRequestWfSecCmp::initialize()
{
        {
                ActionTable actionTable;
                actionTable.addAction(&ActionHandlers::process_sec_mode_resp);
                actionTable.addAction(&ActionHandlers::send_init_ctxt_req_to_ue_svc_req);
                actionTable.setNextState(ServiceRequestWfInitCtxtResp::Instance());
                eventToActionsMap.insert(pair<Event_e, ActionTable>(Event_e::SEC_MODE_RESP_FROM_UE, actionTable));
        }
        {
                ActionTable actionTable;
                actionTable.addAction(&ActionHandlers::send_s1_rel_cmd_to_ue);
                actionTable.addAction(&ActionHandlers::abort_service_req_procedure);
                eventToActionsMap.insert(pair<Event_e, ActionTable>(Event_e::ABORT_EVENT, actionTable));
        }
}