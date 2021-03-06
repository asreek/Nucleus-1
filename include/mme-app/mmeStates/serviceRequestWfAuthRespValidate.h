/*
 * Copyright 2020-present Infosys Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */
 
 /******************************************************
 * serviceRequestWfAuthRespValidate.h
 * This is an auto generated file.
 * Please do not edit this file.
 * All edits to be made through template source file
 * <TOP-DIR/scripts/SMCodeGen/templates/stateMachineTmpls/state.h.tt>
 ******************************************************/
 
#ifndef __ServiceRequestWfAuthRespValidate__
#define __ServiceRequestWfAuthRespValidate__

#include "state.h"

namespace mme {

	class ServiceRequestWfAuthRespValidate : public SM::State
	{
		public:
			/******************************************
			* Instance 
			*    Creates static instance for the state
			*******************************************/
			static ServiceRequestWfAuthRespValidate* Instance();

			/****************************************
			* ServiceRequestWfAuthRespValidate
			*    Destructor
			****************************************/
			~ServiceRequestWfAuthRespValidate();			
			
			/******************************************
			* initialize
			*  Initializes action handlers for the state
			* and next state
			******************************************/
			void initialize();
	
		private:
			/****************************************
			* ServiceRequestWfAuthRespValidate
			*    Private constructor
			****************************************/
			ServiceRequestWfAuthRespValidate();  
	};
};
#endif // __ServiceRequestWfAuthRespValidate__
