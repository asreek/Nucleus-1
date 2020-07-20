#include "log.h"
#include "mmeNasUtils.h"
#include "structs.h"
#include "f9.h"
#include "nas_headers.h"
#include <utils/mmeCommonUtils.h>
#include <contextManager/subsDataGroupManager.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/cmac.h>

using namespace SM;
using namespace mme;
using namespace cmn;

#define NAS_SERVICE_REQUEST 0x4D
#define AES_128_KEY_SIZE 16
static unsigned short get_length(unsigned char **msg) 
{
    /* get length and consume msg bytes accordingly */

    unsigned short ie_len = 0;

    unsigned char val = ((*msg)[0] & 0xc0) >> 6;
    if(val == 2) {
        //log_msg(LOG_INFO, "length more than 128\n");
        unsigned short higher = (unsigned char)(*msg)[0] & 0x3f;
        (*msg)++;
        unsigned short lower = (unsigned char)(*msg)[0];
        ie_len = (higher << 8) | lower;
    } else {
        //log_msg(LOG_INFO, "length less than 128\n");
        ie_len = (unsigned short)(*msg)[0];
    }
    (*msg)++;
    return ie_len;
}

static unsigned char *msg_to_hex_str(unsigned char *msg, int len, unsigned char **buffer) 
{

  char chars[]= "0123456789abcdef";
  unsigned char *local;

  if (!len)
      return NULL;

  if (!((*buffer) = (unsigned char *)malloc(2 * len + 1)))
      return NULL;

  local = *buffer;
  for (int i = 0; i < len; i++) {
      local[2 * i] = chars[(msg[i] >> 4) & 0x0F];
      local[2 * i + 1] = chars[(msg[i]) & 0x0F];
  }
  local[2 * len] = '\0';

  return local;
}


void printBytes(unsigned char *buf, size_t len) 
{
    //Can enable if needed to check encrypted bytes.
#if 0
  for(unsigned int i=0; i<len; i++) {
    log_msg(LOG_DEBUG,"%02x \n", buf[i]);
  }
  log_msg(LOG_DEBUG,"\n");
#endif
}

static void
calculate_aes_mac(uint8_t *int_key, uint32_t count, uint8_t direction,
		uint8_t bearer, uint8_t *data, uint16_t data_len,
		uint8_t *mac)
{
  unsigned char mact[16] = {0};
  size_t mactlen = 0;
  unsigned char* message = (unsigned char*)calloc(data_len+8, sizeof(uint8_t));
  uint32_t msg_len = 0;
  log_msg(LOG_DEBUG,"count %d, bearer %d direction %d, data_len %d \n", count, bearer, direction, data_len);
  log_msg(LOG_DEBUG,"nas data \n");
  printBytes(data, data_len);
  log_msg(LOG_DEBUG,"nas key \n");
  printBytes(int_key, AES_128_KEY_SIZE);
  if(message == NULL)
  {
      log_msg(LOG_ERROR,"Memory alloc for mac calculation failed.\n");
      return;
  }
  else
  {
      uint32_t local_count = htonl(count);
      msg_len = data_len + 8;
      memcpy (&message[0], &local_count, 4);
      message[4] = ((bearer & 0x1F) << 3) | ((direction & 0x01) << 2);
      memcpy(&message[8], data, data_len);
  }

  printBytes(message, msg_len);
  CMAC_CTX *ctx = CMAC_CTX_new();
  CMAC_Init(ctx, int_key, 16, EVP_aes_128_cbc(), NULL);
  printf("message length = %lu bytes (%lu bits)\n",sizeof(message), sizeof(message)*8);

  CMAC_Update(ctx, message, sizeof(message));
  CMAC_Final(ctx, mact, &mactlen);
  log_msg(LOG_DEBUG,"mac length = %lu\n",mactlen);

  printBytes(mact, mactlen);

  CMAC_CTX_free(ctx);
  memcpy(mac, mact, MAC_SIZE);
  return;
}

#if 0
static void
calculate_aes_mac(uint8_t *int_key, uint32_t count, uint8_t direction,
		uint8_t bearer, uint8_t *data, uint16_t data_len,
		uint8_t *mac)
{
  unsigned char mact[16] = {0};
  EVP_MAC *mac_evp = EVP_MAC_fetch(NULL, "CMAC", NULL);
  const char cipher[] = "AES-128-CBC";
  EVP_MAC_CTX *ctx = NULL;

  log_msg(LOG_DEBUG,"count %d, bearer %d direction %d, data_len %d \n", count, bearer, direction, data_len);
  log_msg(LOG_DEBUG,"nas data \n");
  printBytes(data, data_len);
  OSSL_PARAM params[3];
  size_t params_n = 0;
  size_t mactlen = 0;
  unsigned char* message = (unsigned char*)calloc(data_len+8, sizeof(uint8_t));
  uint32_t msg_len = 0;
  if(message == NULL)
  {
      log_msg(LOG_ERROR,"Memory alloc for mac calculation failed.\n");
      return;
  }
  else
  {
      uint32_t local_count = htonl(count);
      msg_len = data_len + 8;
      memcpy (&message[0], &local_count, 4);
      message[4] = ((bearer & 0x1F) << 3) | ((direction & 0x01) << 2);
      memcpy(&message[8], data, data_len);
  }

  log_msg(LOG_DEBUG,"cipher %s %d\n",cipher, strlen(cipher));
  printBytes(int_key, AES_128_KEY_SIZE);
  log_msg(LOG_DEBUG,"key  %d\n", strlen((const char*)int_key));
  params[params_n++] =
      OSSL_PARAM_construct_utf8_string("cipher", (char*)cipher, strlen(cipher));
  params[params_n++] =
      OSSL_PARAM_construct_octet_string("key", int_key, AES_128_KEY_SIZE);
  params[params_n] = OSSL_PARAM_construct_end();

  ctx = EVP_MAC_CTX_new(mac_evp);
  if(ctx==NULL)
  {
      log_msg(LOG_ERROR,"ctx null\n");
      return;
  }

  if(EVP_MAC_CTX_set_params(ctx, params) <= 0)
  {
      log_msg(LOG_ERROR,"set params fail\n");
      return;
  }

  if(!EVP_MAC_init(ctx))
  {
      log_msg(LOG_ERROR,"init fail");
      return;
  }

  printBytes(message, msg_len);
  EVP_MAC_update(ctx, message, msg_len);
  log_msg(LOG_DEBUG,"message length = %lu bytes (%lu bits)\n", 
                strlen((const char*)message), strlen((const char*)message)*8);
  EVP_MAC_final(ctx, mact, &mactlen, msg_len);
  log_msg(LOG_DEBUG,"mac length = %lu\n",mactlen);

  printBytes(mact, mactlen);
  /* expected result T = 070a16b4 6b4d4144 f79bdd9d d04a287c */

  EVP_MAC_CTX_free(ctx);
  memcpy(mac, mact, MAC_SIZE);
  return;
}
#endif

static void
calculate_s3g_mac(uint8_t *int_key, uint32_t seq_no, uint8_t direction,
        uint8_t bearer, uint8_t *data, uint16_t data_len,
        uint8_t *mac)
{
    uint8_t *out;
    log_msg(LOG_DEBUG,"count %d, bearer %d direction %d, data_len %d \n", seq_no, bearer, direction, data_len);
    log_msg(LOG_DEBUG,"nas data \n");
    printBytes(data, data_len);
    log_msg(LOG_DEBUG,"nas key \n");
    printBytes(int_key, AES_128_KEY_SIZE);

    out = f9(int_key, seq_no, bearer, direction, data, data_len * 8);

    memcpy(mac, out, MAC_SIZE);

    return;
}

static void
calculate_mac(uint8_t *int_key, uint32_t count, uint8_t direction,
		uint8_t bearer, uint8_t *data, uint16_t data_len,
		uint8_t *mac, nas_int_algo_enum int_alg)
{
    log_msg(LOG_DEBUG, "Calculate mac. Int Alg : %d.\n",int_alg);
    switch(int_alg)
    {
        case NAS_INT_ALGORITHMS_EIA2:
            log_msg(LOG_DEBUG,"Integrity algo use is AES.\n");
            calculate_aes_mac(int_key, count, direction, 
                              bearer, data, data_len, mac);
            break;
        case NAS_INT_ALGORITHMS_EIA1:
            log_msg(LOG_WARNING, "Integrity algo use is Snow3G. Need to apply for license to productize this.\n");
            calculate_s3g_mac(int_key, count, direction, bearer, data, data_len, mac);
            break;
        default:
            log_msg(LOG_ERROR,"Integrity Algo not supported. Defaulting to no Integrity.\n");

    }
}

static void log_buffer_free(unsigned char** buffer)
{
    if(*buffer != NULL)
        free(*buffer);
    *buffer = NULL;
}

