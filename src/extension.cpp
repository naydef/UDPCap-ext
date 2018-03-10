/**
* vim: set ts=4 :
* =============================================================================
* SourceMod Sample Extension
* Copyright (C) 2004-2008 AlliedModders LLC.  All rights reserved.
* =============================================================================
*
* This program is free software; you can redistribute it and/or modify it under
* the terms of the GNU General Public License, version 3.0, as published by the
* Free Software Foundation.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
* details.
*
* You should have received a copy of the GNU General Public License along with
* this program.  If not, see <http://www.gnu.org/licenses/>.
*
* As a special exception, AlliedModders LLC gives you permission to link the
* code of this program (as well as its derivative works) to "Half-Life 2," the
* "Source Engine," the "SourcePawn JIT," and any Game MODs that run on software
* by the Valve Corporation.  You must obey the GNU General Public License in
* all respects for all other code used.  Additionally, AlliedModders LLC grants
* this exception to all derivative works.  AlliedModders LLC defines further
* exceptions, found in LICENSE.txt (as of this writing, version JULY-31-2007),
* or <http://www.sourcemod.net/license.php>.
*
* Version: $Id$
*/

#include "extension.h"
#include "inetchannel.h"
#include "fastdefs.h"
#include "windows.h"

/**
* @file extension.cpp
* @brief Implement extension code here.
*/

UDPCap g_Sample;		/**< Global singleton for extension's main interface */

SMEXT_LINK(&g_Sample);

INetChannel* netchannelptr=nullptr;
CDetour *g_pDetourProcessPacket=nullptr;
CDetour *g_pDetourSendPacket=nullptr;
IForward *g_pProcessPacketIn = nullptr;
IForward *g_pProcessPacketOut = nullptr;
IGameConfig *g_pGameConf = nullptr;

DETOUR_DECL_MEMBER1(Detour_ProcessPacketIn, bool, netpacket_t*, packet)
{
	if(!g_pProcessPacketIn)
	{
		return DETOUR_MEMBER_CALL(Detour_ProcessPacketIn)(packet); // This is not supposed to happen normally. Generally speaking, don't mess with destiny.
	}
	cell_t result=0;

	g_pProcessPacketIn->PushString(packet->from.ToString());
	g_pProcessPacketIn->PushCell(packet->from.port);
	g_pProcessPacketIn->PushString((char*)packet->data);
	g_pProcessPacketIn->PushCell(packet->size);
	g_pProcessPacketIn->PushCell(packet->from.type);

	g_pProcessPacketIn->Execute(&result);

	if(result==Pl_Handled)
	{
		return 1;
	}
	return DETOUR_MEMBER_CALL(Detour_ProcessPacketIn)(packet);
}

DETOUR_DECL_STATIC5(Detour_ProcessPacketOut, int, void*, netchannel, int, socks, const netadr_t&, to, unsigned char*, data, void*, length)
{
	if(!g_pProcessPacketOut)
	{
		return DETOUR_STATIC_CALL(Detour_ProcessPacketOut)(netchannel, socks, to, data); // This is not supposed to happen normally. Generally speaking, don't mess with destiny.
	}
	cell_t result=0;

	g_pProcessPacketOut->Execute(&result);

	if(result==Pl_Handled)
	{
		return 1;
	}
	return DETOUR_STATIC_CALL(Detour_ProcessPacketOut)(netchannel, socks, to, data);
}

bool UDPCap::SDK_OnLoad(char *error, size_t maxlength, bool late)
{
	if(!gameconfs->LoadGameConfigFile("udp-cap", &g_pGameConf, error, maxlength))
	{
		return false;
	}
	SM_GET_IFACE(MEMORYUTILS, memutils);
	CDetourManager::Init(g_pSM->GetScriptingEngine(), g_pGameConf);

	g_pDetourProcessPacket=DETOUR_CREATE_MEMBER(Detour_ProcessPacketIn, "CBaseServer::ProcessConnectionlessPacket");

	if(g_pDetourProcessPacket==nullptr)
	{
		snprintf(error, maxlength, "Error loading detour ProcessPacket!");
		return false;
	}

	g_pDetourSendPacket=DETOUR_CREATE_STATIC(Detour_ProcessPacketOut, "NET_SendPacket");

	if(g_pDetourSendPacket==nullptr)
	{
		snprintf(error, maxlength, "Error loading detour NET_SendPacket!");
		return false;
	}

	g_pDetourProcessPacket->EnableDetour();
	g_pDetourSendPacket->EnableDetour();

	g_pProcessPacketIn = forwards->CreateForward("UDPC_ProcessInboundPacket", ET_Event, 5, NULL, Param_String, Param_Cell, Param_String, Param_Cell, Param_Cell);
	g_pProcessPacketOut = forwards->CreateForward("UDPC_ProcessOutboundPacket", ET_Event, 5, NULL, Param_String, Param_Cell, Param_String, Param_Cell, Param_Cell);

	return true;
}

void UDPCap::SDK_OnUnload()
{
	g_pDetourProcessPacket->Destroy();
	gameconfs->CloseGameConfigFile(g_pGameConf);
	forwards->ReleaseForward(g_pProcessPacketIn);
	forwards->ReleaseForward(g_pProcessPacketOut);
}
