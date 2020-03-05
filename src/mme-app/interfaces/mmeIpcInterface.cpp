/*
 * Copyright (c) 2019, Infosys Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <interfaces/mmeIpcInterface.h>
#include <blockingCircularFifo.h>
#include <ipcTypes.h>
#include <tipcSocket.h>
#include <tipcTypes.h>
#include <msgBuffer.h>
#include <mme_app.h>
#include <msgHandlers/gtpMsgHandler.h>
#include <msgHandlers/s1MsgHandler.h>
#include <msgHandlers/s6MsgHandler.h>

extern "C" {
	#include "log.h"
}

using namespace cmn::ipc;
using namespace cmn::utils;

extern BlockingCircularFifo<MsgBuffer, fifoQSize_c> mmeIpcEgressFifo_g;

MmeIpcInterface::MmeIpcInterface():sender_mp(), reader_mp()
{

}

MmeIpcInterface::~MmeIpcInterface()
{
	teardown();
}

bool MmeIpcInterface::setup()
{

	// INIT socket for receiving ipc message
	TipcSocket* receiver_sock = new TipcSocket();

	IpcAddress myAddress;
	myAddress.u32 = mmeAppInstanceNum_c;

	if (receiver_sock->bindTipcSocket(myAddress) == false)
	{
		log_msg(LOG_ERROR, "MmeIpcInterface Setup Failed!!!\n");

		delete receiver_sock;

		return false;
	}

	// INIT socket for sending ipc message
	TipcSocket* sender_sock = new TipcSocket();

	sender_mp = sender_sock;
	reader_mp = receiver_sock;

	return true;
}

void MmeIpcInterface::teardown()
{
	if (sender_mp != NULL)
		delete sender_mp;

	if (reader_mp != NULL)
		delete reader_mp;
}

cmn::ipc::IpcChannel* MmeIpcInterface::sender()
{
	if (sender_mp != NULL)
		return sender_mp;
	else
		return NULL;
}

cmn::ipc::IpcChannel* MmeIpcInterface::reader()
{
	if (reader_mp != NULL)
		return reader_mp;
	else
		return NULL;
}

void MmeIpcInterface::handleIpcMsg(MsgBuffer* msgBuf)
{
	uint32_t srcAddr, destAddr;

	msgBuf->readUint32(destAddr);
	msgBuf->readUint32(srcAddr);

	log_msg(LOG_INFO, "IPC Message from src %u to dest %u\n", srcAddr, destAddr);

	switch (srcAddr)

	{
	case TipcInstanceTypes::s1apAppInstanceNum_c:
		S1MsgHandler::Instance()->handleS1Message_v(msgBuf);
		break;
	case TipcInstanceTypes::s11AppInstanceNum_c:
		GtpMsgHandler::Instance()->handleGtpMessage_v(msgBuf);
		break;
	case TipcInstanceTypes::s6AppInstanceNum_c:
		S6MsgHandler::Instance()->handleS6Message_v(msgBuf);
		break;
	default:
		log_msg(LOG_INFO, "IPC Message from unsupported instance\n");
	}
}

bool MmeIpcInterface::dispatchIpcMsg(char* buf, uint32_t len, cmn::ipc::IpcAddress& destAddr)
{
	cmn::ipc::IpcMsgHeader msgHeader;
	msgHeader.srcAddr.u32 = TipcInstanceTypes::mmeAppInstanceNum_c;
	msgHeader.destAddr.u32 = destAddr.u32;

	MsgBuffer *msgBuf = new MsgBuffer(len + sizeof(cmn::ipc::IpcMsgHeader));
	msgBuf->writeUint32(msgHeader.destAddr.u32);
        msgBuf->writeUint32(msgHeader.srcAddr.u32);
	msgBuf->writeBytes((uint8_t*)buf, len);

	log_msg(LOG_INFO, "Dispatch IPC msg\n");

	return mmeIpcEgressFifo_g.push(msgBuf);
}

bool MmeIpcInterface::dispatchIpcMsg(cmn::utils::MsgBuffer* msgBuf, cmn::ipc::IpcAddress& destAddr)
{
	cmn::ipc::IpcMsgHeader msgHeader;
	msgHeader.srcAddr.u32 = TipcInstanceTypes::mmeAppInstanceNum_c;
	msgHeader.destAddr.u32 = destAddr.u32;

	msgBuf->rewind();
	msgBuf->writeUint32(msgHeader.destAddr.u32, false);
	msgBuf->writeUint32(msgHeader.srcAddr.u32, false);

	return mmeIpcEgressFifo_g.push(msgBuf);
}

