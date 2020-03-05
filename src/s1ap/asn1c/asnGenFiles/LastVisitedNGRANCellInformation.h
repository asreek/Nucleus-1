/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "S1AP-IEs"
 * 	found in "./asn1c/S1AP-IEs.asn"
 * 	`asn1c -fcompound-names -fno-include-deps -gen-PER -findirect-choice -pdu=S1AP-PDU`
 */

#ifndef	_LastVisitedNGRANCellInformation_H_
#define	_LastVisitedNGRANCellInformation_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* LastVisitedNGRANCellInformation */
typedef OCTET_STRING_t	 LastVisitedNGRANCellInformation_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_LastVisitedNGRANCellInformation;
asn_struct_free_f LastVisitedNGRANCellInformation_free;
asn_struct_print_f LastVisitedNGRANCellInformation_print;
asn_constr_check_f LastVisitedNGRANCellInformation_constraint;
ber_type_decoder_f LastVisitedNGRANCellInformation_decode_ber;
der_type_encoder_f LastVisitedNGRANCellInformation_encode_der;
xer_type_decoder_f LastVisitedNGRANCellInformation_decode_xer;
xer_type_encoder_f LastVisitedNGRANCellInformation_encode_xer;
oer_type_decoder_f LastVisitedNGRANCellInformation_decode_oer;
oer_type_encoder_f LastVisitedNGRANCellInformation_encode_oer;
per_type_decoder_f LastVisitedNGRANCellInformation_decode_uper;
per_type_encoder_f LastVisitedNGRANCellInformation_encode_uper;
per_type_decoder_f LastVisitedNGRANCellInformation_decode_aper;
per_type_encoder_f LastVisitedNGRANCellInformation_encode_aper;

#ifdef __cplusplus
}
#endif

#endif	/* _LastVisitedNGRANCellInformation_H_ */
#include <asn_internal.h>
