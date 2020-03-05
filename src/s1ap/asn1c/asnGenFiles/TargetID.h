/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "S1AP-IEs"
 * 	found in "./asn1c/S1AP-IEs.asn"
 * 	`asn1c -fcompound-names -fno-include-deps -gen-PER -findirect-choice -pdu=S1AP-PDU`
 */

#ifndef	_TargetID_H_
#define	_TargetID_H_


#include <asn_application.h>

/* Including external dependencies */
#include <constr_CHOICE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum TargetID_PR {
	TargetID_PR_NOTHING,	/* No components present */
	TargetID_PR_targeteNB_ID,
	TargetID_PR_targetRNC_ID,
	TargetID_PR_cGI,
	/* Extensions may appear below */
	TargetID_PR_targetgNgRanNode_ID
} TargetID_PR;

/* Forward declarations */
struct TargeteNB_ID;
struct TargetRNC_ID;
struct CGI;
struct TargetNgRanNode_ID;

/* TargetID */
typedef struct TargetID {
	TargetID_PR present;
	union TargetID_u {
		struct TargeteNB_ID	*targeteNB_ID;
		struct TargetRNC_ID	*targetRNC_ID;
		struct CGI	*cGI;
		/*
		 * This type is extensible,
		 * possible extensions are below.
		 */
		struct TargetNgRanNode_ID	*targetgNgRanNode_ID;
	} choice;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} TargetID_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_TargetID;
extern asn_CHOICE_specifics_t asn_SPC_TargetID_specs_1;
extern asn_TYPE_member_t asn_MBR_TargetID_1[4];
extern asn_per_constraints_t asn_PER_type_TargetID_constr_1;

#ifdef __cplusplus
}
#endif

#endif	/* _TargetID_H_ */
#include <asn_internal.h>