void MmeNasUtils::select_sec_alg(UEContext *ue_ctxt)
{
    log_msg(LOG_DEBUG, "Inside select_sec_alg \n");

    uint8_t eea;
    uint8_t eia;
    nas_int_algo_enum int_alg;
    nas_ciph_algo_enum sec_alg;
    memcpy(&eea,
           &ue_ctxt->getUeNetCapab().ue_net_capab_m.capab[0],sizeof(uint8_t));
    memcpy(&eia,
           &ue_ctxt->getUeNetCapab().ue_net_capab_m.capab[1],sizeof(uint8_t));

    int_alg = (nas_int_algo_enum)MmeCommonUtils::select_preferred_int_algo(eia);
    sec_alg = (nas_ciph_algo_enum)MmeCommonUtils::select_preferred_sec_algo(eea);

    ue_ctxt->getUeSecInfo().setSelectedIntAlg(int_alg);
    ue_ctxt->getUeSecInfo().setSelectedSecAlg(sec_alg);

}

int MmeNasUtils::parse_nas_pdu(s1_incoming_msg_data_t* msg_data, unsigned char *msg,  int nas_msg_len, struct nasPDU *nas)
{
   	unsigned short msg_len = nas_msg_len;
   	unsigned char* msg_end = msg + nas_msg_len;

   	unsigned char *bufflog = NULL;
   	log_msg(LOG_INFO, "NAS PDU msg: %s\n", msg_to_hex_str(msg, msg_len, &bufflog));
   	log_buffer_free(&bufflog);

   	nas_pdu_header_sec nas_header_sec;
   	nas_pdu_header_short nas_header_short;
   	nas_pdu_header_long nas_header_long;

   	unsigned char sec_header_type;
   	unsigned char protocol_discr;
    bool skip_mac_check = false;

   	sec_header_type = (msg[0] >> 4) & 0x0F;
   	protocol_discr = msg[0] & 0x0F;
   	unsigned char is_ESM = ((unsigned short)protocol_discr == 0x02);  // see TS 24.007
   	log_msg(LOG_INFO, "Security header=%d\n", sec_header_type);
   	log_msg(LOG_INFO, "Protocol discriminator=%d\n", protocol_discr);
   	log_msg(LOG_INFO, "is_ESM=%d\n", is_ESM);

    unsigned char *buffer = NULL;
    if(0 != sec_header_type) 
	{ /*security header*/
        log_msg(LOG_INFO, "Security header\n");
        if(SERVICE_REQ_SECURITY_HEADER == sec_header_type)
        {
                log_msg(LOG_INFO, "Recvd security header for Service request.");
                nas->header.security_header_type = sec_header_type;
                nas->header.proto_discriminator = protocol_discr;
                buffer = msg;
                msg += 1;
                nas->header.ksi = msg[0] >> 4;
                nas->header.seq_no = msg[0] & 0x0F;
                msg += 1;
                memcpy(nas->header.short_mac, msg, SHORT_MAC_SIZE);
                nas->header.message_type = NAS_SERVICE_REQUEST;
        }
        else
        {
            memcpy(&nas_header_sec, msg, sizeof(nas_pdu_header_sec));
            if((sec_header_type == 1)||(sec_header_type = 2))
            {
                log_msg(LOG_DEBUG,"header type Integrity protected.\n");
                log_msg(LOG_INFO, "seq no=%x\n", nas_header_sec.seq_no);
                nas->header.seq_no = nas_header_sec.seq_no; 
                unsigned char *msg_ptr = msg + 6;

                sec_header_type = msg_ptr[0] >> 4;
                protocol_discr = msg_ptr[0] & 0x0F;
                unsigned char is_EMM = ((unsigned short)protocol_discr == 0x07);  // see TS 24.007
                log_msg(LOG_INFO, "Security header=%d\n", sec_header_type);
                log_msg(LOG_INFO, "Protocol discriminator=%d\n", protocol_discr);
                log_msg(LOG_INFO, "is_EMM=%d\n", is_EMM);
                if(is_EMM){
                    log_msg(LOG_INFO, "NAS PDU is EMM\n");
                    memcpy(&nas_header_short, msg_ptr, sizeof(nas_header_short)); /*copy only till msg type*/
                    msg_ptr += 2;

                    nas->header.security_header_type = nas_header_short.security_header_type;
                    nas->header.proto_discriminator = nas_header_short.proto_discriminator;
                    nas->header.message_type = nas_header_short.message_type;
                }

                if((NAS_IDENTITY_RESPONSE == nas->header.message_type)
                   || (NAS_AUTH_RESP == nas->header.message_type)
                   || (NAS_AUTH_FAILURE == nas->header.message_type)
                   || (NAS_TAU_REQUEST == nas->header.message_type)
                   || (NAS_DETACH_REQUEST == nas->header.message_type))
                {
                    log_msg(LOG_DEBUG,"No Need for mac check.\n");
                    skip_mac_check = true;
                }
            }
        }

        if(!skip_mac_check)
        {
            SM::ControlBlock* controlBlk_p = NULL;
            if(nas->header.message_type == NAS_SERVICE_REQUEST)
            {
                msg_data->ue_idx = 
                        ntohl(msg_data->msg_data.rawMsg.s_tmsi.m_TMSI);
                log_msg(LOG_DEBUG,"ue_idx : %u\n", msg_data->ue_idx);
			    uint32_t cbIndex = 
                    SubsDataGroupManager::Instance()->findCBWithmTmsi(
                                                            msg_data->ue_idx);
                log_msg(LOG_DEBUG,"cb_index : %u\n", cbIndex);
                controlBlk_p = 
                    SubsDataGroupManager::Instance()->findControlBlock(cbIndex);
                if(controlBlk_p != NULL)
                {
                    UEContext* ueCtxt_p = 
                        static_cast<UEContext*>(
                                        controlBlk_p->getPermDataBlock());
                    if(ueCtxt_p != NULL)
                    {
                        Secinfo &secContext = ueCtxt_p->getUeSecInfo();
                        unsigned char *bufflog = NULL;
                        log_msg(LOG_INFO, "mac=%s\n", 
                                msg_to_hex_str(
                                   (unsigned char *)nas->header.short_mac, 
                                    SHORT_MAC_SIZE, &bufflog));
                        log_buffer_free(&bufflog);
                        /* calculate mac and compare with received mac */
                        unsigned char int_key[NAS_INT_KEY_SIZE];
                        unsigned char calc_mac[MAC_SIZE] = {0};
                        unsigned char* macPtr;
                        uint32_t ul_count = secContext.getUplinkCount();
                        uint8_t direction = SEC_DIRECTION_UPLINK;
                        uint8_t bearer = 0;
                        int buf_len = 2;
                        secContext.getIntKey(&int_key[0]);
                        nas_int_algo_enum int_alg = 
                            secContext.getSelectIntAlg();
                        calculate_mac(int_key, ul_count,
                                      direction, bearer, buffer, buf_len,
                                      calc_mac, int_alg);

                        log_msg(LOG_DEBUG, "Check Service Req Short mac.\n");
                        unsigned char short_mac_local[SHORT_MAC_SIZE] = {0};
                        bufflog = NULL;
                        log_msg(LOG_INFO, "calcmac=%s\n", 
                                msg_to_hex_str(
                                   (unsigned char *)calc_mac, 
                                    MAC_SIZE, &bufflog));
                        log_buffer_free(&bufflog);
                        macPtr = calc_mac + 2;
                        memcpy(short_mac_local, macPtr, SHORT_MAC_SIZE);
                        log_msg(LOG_INFO, "short calc mac=%s\n", 
                                msg_to_hex_str(
                                   (unsigned char *)short_mac_local, 
                                    SHORT_MAC_SIZE, &bufflog));
                        log_buffer_free(&bufflog);
                        if(memcmp(nas->header.short_mac, 
                                  short_mac_local, SHORT_MAC_SIZE))
                        {
                            log_msg(LOG_ERROR,"MAC not matching. Fail msg.\n");
                            return E_FAIL;
                        }

                        log_msg(LOG_DEBUG, "MAC matched for service req.\n");
                        secContext.increment_uplink_count();
                        return SUCCESS;
                    }
                    else
                    {
                        log_msg(LOG_DEBUG, "No Ue context.\n");
                        return E_FAIL;
                    }
                }
                else
                {
                    log_msg(LOG_ERROR,"Control block not found\n");
                    return E_FAIL;
                }
            }
            else
            {
                controlBlk_p = 
                    SubsDataGroupManager::Instance()->findControlBlock(
                                                            msg_data->ue_idx);

                if(controlBlk_p != NULL)
                {
                    UEContext* ueCtxt_p = static_cast<UEContext*>(
                                                controlBlk_p->getPermDataBlock());
                    if(ueCtxt_p != NULL)
                    {
                        Secinfo &secContext = ueCtxt_p->getUeSecInfo();
                        unsigned char *bufflog = NULL;
                        log_msg(LOG_INFO, "mac=%s\n", msg_to_hex_str((unsigned char *)nas_header_sec.mac, MAC_SIZE, &bufflog));
                        log_buffer_free(&bufflog);
                        /* calculate mac and compare with received mac */
                        unsigned char int_key[NAS_INT_KEY_SIZE];
                        unsigned char calc_mac[MAC_SIZE] = {0};
                        uint32_t ul_count = secContext.getUplinkCount();
                        uint8_t direction = SEC_DIRECTION_UPLINK;
                        uint8_t bearer = 0;
                        buffer = msg + sizeof(nas_pdu_header_sec) - sizeof(uint8_t);
                        int buf_len = nas_msg_len - sizeof(nas_pdu_header_sec) + sizeof(uint8_t);
                        secContext.getIntKey(&int_key[0]);
                        nas_int_algo_enum int_alg = secContext.getSelectIntAlg();
                        calculate_mac(int_key, ul_count,
                                      direction, bearer, buffer, buf_len,
                                      calc_mac, int_alg);
                        bufflog = NULL;
                        log_msg(LOG_INFO, "calcmac=%s\n", 
                                msg_to_hex_str(
                                               (unsigned char *)calc_mac, 
                                               MAC_SIZE, &bufflog));
                        log_buffer_free(&bufflog);
                        if(memcmp(nas_header_sec.mac, calc_mac, MAC_SIZE))
                        {
                            log_msg(LOG_ERROR,"MAC not matching. Fail msg.\n");
                            return E_FAIL;
                        }
                            
                        log_msg(LOG_DEBUG, "MAC matched.\n");
                        secContext.increment_uplink_count();
                    }
                }
                else
                {
                    log_msg(LOG_ERROR,"Control block not found\n");
                }
            }
        }
        
        log_msg(LOG_INFO, "seq no=%x\n", nas_header_sec.seq_no);
        nas->header.seq_no = nas_header_sec.seq_no; 
        msg += 6;

        sec_header_type = msg[0] >> 4;
        protocol_discr = msg[0] & 0x0F;
        unsigned char is_ESM = ((unsigned short)protocol_discr == 0x02);  // see TS 24.007
        log_msg(LOG_INFO, "Security header=%d\n", sec_header_type);
        log_msg(LOG_INFO, "Protocol discriminator=%d\n", protocol_discr);
        log_msg(LOG_INFO, "is_ESM=%d\n", is_ESM);
        if (is_ESM) {
                log_msg(LOG_INFO, "NAS PDU is ESM\n");
                memcpy(&nas_header_long, msg, sizeof(nas_header_long)); /*copy only till msg type*/
                msg += 3;

                nas->header.security_header_type = nas_header_long.security_header_type;
                nas->header.proto_discriminator = nas_header_long.proto_discriminator;
                nas->header.procedure_trans_identity = nas_header_long.procedure_trans_identity;
                nas->header.message_type = nas_header_long.message_type;
        } else {
                log_msg(LOG_INFO, "NAS PDU is EMM\n");
                memcpy(&nas_header_short, msg, sizeof(nas_header_short)); /*copy only till msg type*/
                msg += 2;

                nas->header.security_header_type = nas_header_short.security_header_type;
                nas->header.proto_discriminator = nas_header_short.proto_discriminator;
                nas->header.message_type = nas_header_short.message_type;
        }
    } 
	else 
	{
        log_msg(LOG_INFO, "No security header\n");
        memcpy(&nas_header_short, msg, sizeof(nas_header_short)); /*copy only till msg type*/
        msg += 2;

        nas->header.security_header_type = nas_header_short.security_header_type;
        nas->header.proto_discriminator = nas_header_short.proto_discriminator;
        nas->header.message_type = nas_header_short.message_type;
    }


    log_msg(LOG_INFO, "Nas msg type: %X\n", nas->header.message_type);

    switch(nas->header.message_type) 
    {
        case NAS_ESM_RESP:
		{
            log_msg(LOG_INFO, "NAS_ESM_RESP recvd\n");

            unsigned char element_id;
            memcpy(&element_id, msg, 1);
            msg++;
            nas->elements_len +=1;

            nas->elements = (nas_pdu_elements *) calloc(sizeof(nas_pdu_elements), 2);

            nas->elements[0].msgType = NAS_IE_TYPE_APN;
            memcpy(&(nas->elements[0].pduElement.apn.len), msg, 1);
            msg++;
            memcpy(nas->elements[0].pduElement.apn.val, msg, nas->elements[0].pduElement.apn.len);
            log_msg(LOG_INFO, "APN name - %s\n", nas->elements[0].pduElement.apn.val);
            break;
        }

        case NAS_SEC_MODE_COMPLETE:
		{
            log_msg(LOG_INFO, "NAS_SEC_MODE_COMPLETE recvd\n");
            break;
		}

        case NAS_AUTH_RESP:
		{
            log_msg(LOG_INFO, "NAS_AUTH_RESP recvd\n");
            nas->elements_len = 1;
            nas->elements = (nas_pdu_elements *)calloc(sizeof(nas_pdu_elements), 5);
            //if(NULL == nas.elements)...
            memcpy(&(nas->elements[0].pduElement.auth_resp), msg, sizeof(struct XRES));
            uint64_t res = *(uint64_t *)(&nas->elements[0].pduElement.auth_resp.val);
            log_msg(LOG_INFO, "NAS_AUTH_RESP recvd len %d %lu\n",nas->elements[0].pduElement.auth_resp.len,res);
            break;
		}

        case NAS_IDENTITY_RESPONSE: 
		{
            log_msg(LOG_INFO, "NAS_IDENTITY_RESPONSE recvd\n");
            nas->elements_len = 1;
            nas->elements = (nas_pdu_elements *)calloc(sizeof(nas_pdu_elements), 1);
            unsigned short imsi_len = get_length(&msg);
            /*EPS mobility identity. TODO : More error checking */
            memcpy(&(nas->elements[0].pduElement.IMSI), msg, imsi_len);
            break;
        }

        case NAS_TAU_REQUEST:
		{
            log_msg(LOG_INFO, "NAS_TAU_REQUEST recvd\n");
            break;
		}

        case NAS_AUTH_FAILURE:
        {
                nas->elements_len = 1;
                nas->elements = (nas_pdu_elements *) calloc(sizeof(nas_pdu_elements), 1);
                //if(NULL == nas.elements)...
                char err = *(char*)(msg);
                if(err == AUTH_SYNC_FAILURE)
                {
                        log_msg(LOG_INFO, "AUTH Sync Failure. Start Re-Sync");
                        nas->elements[0].msgType = NAS_IE_TYPE_AUTH_FAIL_PARAM;
                        memcpy(&(nas->elements[0].pduElement.auth_fail_resp), msg + 2, sizeof(struct AUTS));
                }
                else       
                {
                        log_msg(LOG_ERROR, "Authentication Failure. Mac Failure");
                }

        }break;
        case NAS_ATTACH_REQUEST:
		{
            log_msg(LOG_INFO, "NAS_ATTACH_REQUEST recvd\n");

            unsigned char tmp = msg[0];
            nas->header.security_encryption_algo = (tmp & 0xF0) >> 4;
            nas->header.security_integrity_algo = tmp & 0x0F;
            msg++;

            nas->elements_len = 7;
            nas->elements = (nas_pdu_elements *) calloc(sizeof(nas_pdu_elements), nas->elements_len);


            int index = 0;
            unsigned short imsi_len = get_length(&msg);

            bool odd = msg[0] & 0x08;
            unsigned char eps_identity = msg[0] & 0x07;
            switch(eps_identity) 
			{
                case 0x01: 
				{
                    // Mobile Identity contains imsi
                    nas->flags |= NAS_MSG_UE_IE_IMSI;
                    log_msg(LOG_INFO, "IMSI len=%u - %u\n", imsi_len, BINARY_IMSI_LEN);
                    nas->elements[index].msgType = NAS_IE_TYPE_EPS_MOBILE_ID_IMSI;
                    memcpy(&(nas->elements[index].pduElement.IMSI), msg, imsi_len);
                    break;
                }
                case 0x06: 
				{
                    log_msg(LOG_INFO, "Mobile identity GUTI Rcvd \n");
                    // Mobile Identity contains GUTI
                    // MCC+MNC offset = 3
                    // MME Group Id   = 2
                    // MME Code       = 1
                    // MTMSI offset from start of this AVP = 3 + 2 + 1
                    nas->elements[index].msgType = NAS_IE_TYPE_EPS_MOBILE_ID_IMSI;
                    memcpy(&nas->elements[index].pduElement.mi_guti.plmn_id.idx, &msg[1], 3);
                    nas->elements[index].pduElement.mi_guti.mme_grp_id = ntohs(*(short int *)(&msg[4]));
                    nas->elements[index].pduElement.mi_guti.mme_code = msg[6];
                    nas->elements[index].pduElement.mi_guti.m_TMSI = ntohl(*((unsigned int *)(&msg[7])));
                    log_msg(LOG_INFO, "NAS Attach Request Rcvd ID: GUTI. PLMN id %d %d %d \n", nas->elements[index].pduElement.mi_guti.plmn_id.idx[0], 
                                    nas->elements[index].pduElement.mi_guti.plmn_id.idx[1], 
                                    nas->elements[index].pduElement.mi_guti.plmn_id.idx[2] );
                    log_msg(LOG_INFO, "NAS Attach Request Rcvd ID: GUTI. mme group id = %d, MME code %d  mtmsi = %d\n", 
                                    nas->elements[index].pduElement.mi_guti.mme_grp_id, 
                                    nas->elements[index].pduElement.mi_guti.mme_code,
                                    nas->elements[index].pduElement.mi_guti.m_TMSI);
                    nas->flags |= NAS_MSG_UE_IE_GUTI;
                    break;
                }
                case 0x03: 
				{
                   // Mobile Identity contains imei
                	break;
                }
                }

                msg += imsi_len;
                unsigned char *bufflog = NULL;
                log_msg(LOG_INFO, "IMSI=%s [to be read nibble-swapped]\n",
                                msg_to_hex_str((unsigned char *)nas->elements[index].pduElement.IMSI, imsi_len, &bufflog));
                log_buffer_free(&bufflog);

                /*UE network capacity*/
                index++;
                nas->elements[index].msgType = 
                        NAS_IE_TYPE_UE_NETWORK_CAPABILITY;
                nas->elements[index].pduElement.ue_network.len = msg[0];
                msg++;
                memcpy((nas->elements[index].pduElement.ue_network.capab)
                       ,msg, nas->elements[index].pduElement.ue_network.len);
                msg += nas->elements[index].pduElement.ue_network.len;

                index++;
                /*ESM msg container*/
                unsigned short len = msg[0] << 8 | msg[1];
                msg += 2;
                //now msg points to ESM message contents
                log_msg(LOG_INFO, "len=%x\n", len);
                log_msg(LOG_INFO, "msg[0]=%x\n", msg[0]);
                nas->elements[index].pduElement.pti = msg[1];
                nas->elements[index].msgType = NAS_IE_TYPE_PTI;
                log_msg(LOG_INFO, "pti=%x\n", nas->elements[index].pduElement.pti);
                unsigned short int msg_offset = 4;
                /*ESM message header len is 4: bearer_id_flags(1)+proc_tx_id(1)+msg_id(1)
                 * +pdn_type(1)*/
                /*element id 13(1101....) = "esm required" flag*/
                //if tx_flag is absent then it means flag is set to false 
                //nas->elements[index].pduElement.esm_info_tx_required = false;
                while(msg_offset < len)
                {
                    unsigned char val = msg[msg_offset];
                    log_msg(LOG_INFO, "ESM container AVP val=%x\n", val);
                    if(13 == (val>>4))
                    {
                        index++;
                        nas->elements[index].msgType = NAS_IE_TYPE_TX_FLAG;
                        // byte 0 - EBI+PD, byte1 - pti, byte2 - message type, byte3 - pdntype+reqtype, byte4 - ESM info transfer flag == Total 5 bytes... msg[0] to msg[4]
                        //nas->elements[2].esm_info_tx_required = true;
                        if(val & 1) {
                                nas->elements[index].pduElement.esm_info_tx_required = true;
                                log_msg(LOG_INFO, "ESM information requested ");
                        }
                        msg_offset++; /* just one byte AVP */
                        continue;

                    }

                    if(0x27 == val)
                    {
                        unsigned char pco_length = msg[msg_offset+1];
                        // element Id 1 byte and pco length 1 byte 
                        // Copy from - 1 byte header Extension + Configuration Protocol
                        index++;
                        nas->elements[index].msgType = NAS_IE_TYPE_PCO;
                        memcpy(&nas->elements[index].pduElement.pco_opt.pco_options[0], &msg[msg_offset+2], pco_length); 
                        nas->elements[index].pduElement.pco_opt.pco_length = pco_length;
                        msg_offset = pco_length + 2; // msg offset was already at PCO AVP type. Now it should point to next AVP type
                        log_msg(LOG_DEBUG, "PCO length = %d \n", pco_length);
                        continue;
                    }
                    break; // unhandled ESM AVP...Add support..for now just break out..else we would be in tight loop
                }
                msg += len;

                unsigned char elem_id = msg[0];
                while(msg != msg_end)
                {
                    elem_id = msg[0];
                    elem_id >>= 4;
                    if((NAS_IE_TYPE_GUTI_TYPE == elem_id)
                                    || (NAS_IE_TYPE_TMSI_STATUS == elem_id)
                                    || (NAS_IE_TYPE_MS_NETWORK_FEATURE_SUPPORT == elem_id))
                    {
                        switch(elem_id)
                        {
                                case NAS_IE_TYPE_GUTI_TYPE:
                                        {
                                                log_msg(LOG_DEBUG, "Old guti type : Skipping.\n");
                                                msg++;
                                        }break;
                                case NAS_IE_TYPE_TMSI_STATUS:
                                        {
                                                log_msg(LOG_DEBUG, "TMSI Status : Skipping.\n");
                                                msg++;
                                        }break;
                                case NAS_IE_TYPE_MS_NETWORK_FEATURE_SUPPORT:
                                        {
                                                log_msg(LOG_DEBUG, "MS Network feature support : Skipping.\n");
                                                msg++;
                                        }break;
                                default:
                                        log_msg(LOG_WARNING, "Unknown AVP in attach msg. %d \n",elem_id);
                                        msg++;
                        }
                        continue;
                    }
                    else
                    {
                            elem_id = msg[0];
                            switch(elem_id)
                            {
                                    case NAS_IE_TYPE_DRX_PARAM:
                                            {
                                                    log_msg(LOG_DEBUG, "DRX Param : Skipping.\n");
                                                    msg += 3;
                                            }break;
                                    case NAS_IE_TYPE_TAI:
                                            {
                                                    log_msg(LOG_DEBUG, "TAI : Skipping.\n");
                                                    msg += 6;
                                            }break;
                                    case NAS_IE_TYPE_MS_CLASSMARK_2:
                                            {
                                                    log_msg(LOG_DEBUG, "MS classmark 2 : Skipping.\n");
                                                    int len = msg[1];
                                                    msg += len + 2; //msgid + len field + len;
                                            }break;
                                    case NAS_IE_TYPE_VOICE_DOMAIN_PREF_UE_USAGE_SETTING:
                                            {
                                                    log_msg(LOG_DEBUG, "Voice domain UE Usage : Skipping.\n");
                                                    int len = msg[1];
                                                    msg += len + 2; //msgid + len field + len;
                                            }break;
                                    case NAS_IE_TYPE_MS_NETWORK_CAPABILITY:
                                            {
                                                    log_msg(LOG_DEBUG, "MS Network capability : Handling.\n");
                                                    index++;
                                                    nas->elements[index].msgType = NAS_IE_TYPE_MS_NETWORK_CAPABILITY;
                                                    nas->elements[index].pduElement.ms_network.pres = true;
                                                    nas->elements[index].pduElement.ms_network.element_id 
                                                            = msg[0];
                                                    msg++;
                                                    nas->elements[index].pduElement.ms_network.len = msg[0];
                                                    msg++;
                                                    memcpy(
                                                                    (nas->elements[index].pduElement.ms_network.capab), 
                                                                    msg, 
                                                                    nas->elements[index].pduElement.ms_network.len);
                                                    msg += 
                                                            nas->elements[index].pduElement.ms_network.len;
                                            }break;
                                    default:
                                            log_msg(LOG_WARNING, "Unknown AVP in Attach Req  %d \n", elem_id);
                                            msg++;
                            }

                            continue;
                    }
                }
                break;
        }
        case NAS_ATTACH_COMPLETE:
		{
            log_msg(LOG_INFO, "NAS_ATTACH_COMPLETE recvd\n");
            /*Other than error check there seems no information to pass to mme. Marking TODO for protocol study*/
            break;
		}
        case NAS_DETACH_REQUEST: 
        {
            log_msg(LOG_INFO, "NAS_DETACH_REQUEST recvd\n");
            nas->elements_len = 1;
            nas->elements = (nas_pdu_elements *) calloc(sizeof(nas_pdu_elements), 1);

            /*EPS mobility identity*/
            uint8_t msg_len = msg[1];
            unsigned char eps_identity = msg[2] & 0x07;
            log_msg(LOG_INFO, "NAS Detach Request Rcvd :  %d %d %d %d, eps id %d\n", msg[0],msg[1],msg[2],msg[4],eps_identity); 
            if(eps_identity == 0x06)
            {
                log_msg(LOG_INFO, "Mobile identity GUTI Rcvd \n");
                // Mobile Identity contains GUTI
                // MCC+MNC offset = 3
                // MME Group Id   = 2
                // MME Code       = 1
                // MTMSI offset from start of this AVP = 3 + 2 + 1
                nas->elements[0].msgType = NAS_IE_TYPE_EPS_MOBILE_ID_IMSI;
                memcpy(&nas->elements[0].pduElement.mi_guti.plmn_id.idx, &msg[3], 3);
                nas->elements[0].pduElement.mi_guti.mme_grp_id = ntohs(*(short int *)(&msg[6]));
                nas->elements[0].pduElement.mi_guti.mme_code = msg[8];
                nas->elements[0].pduElement.mi_guti.m_TMSI = ntohl(*((unsigned int *)(&msg[9])));
                log_msg(LOG_INFO, "NAS Detach Request Rcvd ID: GUTI. PLMN id %d %d %d \n", nas->elements[0].pduElement.mi_guti.plmn_id.idx[0], 
                                nas->elements[0].pduElement.mi_guti.plmn_id.idx[1], 
                                nas->elements[0].pduElement.mi_guti.plmn_id.idx[2] );
                log_msg(LOG_INFO, "NAS Detach Request Rcvd ID: GUTI. mme group id = %d, MME code %d  mtmsi = %d\n", 
                                nas->elements[0].pduElement.mi_guti.mme_grp_id, 
                                nas->elements[0].pduElement.mi_guti.mme_code,
                                nas->elements[0].pduElement.mi_guti.m_TMSI);
                nas->flags |= NAS_MSG_UE_IE_GUTI;
            }
            break;
        }
        case NAS_DETACH_ACCEPT: 
		{
            log_msg(LOG_INFO, "NAS_DETACH_ACCEPT recvd\n");
            break;
        }
        default:
		{
            log_msg(LOG_ERROR, "Unknown NAS Message type- 0x%x\n", nas->header.message_type);
            break;

        }
	}

    return SUCCESS;
}

