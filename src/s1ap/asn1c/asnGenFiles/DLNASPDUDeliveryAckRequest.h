/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "S1AP-IEs"
 * 	found in "./asn1c/S1AP-IEs.asn"
 * 	`asn1c -fcompound-names -fno-include-deps -gen-PER -findirect-choice -pdu=S1AP-PDU`
 */

#ifndef	_DLNASPDUDeliveryAckRequest_H_
#define	_DLNASPDUDeliveryAckRequest_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum DLNASPDUDeliveryAckRequest {
	DLNASPDUDeliveryAckRequest_requested	= 0
	/*
	 * Enumeration is extensible
	 */
} e_DLNASPDUDeliveryAckRequest;

/* DLNASPDUDeliveryAckRequest */
typedef long	 DLNASPDUDeliveryAckRequest_t;

/* Implementation */
extern asn_per_constraints_t asn_PER_type_DLNASPDUDeliveryAckRequest_constr_1;
extern asn_TYPE_descriptor_t asn_DEF_DLNASPDUDeliveryAckRequest;
extern const asn_INTEGER_specifics_t asn_SPC_DLNASPDUDeliveryAckRequest_specs_1;
asn_struct_free_f DLNASPDUDeliveryAckRequest_free;
asn_struct_print_f DLNASPDUDeliveryAckRequest_print;
asn_constr_check_f DLNASPDUDeliveryAckRequest_constraint;
ber_type_decoder_f DLNASPDUDeliveryAckRequest_decode_ber;
der_type_encoder_f DLNASPDUDeliveryAckRequest_encode_der;
xer_type_decoder_f DLNASPDUDeliveryAckRequest_decode_xer;
xer_type_encoder_f DLNASPDUDeliveryAckRequest_encode_xer;
oer_type_decoder_f DLNASPDUDeliveryAckRequest_decode_oer;
oer_type_encoder_f DLNASPDUDeliveryAckRequest_encode_oer;
per_type_decoder_f DLNASPDUDeliveryAckRequest_decode_uper;
per_type_encoder_f DLNASPDUDeliveryAckRequest_encode_uper;
per_type_decoder_f DLNASPDUDeliveryAckRequest_decode_aper;
per_type_encoder_f DLNASPDUDeliveryAckRequest_encode_aper;

#ifdef __cplusplus
}
#endif

#endif	/* _DLNASPDUDeliveryAckRequest_H_ */
#include <asn_internal.h>
