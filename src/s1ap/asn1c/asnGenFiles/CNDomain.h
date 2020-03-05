/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "S1AP-IEs"
 * 	found in "./asn1c/S1AP-IEs.asn"
 * 	`asn1c -fcompound-names -fno-include-deps -gen-PER -findirect-choice -pdu=S1AP-PDU`
 */

#ifndef	_CNDomain_H_
#define	_CNDomain_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum CNDomain {
	CNDomain_ps	= 0,
	CNDomain_cs	= 1
} e_CNDomain;

/* CNDomain */
typedef long	 CNDomain_t;

/* Implementation */
extern asn_per_constraints_t asn_PER_type_CNDomain_constr_1;
extern asn_TYPE_descriptor_t asn_DEF_CNDomain;
extern const asn_INTEGER_specifics_t asn_SPC_CNDomain_specs_1;
asn_struct_free_f CNDomain_free;
asn_struct_print_f CNDomain_print;
asn_constr_check_f CNDomain_constraint;
ber_type_decoder_f CNDomain_decode_ber;
der_type_encoder_f CNDomain_encode_der;
xer_type_decoder_f CNDomain_decode_xer;
xer_type_encoder_f CNDomain_encode_xer;
oer_type_decoder_f CNDomain_decode_oer;
oer_type_encoder_f CNDomain_encode_oer;
per_type_decoder_f CNDomain_decode_uper;
per_type_encoder_f CNDomain_encode_uper;
per_type_decoder_f CNDomain_decode_aper;
per_type_encoder_f CNDomain_encode_aper;

#ifdef __cplusplus
}
#endif

#endif	/* _CNDomain_H_ */
#include <asn_internal.h>