void MmeNasUtils::copy_nas_to_s1msg(struct nasPDU *nas, s1_incoming_msg_data_t *s1Msg)
{
	switch(nas->header.message_type)
	{
		case NAS_ATTACH_REQUEST:
		{
			log_msg(LOG_INFO, "Copy Required details of message ATTACH REQUEST\n");
    		int nas_index = 0;
			s1Msg->msg_type = msg_type_t::attach_request;
			s1Msg->msg_data.ue_attach_info_m.enb_fd = s1Msg->msg_data.rawMsg.enodeb_fd;
			s1Msg->msg_data.ue_attach_info_m.criticality = s1Msg->msg_data.rawMsg.criticality;
	        s1Msg->msg_data.ue_attach_info_m.s1ap_enb_ue_id = s1Msg->msg_data.rawMsg.s1ap_enb_ue_id;
            memcpy(&(s1Msg->msg_data.ue_attach_info_m.tai), &(s1Msg->msg_data.rawMsg.tai), sizeof(struct TAI));
            memcpy(&(s1Msg->msg_data.ue_attach_info_m.utran_cgi), &(s1Msg->msg_data.rawMsg.utran_cgi), sizeof(struct CGI));

			while(nas_index < nas->elements_len)
            {
                log_msg(LOG_INFO, "nasIndex %d, msgType %d\n", nas_index, nas->elements[nas_index].msgType);
                s1Msg->msg_data.ue_attach_info_m.seq_no = nas->header.seq_no;
                switch(nas->elements[nas_index].msgType)
                {
                    case NAS_IE_TYPE_ESM_MSG:
                        {
                            break;
                        }
                    case NAS_IE_TYPE_EPS_MOBILE_ID_IMSI:
                        {
                            s1Msg->msg_data.ue_attach_info_m.flags = nas->flags;
                            if(UE_ID_IMSI(nas->flags))
                            { 
                                memcpy(&(s1Msg->msg_data.ue_attach_info_m.IMSI), 
                                       &(nas->elements[nas_index].pduElement.IMSI),
                                       BINARY_IMSI_LEN);
                            }
                            else if(UE_ID_GUTI(nas->flags))
                            {
                                memcpy(&(s1Msg->msg_data.ue_attach_info_m.mi_guti), &(nas->elements[nas_index].pduElement.mi_guti),
                                       sizeof(struct guti));
                            }
                            break;
                        }
                    case NAS_IE_TYPE_UE_NETWORK_CAPABILITY:
                        {
                            memcpy(&(s1Msg->msg_data.ue_attach_info_m.ue_net_capab),
                                   &(nas->\
                                     elements[nas_index].pduElement.ue_network),
                                   sizeof(struct UE_net_capab));

                            break;
                        }
                    case NAS_IE_TYPE_MS_NETWORK_CAPABILITY:
                        {
                            memcpy(&(s1Msg->msg_data.ue_attach_info_m.ms_net_capab),
                                   &(nas->\
                                     elements[nas_index].pduElement.ms_network),
                                   sizeof(struct MS_net_capab));

                            break;
                        }
                    case NAS_IE_TYPE_TX_FLAG:
                        {
                            s1Msg->msg_data.ue_attach_info_m.esm_info_tx_required =
                                nas->elements[nas_index].pduElement.esm_info_tx_required;
			log_msg(LOG_INFO, "ESM info flag %d \n", s1Msg->msg_data.ue_attach_info_m.esm_info_tx_required);
                            break;
                        }
                    case NAS_IE_TYPE_PTI:
                        {
                            s1Msg->msg_data.ue_attach_info_m.pti =
                                nas->elements[nas_index].pduElement.pti;
                            break;
                        }
                    case NAS_IE_TYPE_PCO:
                        {
                           	 memcpy(&s1Msg->msg_data.ue_attach_info_m.pco_options[0], &nas->elements[nas_index].pduElement.pco_opt.pco_options[0], nas->elements[nas_index].pduElement.pco_opt.pco_length);
							 s1Msg->msg_data.ue_attach_info_m.pco_length = nas->elements[nas_index].pduElement.pco_opt.pco_length;

                            break;
                        }
                    default:
                        {
                            log_msg(LOG_INFO, "nas element not handled\n");
                        }
                }

                nas_index++;
            }
			
			break;
		}
		case NAS_AUTH_RESP:
		{
			log_msg(LOG_INFO, "Copy Required details of message AUTH RESPONSE \n");
			s1Msg->msg_type = msg_type_t::auth_response;
			if(nas->header.message_type != NAS_AUTH_RESP)
            {
				s1Msg->msg_data.authresp_Q_msg_m.status = S1AP_AUTH_FAILED;
			}
			else
			{
                s1Msg->msg_data.authresp_Q_msg_m.status = SUCCESS;
			}
            memcpy(&(s1Msg->msg_data.authresp_Q_msg_m.res), 
            	   &(nas->elements[0].pduElement.auth_resp),
                   sizeof(struct XRES));
	    	break;
		}
		case NAS_AUTH_FAILURE:
		{
			log_msg(LOG_INFO, "Copy Required details of message AUTH FAILURE \n");
			s1Msg->msg_type = msg_type_t::auth_response;
            s1Msg->msg_data.authresp_Q_msg_m.status = S1AP_AUTH_FAILED;//Error in authentication
	        memcpy(&(s1Msg->msg_data.authresp_Q_msg_m.auts), 
                   &(nas->elements[0].pduElement.auth_fail_resp),
		                   sizeof(struct AUTS));
 
	    	break;
		}
		case NAS_SEC_MODE_COMPLETE:
		{
			log_msg(LOG_INFO, "Copy Required details of message SEC MODE COMPLETE \n");
			s1Msg->msg_type = msg_type_t::sec_mode_complete;
            if(nas->header.message_type != NAS_SEC_MODE_COMPLETE)
            {
                s1Msg->msg_data.secmode_resp_Q_msg_m.status = S1AP_SECMODE_FAILED;//Error in authentication
            }
            else
            {
                s1Msg->msg_data.secmode_resp_Q_msg_m.status = SUCCESS;
            }
	    	break;
		}
		case NAS_ESM_RESP:
		{
			log_msg(LOG_INFO, "Copy Required details of message ESM RESPONSE \n");
			s1Msg->msg_type = msg_type_t::esm_info_response;
            if(nas->header.message_type != NAS_ESM_RESP)
            {
                s1Msg->msg_data.esm_resp_Q_msg_m.status = S1AP_SECMODE_FAILED;//Error in authentication
            }
            else
            {
                s1Msg->msg_data.esm_resp_Q_msg_m.status = SUCCESS;
	            memcpy(&(s1Msg->msg_data.esm_resp_Q_msg_m.apn), &(nas->elements[0].pduElement.apn),
		               sizeof(struct apn_name));
            }
	    	break;
		}
		case NAS_ATTACH_COMPLETE:
		{
			log_msg(LOG_INFO, "Copy Required details of message ATTACH COMPLETE \n");
			s1Msg->msg_type = msg_type_t::attach_complete;
	    	break;
		}
		case NAS_SERVICE_REQUEST:
		{
			log_msg(LOG_INFO, "Copy Required details of message SERVICE REQUEST \n");
			s1Msg->msg_type = msg_type_t::service_request;
			s1Msg->msg_data.service_req_Q_msg_m.enb_fd = s1Msg->msg_data.rawMsg.enodeb_fd;
            s1Msg->msg_data.service_req_Q_msg_m.ksi = nas->header.ksi;;
            s1Msg->msg_data.service_req_Q_msg_m.seq_no = nas->header.seq_no;
            memcpy(&s1Msg->msg_data.service_req_Q_msg_m.mac, nas->header.short_mac, sizeof(uint16_t));
			break;
		}
		case NAS_DETACH_REQUEST:
		{
			log_msg(LOG_INFO, "Copy Required details of message DETACH REQUEST \n");
			s1Msg->msg_type = msg_type_t::detach_request;
		   	s1Msg->msg_data.detachReq_Q_msg_m.ue_m_tmsi = nas->elements[0].pduElement.mi_guti.m_TMSI;
			break;
		}
		case NAS_DETACH_ACCEPT:
		{
			log_msg(LOG_INFO, "Copy Required details of message DETACH ACCEPT \n");
			s1Msg->msg_type = msg_type_t::detach_accept_from_ue;
			break;
		}
		case NAS_TAU_REQUEST:
		{
			log_msg(LOG_INFO, "Copy Required details of message TAU REQUEST \n");
			s1Msg->msg_type = msg_type_t::tau_request;
			s1Msg->msg_data.tauReq_Q_msg_m.enb_fd = s1Msg->msg_data.rawMsg.enodeb_fd;
	        s1Msg->msg_data.tauReq_Q_msg_m.s1ap_enb_ue_id = s1Msg->msg_data.rawMsg.s1ap_enb_ue_id;
			//ue_idx no need to copy 
           	s1Msg->msg_data.tauReq_Q_msg_m.seq_num = nas->header.seq_no;
	    	break;
		}
		case NAS_IDENTITY_RESPONSE:
		{
			log_msg(LOG_INFO, "Copy Required details of message IDENTITY RESPONSE \n");
			s1Msg->msg_type = msg_type_t::id_response;
            if(nas->header.message_type != NAS_IDENTITY_RESPONSE)
            {
                s1Msg->msg_data.identityResp_Q_msg_m.status  = S1AP_IDENTITY_FAILED; 
            }
            else
            {
                s1Msg->msg_data.identityResp_Q_msg_m.status = SUCCESS;
            }

            memcpy(&(s1Msg->msg_data.identityResp_Q_msg_m.IMSI), 
                   &(nas->elements[0].pduElement.IMSI),
                   BINARY_IMSI_LEN);

	    	break;
		}
		default:
		{
			log_msg(LOG_INFO, "Copy Required details of message ATTACH REQUEST\n");
			s1Msg->msg_type = attach_request;
			break;
		}
	}
	return;
}

