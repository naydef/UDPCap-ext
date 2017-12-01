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
CDetour *g_pDetour=nullptr;
IForward *g_pProcessPacket = nullptr;
IGameConfig *g_pGameConf = nullptr;


/*
#define WINDOWS_SIGNATURE "\x55\x8B\xEC\x81\xEC\x50\x0A"
#define WINDOWS_SIG_LENGTH 7

#define LINUX_SIGNATURE "_ZN11CBaseServer27ProcessConnectionlessPacketEP11netpacket_s"

#define DETOUR_CREATE_STATIC_PTR(name, ptr) CDetourManager::CreateDetour(GET_STATIC_CALLBACK(name), GET_STATIC_TRAMPOLINE(name), ptr);
#define DETOUR_CREATE_MEMBER_PTR(name, ptr) CDetourManager::CreateDetour(GET_MEMBER_CALLBACK(name), GET_MEMBER_TRAMPOLINE(name), ptr);
*/

DETOUR_DECL_MEMBER1(Detour_ProcessPacket, bool, netpacket_t*, packet)
{
    if(!g_pProcessPacket)
    {
        return DETOUR_MEMBER_CALL(Detour_ProcessPacket)(packet); // This is not supposed to happen normally. Generally speaking, don't mess with destiny.
    }
    cell_t result=0;

    g_pProcessPacket->PushString(packet->from.ToString());
    g_pProcessPacket->PushCell(packet->from.port);
    g_pProcessPacket->PushString((char*)packet->data);
    g_pProcessPacket->PushCell(packet->size);
    g_pProcessPacket->PushCell(packet->from.type);

    g_pProcessPacket->Execute(&result);

    if(result==Pl_Handled)
    {
        return 1;
    }
    return DETOUR_MEMBER_CALL(Detour_ProcessPacket)(packet);
}

bool UDPCap::SDK_OnLoad(char *error, size_t maxlength, bool late)
{
    if(!gameconfs->LoadGameConfigFile("udp-cap", &g_pGameConf, error, maxlength))
    {
        return false;
    }
    SM_GET_IFACE(MEMORYUTILS, memutils);
	CDetourManager::Init(g_pSM->GetScriptingEngine(), g_pGameConf);

	/*
    //Yea, this is copypasted from Console Cleaner extension
#ifdef PLATFORM_WINDOWS
	HMODULE tier0 = GetModuleHandle("engine.dll");
	void * fn = memutils->FindPattern(tier0, WINDOWS_SIGNATURE, WINDOWS_SIG_LENGTH);
#elif defined PLATFORM_LINUX
	void * tier0 = dlopen("engine_srv.so", RTLD_NOW);
	void * fn = memutils->ResolveSymbol(tier0, LINUX_SIGNATURE);
#endif
    */
    g_pDetour=DETOUR_CREATE_MEMBER(Detour_ProcessPacket, "CBaseServer::ProcessConnectionlessPacket");

    if(g_pDetour==nullptr)
	{
		snprintf(error, maxlength, "Error loading detour ProcessPacket!");
		return false;
	}
	g_pDetour->EnableDetour();
    g_pProcessPacket = forwards->CreateForward("UDPC_ProcessIncomingPacket", ET_Event, 5, NULL, Param_String, Param_Cell, Param_String, Param_Cell, Param_Cell);

	return true;
}

void UDPCap::SDK_OnUnload()
{
    g_pDetour->Destroy();
    gameconfs->CloseGameConfigFile(g_pGameConf);
    forwards->ReleaseForward(g_pProcessPacket);
}
