/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "RSPDefinitions"
 * 	found in "../../../asn1/rsp.asn"
 * 	`asn1c -fwide-types -fcompound-names -fincludes-quoted -no-gen-example`
 */

#ifndef	_EUICCSigned2_H_
#define	_EUICCSigned2_H_


#include "asn_application.h"

/* Including external dependencies */
#include "TransactionId.h"
#include "OCTET_STRING.h"
#include "Octet32.h"
#include "constr_SEQUENCE.h"

#ifdef __cplusplus
extern "C" {
#endif

/* EUICCSigned2 */
typedef struct EUICCSigned2 {
	TransactionId_t	 transactionId;
	OCTET_STRING_t	 euiccOtpk;
	Octet32_t	*hashCc	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} EUICCSigned2_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_EUICCSigned2;
extern asn_SEQUENCE_specifics_t asn_SPC_EUICCSigned2_specs_1;
extern asn_TYPE_member_t asn_MBR_EUICCSigned2_1[3];

#ifdef __cplusplus
}
#endif

#endif	/* _EUICCSigned2_H_ */
#include "asn_internal.h"