void
MmeNasUtils::get_negotiated_qos_value(struct esm_qos *qos)
{
	qos->delay_class = 1;
	qos->reliability_class = 3;
	qos->peak_throughput = 5;
	qos->precedence_class = 2;
	qos->mean_throughput = 31;
	qos->traffic_class = 3;
	qos->delivery_order = 2;
	qos->delivery_err_sdu = 3;
	qos->max_sdu_size = 140;
	qos->mbr_ul = 254;
	qos->mbr_dl = 86;
	qos->residual_ber = 7;
	qos->sdu_err_ratio = 6;
	qos->transfer_delay = 18;
	qos->trffic_prio = 3;
	qos->gbr_ul = 86;
	qos->gbr_dl = 86;
	qos->sig_ind = 0;
	qos->src_stat_desc = 0;
	qos->mbr_dl_ext = 108;
	qos->gbr_dl_ext = 0;
	qos->mbr_ul_ext = 108;
	qos->gbr_ul_ext = 0;

	return;
}

static void buffer_copy(struct Buffer *buffer, void *value, size_t size)
{
	memcpy(buffer->buf + buffer->pos , value, size);
	buffer->pos += size;
	return;
}

static int
copyU16(unsigned char *buffer, uint32_t val)
{
	if (val < 255) {
		buffer[0] = (val >> 8) & 0xFF;
		buffer[1] = val & 0xFF;
		return 2;
	} else if (val < 65535) {
		buffer[0] = 0x40;
		buffer[1] = (val >> 8) & 0xFF;
		buffer[2] = val & 0xFF;
		return 3;
	} else {
		buffer[0] = 0x80;
		buffer[1] = (val >> 16) & 0xFF;
		buffer[2] = (val >> 8) & 0xFF;
		buffer[3] = val & 0xFF;
		return 4;
	}
}

static int 
encode_network_name_ie(char* network_name, char* enc_str)
{
  uint32_t          i;
  uint32_t          bit_offset;
  uint32_t          byte_offset;
  uint32_t          name_length = strlen(network_name);

  // We put limit on the number if characters in the network name.
  assert(name_length <= 10);

  bit_offset  = 0;
  byte_offset = 2; // Min length; length  1 byte and 1 byte mandatory header field. 

  unsigned char mask_1[8] = {0xff ,0xff, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0,  0x80};
  unsigned char mask_2[8] = {0xff, 0xff, 0x01, 0x03, 0x07, 0x0f, 0x1f,  0x3f};

  for (i = 0; i < name_length; i++) {
    // check if printable character. Except some special charaters are not allowed    
    if (network_name[i] == 0x0A || network_name[i] == 0x0D || 
       (network_name[i] >= 0x20 && network_name[i] <= 0x3F) ||
       (network_name[i] >= 0x41 && network_name[i] <= 0x5A) || 
       (network_name[i] >= 0x61 && network_name[i] <= 0x7A)) {
      enc_str[byte_offset] |= ((network_name[i] << bit_offset) & mask_1[bit_offset]);
      if(bit_offset >=1) {
        byte_offset ++;
        enc_str[byte_offset] |= ((network_name[i] >>(8-bit_offset)) & mask_2[bit_offset]);
      }
      bit_offset = (bit_offset+7) % 8; 
    }
    else {
      return E_FAIL; //unsupported charater 
    }
  }
  if (0 == bit_offset) {
    enc_str[0] = byte_offset - 1;
    enc_str[1] = 0x80 ; // ci not supported 
  } else {
    enc_str[0] = byte_offset;
    enc_str[1] = 0x80 | ((8 - bit_offset) & 0x07);
  }
  return SUCCESS;
}


/* Encode NAS mesage */

void MmeNasUtils::encode_nas_msg(struct Buffer *nasBuffer, struct nasPDU *nas, Secinfo& secContext)
{
	switch(nas->header.message_type)
	{
		case AuthenticationRequest:
		{
			log_msg(LOG_DEBUG, "Encoding Authentication Request NAS message in mme-app\n");
			uint8_t value = (nas->header.security_header_type) | nas->header.proto_discriminator;
			nasBuffer->pos = 0;
			buffer_copy(nasBuffer, &value, sizeof(value));
			buffer_copy(nasBuffer, &nas->header.message_type, sizeof(nas->header.message_type));
			buffer_copy(nasBuffer, &nas->header.nas_security_param, sizeof(nas->header.nas_security_param));
			buffer_copy(nasBuffer, &nas->elements[0].pduElement.rand,
	     			sizeof(nas->elements[0].pduElement.rand));
			uint8_t datalen = 16;
			buffer_copy(nasBuffer, &datalen, sizeof(datalen));
			buffer_copy(nasBuffer, &nas->elements[1].pduElement.autn,
	   					sizeof(nas->elements[1].pduElement.autn));
			break;
		}
		case ESMInformationRequest: 
		{
			log_msg(LOG_DEBUG, "Encoding ESM information request NAS message in mme-app\n");
			nasBuffer->pos = 0;
			unsigned char value = (nas->header.security_header_type << 4 | nas->header.proto_discriminator);
			buffer_copy(nasBuffer, &value, sizeof(value));
			uint8_t mac_data_pos;
			buffer_copy(nasBuffer, &nas->header.mac, MAC_SIZE);
			mac_data_pos = nasBuffer->pos;
			buffer_copy(nasBuffer, &nas->header.seq_no, sizeof(nas->header.seq_no));
			value = (nas->header.eps_bearer_identity << 4 | EPSSessionManagementMessage);
			buffer_copy(nasBuffer, &value, sizeof(value));
			buffer_copy(nasBuffer, &nas->header.procedure_trans_identity, sizeof(nas->header.procedure_trans_identity));
			buffer_copy(nasBuffer, &nas->header.message_type, sizeof(nas->header.message_type));
			/* Calculate mac */
			uint8_t direction = 1;
			uint8_t bearer = 0;
			unsigned char int_key[NAS_INT_KEY_SIZE];
			secContext.getIntKey(&int_key[0]);
            nas_int_algo_enum int_alg = secContext.getSelectIntAlg();
			calculate_mac(int_key, nas->dl_count, direction,
						  	bearer, &nasBuffer->buf[mac_data_pos],
							nasBuffer->pos - mac_data_pos,
							&nasBuffer->buf[mac_data_pos - MAC_SIZE], int_alg);

			break;
		}
		case SecurityModeCommand: 
		{
			log_msg(LOG_DEBUG, "Encoding Security Mode Command NAS message in mme-app\n");
			nasBuffer->pos = 0;
			unsigned char value = (nas->header.security_header_type << 4 | nas->header.proto_discriminator);
			buffer_copy(nasBuffer, &value, sizeof(value));
			uint8_t mac_data_pos;
			buffer_copy(nasBuffer, &nas->header.mac, MAC_SIZE);
			mac_data_pos = nasBuffer->pos;
			buffer_copy(nasBuffer, &nas->header.seq_no, sizeof(nas->header.seq_no));
			unsigned char inner_security_header_type = Plain;
			value = inner_security_header_type | nas->header.proto_discriminator;
			buffer_copy(nasBuffer, &value, sizeof(value));
			buffer_copy(nasBuffer, &nas->header.message_type, sizeof(nas->header.message_type));
			value = (nas->header.security_encryption_algo << 4 | nas->header.security_integrity_algo);
			buffer_copy(nasBuffer, &value, sizeof(value));
			buffer_copy(nasBuffer, &nas->header.nas_security_param, sizeof(nas->header.nas_security_param));
			buffer_copy(nasBuffer, &nas->elements->pduElement.ue_network.len,
						sizeof(nas->elements->pduElement.ue_network.len));

			buffer_copy(nasBuffer, &nas->elements->pduElement.ue_network.capab,
						nas->elements->pduElement.ue_network.len);

 			/* Request IMEI from the device */
			uint8_t imei = 0xc1;
			buffer_copy(nasBuffer, &imei, sizeof(imei));
			/* Calculate mac */
			uint8_t direction = 1;
			uint8_t bearer = 0;

			unsigned char int_key[NAS_INT_KEY_SIZE];
			secContext.getIntKey(&int_key[0]);
            nas_int_algo_enum int_alg = secContext.getSelectIntAlg();
			calculate_mac(int_key, nas->dl_count,
							direction, bearer, &nasBuffer->buf[mac_data_pos],
							nasBuffer->pos - mac_data_pos,
							&nasBuffer->buf[mac_data_pos - MAC_SIZE], int_alg);

			break;
		}
		case AttachAccept: 
		{
			log_msg(LOG_DEBUG, "Encoding Attach Accept NAS message in mme-app\n");
			nasBuffer->pos = 0;
			unsigned char value = (nas->header.security_header_type << 4 | nas->header.proto_discriminator);
			buffer_copy(nasBuffer, &value, sizeof(value));
			uint8_t mac_data_pos;
			buffer_copy(nasBuffer, &nas->header.mac, MAC_SIZE);
			mac_data_pos = nasBuffer->pos;
			buffer_copy(nasBuffer, &nas->header.seq_no, sizeof(nas->header.seq_no));
			value = (Plain << 4 | nas->header.proto_discriminator);
			buffer_copy(nasBuffer, &value, sizeof(value));
			buffer_copy(nasBuffer, &nas->header.message_type, sizeof(nas->header.message_type));

			/* eps attach result */
			buffer_copy(nasBuffer, &(nas->elements[0].pduElement.attach_res), sizeof(unsigned char));

			buffer_copy(nasBuffer, &(nas->elements[1].pduElement.t3412), sizeof(unsigned char));

			/* TAI list */
			unsigned char u8value = 6;
			buffer_copy(nasBuffer, &u8value, sizeof(u8value));
			u8value = 32; /* TODO: use value from tai list */
			buffer_copy(nasBuffer, &u8value, sizeof(u8value));
			buffer_copy(nasBuffer, &(nas->elements[2].pduElement.tailist.partial_list[0].plmn_id.idx), 3);
			buffer_copy(nasBuffer, &(nas->elements[2].pduElement.tailist.partial_list[0].tac), 2);

			/* ESM container */
			unsigned char esm_len_pos = nasBuffer->pos;
			/* esm message container length */
			char tmplen[2] = {0, 0};
			buffer_copy(nasBuffer, tmplen, 2);

			/* esm message bearer id and protocol discriminator */
			u8value = (nas->elements[3].pduElement.esm_msg.eps_bearer_id << 4 | nas->elements[3].pduElement.esm_msg.proto_discriminator);
			buffer_copy(nasBuffer, &u8value, sizeof(u8value));

			/* esm message procedure identity */
			buffer_copy(nasBuffer, &(nas->elements[3].pduElement.esm_msg.procedure_trans_identity), sizeof(u8value));

			/* esm message session management message */
			buffer_copy(nasBuffer, &(nas->elements[3].pduElement.esm_msg.session_management_msgs), sizeof(u8value));

			/* eps qos */
			uint8_t datalen = 1;
			buffer_copy(nasBuffer, &datalen, sizeof(datalen));
			buffer_copy(nasBuffer, &(nas->elements[3].pduElement.esm_msg.eps_qos), sizeof(datalen));

			/* apn */
			// There is one category of UE, they do not send not apn to MME.
			// In this case, the apn length from esm will be 0.
			// Then MME will use the selected apn name from HSS-DB.
			if (nas->elements[3].pduElement.esm_msg.apn.len == 0 ) {
#if 0
				datalen = g_icsReqInfo->selected_apn.len + 1;
				buffer_copy(nasBuffer, &datalen, sizeof(datalen));
				buffer_copy(nasBuffer, g_icsReqInfo->selected_apn.val, g_icsReqInfo->selected_apn.len);
#endif

			}else {
				// Return the same apn sent by UE
				datalen = nas->elements[3].pduElement.esm_msg.apn.len;
				buffer_copy(nasBuffer, &datalen, sizeof(datalen));
				buffer_copy(nasBuffer, (char *)nas->elements[3].pduElement.esm_msg.apn.val, datalen);
			}

			/* pdn address */
			datalen = 5; //sizeof(ies[3].esm_msg.pdn_addr);
			buffer_copy(nasBuffer, &datalen, sizeof(datalen));
			u8value = 1;
			buffer_copy(nasBuffer, &u8value, sizeof(u8value));
			//buffer_copy(&g_ics_buffer, &(ies[3].esm_msg.pdn_addr.pdn_type), 1);
			buffer_copy(nasBuffer, &(nas->elements[3].pduElement.esm_msg.pdn_addr.ipv4), datalen-1);

			/* linked ti */
			u8value = 0x5d; /* element id TODO: define macro or enum */
			buffer_copy(nasBuffer, &u8value, sizeof(u8value));
			datalen = 1;//sizeof(ies[3].esm_msg.linked_ti);
			buffer_copy(nasBuffer, &datalen, sizeof(datalen));
			buffer_copy(nasBuffer, &(nas->elements[3].pduElement.esm_msg.linked_ti), datalen);

			/* negotiated qos */
			u8value = 0x30; /* element id TODO: define macro or enum */
			buffer_copy(nasBuffer, &u8value, sizeof(u8value));
			datalen = 16;//sizeof(ies[3].esm_msg.negotiated_qos);
			buffer_copy(nasBuffer, &datalen, sizeof(datalen));
			buffer_copy(nasBuffer, &(nas->elements[3].pduElement.esm_msg.negotiated_qos), datalen);

			/* apn ambr */
			/* TODO: remove hardcoded values of apn ambr */
			unsigned char apn_ambr[8] = {0x5e, 0x06, 0x80, 0x00, 0x04, 0x05, 0x06, 0x07};
			buffer_copy(nasBuffer, apn_ambr, 8);

			u8value = 0x27; /* element id TODO: define macro or enum */
			buffer_copy(nasBuffer, &u8value, sizeof(u8value));
			uint8_t pco_length = nas->elements[3].pduElement.esm_msg.pco_opt.pco_length;
			buffer_copy(nasBuffer, &(nas->elements[3].pduElement.esm_msg.pco_opt.pco_length), sizeof(pco_length));
			buffer_copy(nasBuffer, &nas->elements[3].pduElement.esm_msg.pco_opt.pco_options[0], pco_length);
			/* ESM message container end */

			/* Copy esm container length to esm container length field */
			uint16_t esm_datalen = nasBuffer->pos - esm_len_pos - 2;
			unsigned char esm_len[2];
			copyU16(esm_len, esm_datalen);
			nasBuffer->buf[esm_len_pos] = esm_len[0];
			nasBuffer->buf[esm_len_pos + 1] = esm_len[1];

			/* EPS mobile identity GUTI */
			u8value = 0x50; /* element id TODO: define macro or enum */
			buffer_copy(nasBuffer, &u8value, sizeof(u8value));
			datalen = 11;
			buffer_copy(nasBuffer, &datalen, sizeof(datalen));

			u8value = 246; /* TODO: remove hard coding */
			buffer_copy(nasBuffer, &u8value, sizeof(u8value));
			buffer_copy(nasBuffer, &(nas->elements[4].pduElement.mi_guti.plmn_id.idx), 3);
			buffer_copy(nasBuffer, &(nas->elements[4].pduElement.mi_guti.mme_grp_id),
					sizeof(nas->elements[4].pduElement.mi_guti.mme_grp_id));
			buffer_copy(nasBuffer, &(nas->elements[4].pduElement.mi_guti.mme_code),
					sizeof(nas->elements[4].pduElement.mi_guti.mme_code));
			buffer_copy(nasBuffer, &(nas->elements[4].pduElement.mi_guti.m_TMSI),
					sizeof(nas->elements[4].pduElement.mi_guti.m_TMSI));

			/* Calculate mac */
			uint8_t direction = 1;
			uint8_t bearer = 0;
			unsigned char int_key[NAS_INT_KEY_SIZE];
			secContext.getIntKey(&int_key[0]);
            nas_int_algo_enum int_alg = secContext.getSelectIntAlg();
			calculate_mac(int_key, nas->dl_count, direction,
						  	bearer, &nasBuffer->buf[mac_data_pos],
							nasBuffer->pos - mac_data_pos,
							&nasBuffer->buf[mac_data_pos - MAC_SIZE], int_alg);

			break;
		}
		case EMMInformation:
		{
			log_msg(LOG_DEBUG, "Encoding EMM informaton NAS message in mme-app\n");
    		nasBuffer->pos = 0; 
    		unsigned char nas_sec_hdr[1] = { 0x27}; 
    		buffer_copy(nasBuffer, nas_sec_hdr, 1);
    		uint8_t mac_data_pos;
    		char mac[4] = { 0x00, 0x00, 0x00, 0x00}; 
    		buffer_copy(nasBuffer, mac, 4);
    		mac_data_pos = nasBuffer->pos;

			buffer_copy(nasBuffer, &nas->header.seq_no, sizeof(nas->header.seq_no));

			unsigned char value = (Plain << 4 | nas->header.proto_discriminator);
			buffer_copy(nasBuffer, &value, sizeof(value));
			buffer_copy(nasBuffer, &nas->header.message_type, sizeof(nas->header.message_type));
   
    		char bufBig[128] = {'\0'};
    		bufBig[0] = 0x43;
    		encode_network_name_ie((char *)nas->elements[0].pduElement.apn.val, &bufBig[1]);
    		buffer_copy(nasBuffer, bufBig, bufBig[1] + 2);

    		char bufShort[20] = {'\0'};
    		bufShort[0] = 0x45;
    		encode_network_name_ie((char *)nas->elements[0].pduElement.apn.val, &bufShort[1]);
    		buffer_copy(nasBuffer, bufShort, bufShort[1] + 2);
    
    		/* Calculate mac */
    		uint8_t direction = 1;
    		uint8_t bearer = 0;
			unsigned char int_key[NAS_INT_KEY_SIZE];
			secContext.getIntKey(&int_key[0]);
            nas_int_algo_enum int_alg = secContext.getSelectIntAlg();
    		calculate_mac(int_key, nas->dl_count, direction,
		    	bearer, &nasBuffer->buf[mac_data_pos],
		    	nasBuffer->pos - mac_data_pos,
		    	&nasBuffer->buf[mac_data_pos - MAC_SIZE], int_alg);

			break;
		}
		case IdentityRequest:
		{
			log_msg(LOG_DEBUG, "Encoding Identity Request NAS message in mme-app\n");
			uint8_t value = (nas->header.security_header_type) | nas->header.proto_discriminator;
			nasBuffer->pos = 0;
			buffer_copy(nasBuffer, &value, sizeof(value));
			buffer_copy(nasBuffer, &nas->header.message_type, sizeof(nas->header.message_type));
			buffer_copy(nasBuffer, &(nas->elements[0].pduElement.ue_id_type), sizeof(unsigned char));
			break;
		}
		case DetachAccept:
		{
			log_msg(LOG_DEBUG, "Encoding detach Accept NAS message in mme-app\n");
			nasBuffer->pos = 0;
			unsigned char value = (nas->header.security_header_type << 4 | nas->header.proto_discriminator);
			buffer_copy(nasBuffer, &value, sizeof(value));
			uint8_t mac_data_pos;
			buffer_copy(nasBuffer, &nas->header.mac, MAC_SIZE);
			mac_data_pos = nasBuffer->pos;
			buffer_copy(nasBuffer, &nas->header.seq_no, sizeof(nas->header.seq_no));
			value = (Plain << 4 | nas->header.proto_discriminator);
			buffer_copy(nasBuffer, &value, sizeof(value));
			buffer_copy(nasBuffer, &nas->header.message_type, sizeof(nas->header.message_type));
			/* Calculate mac */
			uint8_t direction = 1;
			uint8_t bearer = 0;
			unsigned char int_key[NAS_INT_KEY_SIZE];
			secContext.getIntKey(&int_key[0]);
            nas_int_algo_enum int_alg = secContext.getSelectIntAlg();
			calculate_mac(int_key, nas->dl_count, direction,
						  	bearer, &nasBuffer->buf[mac_data_pos],
							nasBuffer->pos - mac_data_pos,
							&nasBuffer->buf[mac_data_pos - MAC_SIZE], int_alg);

			
			break;
		}
		case DetachRequest:
		{
			log_msg(LOG_DEBUG, "Encoding detach request NAS message in mme-app\n");
			nasBuffer->pos = 0;
			unsigned char value = (nas->header.security_header_type << 4 | nas->header.proto_discriminator);
			buffer_copy(nasBuffer, &value, sizeof(value));
			uint8_t mac_data_pos;
			buffer_copy(nasBuffer, &nas->header.mac, MAC_SIZE);
			mac_data_pos = nasBuffer->pos;
			buffer_copy(nasBuffer, &nas->header.seq_no, sizeof(nas->header.seq_no));
			value = (Plain << 4 | nas->header.proto_discriminator);
			buffer_copy(nasBuffer, &value, sizeof(value));
			buffer_copy(nasBuffer, &nas->header.message_type, sizeof(nas->header.message_type));
			value = nas->header.detach_type; 
			buffer_copy(nasBuffer, &value, sizeof(value));
			if(nas->header.emm_cause > 0)
			{
			    uint8_t IEI = NAS_IE_TYPE_EMM_CAUSE;
			    uint16_t u16value = ((uint16_t)nas->header.emm_cause << 8) | (IEI);
			    buffer_copy(nasBuffer, &u16value, sizeof(u16value));
			}
			/* Calculate mac */
			uint8_t direction = 1;
			uint8_t bearer = 0;
			unsigned char int_key[NAS_INT_KEY_SIZE];
			secContext.getIntKey(&int_key[0]);
            nas_int_algo_enum int_alg = secContext.getSelectIntAlg();
			calculate_mac(int_key, nas->dl_count, direction,
						  	bearer, &nasBuffer->buf[mac_data_pos],
							nasBuffer->pos - mac_data_pos,
							&nasBuffer->buf[mac_data_pos - MAC_SIZE], int_alg);
			break;
		}
		case AttachReject:
		{
			log_msg(LOG_DEBUG, "Encoding Attach Reject NAS message in mme-app\n");
			uint8_t value = (nas->header.security_header_type) | nas->header.proto_discriminator;
			nasBuffer->pos = 0;
			buffer_copy(nasBuffer, &value, sizeof(value));
			buffer_copy(nasBuffer, &nas->header.message_type, sizeof(nas->header.message_type));
			buffer_copy(nasBuffer, &(nas->elements[0].pduElement.attach_res), sizeof(unsigned char));
			break;
		}
		case ServiceReject:
		{
			log_msg(LOG_DEBUG, "Encoding Service Reject NAS message in mme-app\n");
			uint8_t value = (nas->header.security_header_type) | nas->header.proto_discriminator;
			nasBuffer->pos = 0;
			buffer_copy(nasBuffer, &value, sizeof(value));
			buffer_copy(nasBuffer, &nas->header.message_type, sizeof(nas->header.message_type));
			buffer_copy(nasBuffer, &(nas->elements[0].pduElement.attach_res), sizeof(unsigned char));
			break;
		}
		case TauAccept:
		{
			log_msg(LOG_DEBUG, "Encoding TAU Accept NAS message in mme-app\n");
			nasBuffer->pos = 0;
			unsigned char value = (nas->header.security_header_type << 4 | nas->header.proto_discriminator);
			buffer_copy(nasBuffer, &value, sizeof(value));
			uint8_t mac_data_pos;
			buffer_copy(nasBuffer, &nas->header.mac, MAC_SIZE);
			mac_data_pos = nasBuffer->pos;
			buffer_copy(nasBuffer, &nas->header.seq_no, sizeof(nas->header.seq_no));
			value = (Plain << 4 | nas->header.proto_discriminator);
			buffer_copy(nasBuffer, &value, sizeof(value));
			buffer_copy(nasBuffer, &nas->header.message_type, sizeof(nas->header.message_type));

			/* I really want to make a loop here and not manual addition  */

			/* add NAS IE - eps update result */
			value = nas->elements[0].pduElement.eps_update_result = 0;
			buffer_copy(nasBuffer, &value, sizeof(value));

			/* add t3412 timer IE - eps update result */
			uint8_t u8value = 0x5a; 
			buffer_copy(nasBuffer, &u8value, sizeof(u8value));
			u8value = 0x21; /* Per Mins TAU */ 
			buffer_copy(nasBuffer, &u8value, sizeof(u8value));

    		/* adding nas IE GUTI */
			u8value = 0x50; /* element id TODO: define macro or enum */
			buffer_copy(nasBuffer, &u8value, sizeof(u8value));
			uint8_t datalen = 11;
			buffer_copy(nasBuffer, &datalen, sizeof(datalen));

			u8value = 246; /* TODO: remove hard coding */
			buffer_copy(nasBuffer, &u8value, sizeof(u8value));
			buffer_copy(nasBuffer, &(nas->elements[3].pduElement.mi_guti.plmn_id.idx), 3);
			buffer_copy(nasBuffer, &(nas->elements[3].pduElement.mi_guti.mme_grp_id),
					sizeof(nas->elements[3].pduElement.mi_guti.mme_grp_id));
			buffer_copy(nasBuffer, &(nas->elements[3].pduElement.mi_guti.mme_code),
					sizeof(nas->elements[3].pduElement.mi_guti.mme_code));
			buffer_copy(nasBuffer, &(nas->elements[3].pduElement.mi_guti.m_TMSI),
					sizeof(nas->elements[3].pduElement.mi_guti.m_TMSI));
			
#if 0
			/* add MS identity - MTMSI */
			u8value = 0x23; /* element id TODO: define macro or enum */
			buffer_copy(nasBuffer, &u8value, sizeof(u8value));
			datalen = 0x05;
			buffer_copy(nasBuffer, &datalen, sizeof(datalen));
    		u8value = 0xf4; 
			buffer_copy(nasBuffer, &u8value, sizeof(u8value));
    		mtmsi = htonl(g_tauRespInfo->ue_idx); 
			buffer_copy(nasBuffer, &(mtmsi), sizeof(mtmsi));
#endif

			/* Calculate mac */
			uint8_t direction = 1;
			uint8_t bearer = 0;
			unsigned char int_key[NAS_INT_KEY_SIZE];
			secContext.getIntKey(&int_key[0]);
            nas_int_algo_enum int_alg = secContext.getSelectIntAlg();
			calculate_mac(int_key, nas->dl_count, direction,
						  	bearer, &nasBuffer->buf[mac_data_pos],
							nasBuffer->pos - mac_data_pos,
							&nasBuffer->buf[mac_data_pos - MAC_SIZE], int_alg);
			break;
		} 

		default:
			log_msg(LOG_DEBUG, "Encoding Authentication Request NAS message in mme-app\n");
	}
}
