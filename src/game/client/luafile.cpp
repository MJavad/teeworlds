/* (c) MAP94. See www.n-lvl.com/ndc/nclient/ for more information. */
/*DGI:Doc-Gen-Info*/
/*DGI:Type:Client*/
/*DGI:Exception:errorfunc*/
/*DGI:Event:OnChat*/
/*DGI:Event:OnStateChange*/
/*DGI:Event:OnControlChange*/
/*DGI:Event:OnRenderLevelItem1*/
/*DGI:Event:OnRenderLevelItem2*/
/*DGI:Event:OnKill*/
/*DGI:Event:OnRenderLevel1*/
/*DGI:Event:OnRenderLevel2*/
/*DGI:Event:OnRenderLevel3*/
/*DGI:Event:OnRenderLevel4*/
/*DGI:Event:OnRenderLevel5*/
/*DGI:Event:OnRenderBackground*/
/*DGI:Event:OnServerBrowserGameTypeRender*/
/*DGI:Event:OnScoreboardRender*/
/*DGI:Event:OnKeyEvent*/
/*DGI:Event:OnNetData*/
/*DGI:Event:OnConsoleRemote*/
/*DGI:Event:OnConsole*/
/*DGI:Event:OnMusicChange*/
#include <time.h>
#include <base/math.h>

#include "lua.h"
#include "components/flow.h"
#include "components/particles.h"

#include <game/generated/client_data.h>

#include <engine/serverbrowser.h>
#include <engine/textrender.h>
#include <engine/sound.h>
#include <engine/graphics.h>
#include <engine/storage.h>
#include <engine/shared/throttle.h>
#include <engine/external/zlib/zlib.h>

#include <game/client/lineinput.h>
#include <game/client/components/menus.h>
#include <game/client/components/chat.h>
#include <game/client/components/countryflags.h>
#include <game/client/components/skins.h>
#include <game/client/components/sounds.h>
#include <game/client/components/stats.h>
#include <game/client/components/damageind.h>
#include <game/client/components/console.h>

#include <game/luaglobal.h>


CLuaFile::CLuaFile()
{
    mem_zero(this, sizeof(CLuaFile));
    Close();
}

CLuaFile::~CLuaFile()
{
    End();
#ifndef CONF_PLATFORM_MACOSX
    if (m_pLua) //there are some problems with this on mac osx
        lua_close(m_pLua);
    m_pLua = 0;
#endif
}

void CLuaFile::UiTick()
{
    if (!g_Config.m_ClLua)
        return;
    for (int i = 0; i < LUAMAXUIELEMENTS; i++)
    {
        if (m_aUiElements[i].m_Used)
        {
            m_aUiElements[i].Tick();
        }
    }
}

void CLuaFile::Tick()
{
    if (!g_Config.m_ClLua)
        return;
    if (m_Error)
        return;

    ErrorFunc(m_pLua);
    m_pLuaShared->Tick();

    FunctionPrepare("Tick");
    PushInteger((int)(time_get() * 1000 / time_freq())); //time in ms
    PushInteger(m_pClient->GetPredictedTick());
    FunctionExec();
}

void CLuaFile::End()
{
    if (m_pLua == 0)
        return;

    //try to call the atexit function
    //Maybe the lua file need to save data eg. a ConfigFile
    FunctionExec("atexit");

    //free everything

    m_pLuaHandler->m_pEventListener->RemoveAllEventListeners(this);

    for (array<int>::range r = m_lTextures.all(); !r.empty(); r.pop_front())
    {
        if (g_pData->m_aImages[IMAGE_GAME].m_Id == r.front())
            g_pData->m_aImages[IMAGE_GAME].m_Id = m_pLuaHandler->m_OriginalGameTexture;
        m_pClient->Graphics()->UnloadTexture(r.front());
    }
    m_lTextures.clear();
    m_pLuaShared->Clear();

    //clear
    mem_zero(m_aUiElements, sizeof(m_aUiElements));
    mem_zero(m_aTitle, sizeof(m_aTitle));
    mem_zero(m_aInfo, sizeof(m_aInfo));
    mem_zero(m_aFilename, sizeof(m_aFilename));
    m_HaveSettings = 0;
    m_FunctionVarNum = 0;
}

int CLuaFile::Panic(lua_State *L)
{
    dbg_break();
    return 0;
}

void CLuaFile::Init(const char *pFile)
{
    //close first
    Close();
    //init ui
    for (int i = 0; i < LUAMAXUIELEMENTS; i++)
        m_aUiElements[i].m_pClient = m_pClient;

    if (pFile)
        str_copy(m_aFilename, pFile, sizeof(m_aFilename));

    m_pLua = luaL_newstate();
    luaL_openlibs(m_pLua);

    lua_atpanic(m_pLua, &Panic);

    //include
    lua_register(m_pLua, ToLower("Include"), this->Include);
    luaL_dostring(m_pLua, "package.path = \"./?;./lua/?.lua;./lua/lib/?.lua;./lua/lib/socket/?.lua\"\n");

    //Settings
    lua_register(m_pLua, ToLower("SetScriptUseSettingPage"), this->SetScriptUseSettingPage);
    lua_register(m_pLua, ToLower("SetScriptTitle"), this->SetScriptTitle);
    lua_register(m_pLua, ToLower("SetScriptInfo"), this->SetScriptInfo);

    //Eventlistener stuff
    lua_register(m_pLua, ToLower("AddEventListener"), this->AddEventListener);
    lua_register(m_pLua, ToLower("RemoveEventListener"), this->RemoveEventListener);

    //menu
    lua_register(m_pLua, ToLower("MenuActive"), this->MenuActive);
    lua_register(m_pLua, ToLower("MenuGameActive"), this->MenuGameActive);
    lua_register(m_pLua, ToLower("MenuPlayersActive"), this->MenuPlayersActive);
    lua_register(m_pLua, ToLower("MenuServerInfoActive"), this->MenuServerInfoActive);
    lua_register(m_pLua, ToLower("MenuCallVoteActive"), this->MenuCallVoteActive);
    lua_register(m_pLua, ToLower("MenuServersActive"), this->MenuServersActive);
    lua_register(m_pLua, ToLower("MenuMusicActive"), this->MenuMusicActive);
    lua_register(m_pLua, ToLower("MenuDemosActive"), this->MenuDemosActive);

    //mouse and keyboard"), this->
    lua_register(m_pLua, ToLower("GetMousePosMenu"), this->GetMousePosMenu);
    lua_register(m_pLua, ToLower("SetMouseModeRelative"), this->SetMouseModeRelative);
    lua_register(m_pLua, ToLower("SetMouseModeRelativ"), this->SetMouseModeRelative); //pwnd by language
    lua_register(m_pLua, ToLower("SetMouseModeAbsolute"), this->SetMouseModeAbsolute);

    //sendinfo
    lua_register(m_pLua, ToLower("SendPlayerInfo"), this->SendPlayerInfo);

    //Player
    lua_register(m_pLua, ToLower("GetPlayerName"), this->GetPlayerName);
    lua_register(m_pLua, ToLower("GetPlayerClan"), this->GetPlayerClan);
    lua_register(m_pLua, ToLower("GetPlayerCountry"), this->GetPlayerCountry);
    lua_register(m_pLua, ToLower("GetPlayerScore"), this->GetPlayerScore);
    lua_register(m_pLua, ToLower("GetPlayerPing"), this->GetPlayerPing);
    lua_register(m_pLua, ToLower("GetPlayerTeam"), this->GetPlayerTeam);
    lua_register(m_pLua, ToLower("GetPlayerSkin"), this->GetPlayerSkin);
    lua_register(m_pLua, ToLower("GetPlayerColorFeet"), this->GetPlayerColorFeet);
    lua_register(m_pLua, ToLower("GetPlayerColorBody"), this->GetPlayerColorBody);
    lua_register(m_pLua, ToLower("GetPlayerColorSkin"), this->GetPlayerColorSkin);

    //Emote
    lua_register(m_pLua, ToLower("Emote"), this->Emote);

    lua_register(m_pLua, ToLower("CreateParticle"), this->CreateParticle);
    lua_register(m_pLua, ToLower("CreateDamageIndicator"), this->CreateDamageIndicator);

    lua_register(m_pLua, ToLower("GetFlow"), this->GetFlow);
    lua_register(m_pLua, ToLower("SetFlow"), this->SetFlow);

    lua_register(m_pLua, ToLower("GetLocalCharacterId"), this->GetLocalCharacterId);
    lua_register(m_pLua, ToLower("GetLocalCharacterPos"), this->GetLocalCharacterPos);
    lua_register(m_pLua, ToLower("GetLocalCharacterWeapon"), this->GetLocalCharacterWeapon);
    lua_register(m_pLua, ToLower("GetLocalCharacterWeaponAmmo"), this->GetLocalCharacterWeaponAmmo);
    lua_register(m_pLua, ToLower("GetLocalCharacterHealth"), this->GetLocalCharacterHealth);
    lua_register(m_pLua, ToLower("GetLocalCharacterArmor"), this->GetLocalCharacterArmor);
    lua_register(m_pLua, ToLower("SetLocalCharacterPos"), this->SetLocalCharacterPos);
    lua_register(m_pLua, ToLower("GetCharacterPos"), this->GetCharacterPos);
    lua_register(m_pLua, ToLower("GetCharacterVel"), this->GetCharacterVel);
    lua_register(m_pLua, ToLower("SetCharacterPos"), this->SetCharacterPos);
    lua_register(m_pLua, ToLower("SetCharacterVel"), this->SetCharacterVel);
    lua_register(m_pLua, ToLower("GetCharacterActiveWeapon"), this->GetCharacterActiveWeapon);
    lua_register(m_pLua, ToLower("CharacterHasFlag"), this->CharacterHasFlag);
    lua_register(m_pLua, ToLower("GetCharacterHookPos"), this->GetCharacterHookPos);
    lua_register(m_pLua, ToLower("GetCharacterHookDir"), this->GetCharacterHookDir);
    lua_register(m_pLua, ToLower("GetCharacterHookState"), this->GetCharacterHookState);
    lua_register(m_pLua, ToLower("GetCharacterHookedPlayer"), this->GetCharacterHookedPlayer);
    lua_register(m_pLua, ToLower("GetCharacterHookTick"), this->GetCharacterHookTick);

    //Music
    lua_register(m_pLua, ToLower("MusicPlay"), this->MusicPlay);
    lua_register(m_pLua, ToLower("MusicPause"), this->MusicPause);
    lua_register(m_pLua, ToLower("MusicStop"), this->MusicStop);
    lua_register(m_pLua, ToLower("MusicNext"), this->MusicNext);
    lua_register(m_pLua, ToLower("MusicPrev"), this->MusicPrev);
    lua_register(m_pLua, ToLower("MusicSetVol"), this->MusicSetVol);
    lua_register(m_pLua, ToLower("MusicGetVol"), this->MusicGetVol);
    lua_register(m_pLua, ToLower("MusicGetState"), this->MusicGetState);

    lua_register(m_pLua, ToLower("GetConfigValue"), this->GetConfigValue);
    lua_register(m_pLua, ToLower("SetConfigValue"), this->SetConfigValue);

    lua_register(m_pLua, ToLower("GetControlValue"), this->GetControlValue);
    lua_register(m_pLua, ToLower("SetControlValue"), this->SetControlValue);
    lua_register(m_pLua, ToLower("SetControlValuePredicted"), this->SetControlValuePredicted);
    lua_register(m_pLua, ToLower("UnSetControlValue"), this->UnSetControlValue);

    //Console Print
    lua_register(m_pLua, ToLower("Print"), this->Print);
    lua_register(m_pLua, ToLower("Console"), this->Console);
    lua_register(m_pLua, ToLower("LocalExecute"), this->LocalExecute);
    lua_register(m_pLua, ToLower("LocalExecuteStroked"), this->LocalExecuteStroked);
    lua_register(m_pLua, ToLower("ConsoleActive"), this->ConsoleActive);
    lua_register(m_pLua, ToLower("ConsoleLocalActive"), this->ConsoleLocalActive);
    lua_register(m_pLua, ToLower("ConsoleRemoteActive"), this->ConsoleRemoteActive);


    //Remote console
    lua_register(m_pLua, ToLower("RconAuth"), this->RconAuth);
    lua_register(m_pLua, ToLower("RconAuthed"), this->RconAuthed);
    lua_register(m_pLua, ToLower("RconExecute"), this->RconExecute);

    //States
    lua_register(m_pLua, ToLower("StateOnline"), this->StateOnline);
    lua_register(m_pLua, ToLower("StateOffline"), this->StateOffline);
    lua_register(m_pLua, ToLower("StateConnecting"), this->StateConnecting);
    lua_register(m_pLua, ToLower("StateDemoplayback"), this->StateDemoplayback);
    lua_register(m_pLua, ToLower("StateLoading"), this->StateLoading);

    //Serverinfo
    lua_register(m_pLua, ToLower("GetGameType"), this->GetGameType);
    lua_register(m_pLua, ToLower("GetServerInfo"), this->GetServerInfo);
    lua_register(m_pLua, ToLower("IsTeamplay"), this->IsTeamplay);

    //Get Net Error
    lua_register(m_pLua, ToLower("GetNetError"), this->GetNetError);

    //Connect
    lua_register(m_pLua, ToLower("Connect"), this->Connect);

    //collision
    lua_register(m_pLua, ToLower("IntersectLine"), this->IntersectLine);
    lua_register(m_pLua, ToLower("MovePoint"), this->MovePoint);
    lua_register(m_pLua, ToLower("MoveBox"), this->MoveBox);
    lua_register(m_pLua, ToLower("GetTile"), this->GetTile);
    lua_register(m_pLua, ToLower("GetMapWidth"), this->GetMapWidth);
    lua_register(m_pLua, ToLower("GetMapHeight"), this->GetMapHeight);
    lua_register(m_pLua, ToLower("SetTile"), this->SetTile);
    lua_register(m_pLua, ToLower("ClosestPointOnLine"), this->ClosestPointOnLine);

    //layer
    lua_register(m_pLua, ToLower("GetNumGroups"), this->GetNumGroups);
    lua_register(m_pLua, ToLower("GetNumLayers"), this->GetNumLayers);
    lua_register(m_pLua, ToLower("GetGroupNumLayers"), this->GetGroupNumLayers);
    lua_register(m_pLua, ToLower("GetLayerType"), this->GetLayerType);
    lua_register(m_pLua, ToLower("GetLayerFlags"), this->GetLayerFlags);
    lua_register(m_pLua, ToLower("GetLayerTileFlags"), this->GetLayerTileFlags);
    lua_register(m_pLua, ToLower("GetLayerTileIndex"), this->GetLayerTileIndex);
    lua_register(m_pLua, ToLower("SetLayerTileFlags"), this->SetLayerTileFlags);
    lua_register(m_pLua, ToLower("SetLayerTileIndex"), this->SetLayerTileIndex);
    lua_register(m_pLua, ToLower("GetLayerSize"), this->GetLayerSize);
    lua_register(m_pLua, ToLower("RenderTilemapGenerateSkip"), this->RenderTilemapGenerateSkip);

    //Chat
    lua_register(m_pLua, ToLower("ChatSend"), this->ChatSend);
    lua_register(m_pLua, ToLower("ChatTeamSend"), this->ChatTeamSend);
    lua_register(m_pLua, ToLower("ChatAddLine"), this->AddChatLine);
    lua_register(m_pLua, ToLower("AddChatLine"), this->AddChatLine);

    //Ui
    lua_register(m_pLua, ToLower("UiDoButton"), this->UiDoButton);
    lua_register(m_pLua, ToLower("UiDoEditBox"), this->UiDoEditBox);
    lua_register(m_pLua, ToLower("UiDoLabel"), this->UiDoLabel);
    lua_register(m_pLua, ToLower("UiDoRect"), this->UiDoRect);
    lua_register(m_pLua, ToLower("UiDoImage"), this->UiDoImage);
    lua_register(m_pLua, ToLower("UiDoImageEx"), this->UiDoImageEx);
    lua_register(m_pLua, ToLower("UiDoLine"), this->UiDoLine);
    lua_register(m_pLua, ToLower("UiDoSlider"), this->UiDoSlider);
    lua_register(m_pLua, ToLower("UiRemoveElement"), this->UiRemoveElement);
    lua_register(m_pLua, ToLower("UiGetText"), this->UiGetText);
    lua_register(m_pLua, ToLower("UiSetText"), this->UiSetText);
    lua_register(m_pLua, ToLower("UiGetColor"), this->UiGetColor);
    lua_register(m_pLua, ToLower("UiSetColor"), this->UiSetColor);
    lua_register(m_pLua, ToLower("UiGetRect"), this->UiGetRect);
    lua_register(m_pLua, ToLower("UiSetRect"), this->UiSetRect);
    lua_register(m_pLua, ToLower("UiGetScreenWidth"), this->UiGetScreenWidth);
    lua_register(m_pLua, ToLower("UiGetScreenHeight"), this->UiGetScreenHeight);
    lua_register(m_pLua, ToLower("UiGetGameTextureID"), this->UiGetGameTextureID);
    lua_register(m_pLua, ToLower("UiGetParticleTextureID"), this->UiGetParticleTextureID);
    lua_register(m_pLua, ToLower("UiGetFlagTextureID"), this->UiGetFlagTextureID);
    //Direct Ui
    lua_register(m_pLua, ToLower("UiDirectSlider"), this->UiDirectSlider);
    lua_register(m_pLua, ToLower("UiDirectEditBox"), this->UiDirectEditBox);
    lua_register(m_pLua, ToLower("UiDirectButton"), this->UiDirectButton);
    lua_register(m_pLua, ToLower("UiDirectRect"), this->UiDirectRect);
    lua_register(m_pLua, ToLower("UiDirectLine"), this->UiDirectLine);
    lua_register(m_pLua, ToLower("UiDirectLabel"), this->UiDirectLabel);
    //DirectArrayUi
    lua_register(m_pLua, ToLower("UiDirectRectArray"), this->UiDirectRectArray);
    //BlendModes
    lua_register(m_pLua, ToLower("BlendNormal"), this->BlendNormal);
    lua_register(m_pLua, ToLower("BlendAdditive"), this->BlendAdditive);

    //
    lua_register(m_pLua, ToLower("GetScreenWidth"), this->GetScreenWidth);
    lua_register(m_pLua, ToLower("GetScreenHeight"), this->GetScreenHeight);

    //Texture
    lua_register(m_pLua, ToLower("TextureLoad"), this->TextureLoad);
    lua_register(m_pLua, ToLower("TextureUnload"), this->TextureUnload);
    lua_register(m_pLua, ToLower("RenderTexture"), this->RenderTexture);
    lua_register(m_pLua, ToLower("RenderSprite"), this->RenderSprite);
    lua_register(m_pLua, ToLower("ReplaceGameTexture"), this->ReplaceGameTexture);

    //Net
    lua_register(m_pLua, ToLower("SendPacket"), this->SendPacket);

    //Sound
    lua_register(m_pLua, ToLower("LoadWvFile"), this->LoadWvFile);
    lua_register(m_pLua, ToLower("PlayWv"), this->PlayWv);
    lua_register(m_pLua, ToLower("PlaySound"), this->PlaySound);

    //demo
    lua_register(m_pLua, ToLower("DemoStart"), this->DemoStart);
    lua_register(m_pLua, ToLower("DemoStop"), this->DemoStop);
    lua_register(m_pLua, ToLower("DemoDelete"), this->DemoDelete);

    //stats
    lua_register(m_pLua, ToLower("StatGetNumber"), this->StatGetNumber);
    lua_register(m_pLua, ToLower("StatGetInfo"), this->StatGetInfo);
    lua_register(m_pLua, ToLower("StatGetRow"), this->StatGetRow);

    //editor
    //lua_register(m_pLua, "SetMapLuaData", this->SetMapLuaData);
    //lua_register(m_pLua, "GetMapLuaData", this->GetMapLuaData);


    lua_register(m_pLua, ToLower("TimeGet"), this->TimeGet);
    lua_register(m_pLua, ToLower("GetDate"), this->GetDate);
    lua_register(m_pLua, ToLower("FPS"), this->FPS);

    //version
    lua_register(m_pLua, ToLower("CheckVersion"), this->CheckVersion);
    lua_register(m_pLua, ToLower("GetVersion"), this->GetVersion);

    //sound hooks
    lua_register(m_pLua, ToLower("GetWaveFrameSize"), this->GetWaveFrameSize);
    lua_register(m_pLua, ToLower("AddWaveToStream"), this->AddWaveToStream);
    lua_register(m_pLua, ToLower("FloatToShortChars"), this->FloatToShortChars);
    lua_register(m_pLua, ToLower("GetWaveBufferSpace"), this->GetWaveBufferSpace);

    //load skin
    lua_register(m_pLua, ToLower("LoadSkin"), this->LoadSkin);


    lua_register(m_pLua, ToLower("CreateDirectory"), this->CreateDirectory);
    lua_register(m_pLua, ToLower("ListDirectory"), this->ListDirectory);


    //thanks to MJavad
    lua_register(m_pLua, ToLower("HostLookup"), this->HostLookup);
	lua_register(m_pLua, ToLower("HostLookupGetResult"), this->HostLookupGetResult);

    m_pLuaShared = new CLuaShared<CLuaFile>(this);

    lua_pushlightuserdata(m_pLua, this);
    lua_setglobal(m_pLua, "pLUA");

    lua_register(m_pLua, ToLower("errorfunc"), this->ErrorFunc);
    //lua_getglobal(m_pLua, "errorfunc"); //could this line stay commented out
    //if this line is used the stack contains one function which would leeds to problems with return values

    if (pFile)
    {
        IOHANDLE File = io_open(m_aFilename, IOFLAG_READ);
        if (!File)
        {
            dbg_msg("Lua", "File not found");
            return;
        }
        if (luaL_loadfile(m_pLua, m_aFilename) == 0)
        {
            if (lua_pcall(m_pLua, 0, LUA_MULTRET, 0))
            {
                dbg_msg("Error", "Fail");
                ErrorFunc(m_pLua);
                m_Error = 1;
            }
        }
        else
        {
            dbg_msg("Error", "Fail");
            m_Error = 1;
            ErrorFunc(m_pLua);
        }
    }
}

void CLuaFile::Close()
{
    //Run the End function
    //-Calls the end function in lua
    //-Deletes eventlistener
    //-Frees textures
    End();

    m_Error = false;

    //kill lua
    if (m_pLua)
        lua_close(m_pLua);
    m_pLua = 0;
}

int CLuaFile::ErrorFunc(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);

    lua_pop(L,1);

    int depth = 0;
    int frameskip = 1;
    lua_Debug frame;

    if (lua_tostring(L, -1) == 0)
        return 0;

    dbg_msg("Lua", pSelf->m_aFilename);
    dbg_msg("Lua", lua_tostring(L, -1));

    dbg_msg("Lua", "Backtrace:");
    while(lua_getstack(L, depth, &frame) == 1)
    {
        depth++;

        lua_getinfo(L, "nlSf", &frame);

        /* check for functions that just report errors. these frames just confuses more then they help */
        if(frameskip && str_comp(frame.short_src, "[C]") == 0 && frame.currentline == -1)
            continue;
        frameskip = 0;

        /* print stack frame */
        dbg_msg("Lua", "%s(%d): %s %s", frame.short_src, frame.currentline, frame.name, frame.namewhat);
    }
    lua_pop(L, 1); // remove error message
    lua_gc(L, LUA_GCCOLLECT, 0);
    return 0;
}


void CLuaFile::ConfigClose()
{
    FunctionExec("ConfigClose");
}

void CLuaFile::PushString(const char *pString)
{
    if (m_pLua == 0)
        return;
    lua_pushstring(m_pLua, pString);
    m_FunctionVarNum++;
}

void CLuaFile::PushData(const char *pData, int Size)
{
    if (m_pLua == 0)
        return;
    lua_pushlstring(m_pLua, pData, Size);
    m_FunctionVarNum++;
}

void CLuaFile::PushInteger(int value)
{
    if (m_pLua == 0)
        return;
    lua_pushinteger(m_pLua, value);
    m_FunctionVarNum++;
}

void CLuaFile::PushFloat(float value)
{
    if (m_pLua == 0)
        return;
    lua_pushnumber(m_pLua, value);
    m_FunctionVarNum++;
}

void CLuaFile::PushBoolean(bool value)
{
    if (m_pLua == 0)
        return;
    lua_pushboolean(m_pLua, value);
    m_FunctionVarNum++;
}

void CLuaFile::PushParameter(const char *pString)
{
    if (m_pLua == 0)
        return;
    if (StrIsInteger(pString))
    {
        PushInteger(str_toint(pString));
    }
    else if (StrIsFloat(pString))
    {
        PushInteger(str_tofloat(pString));
    }
    else
    {
        PushString(pString);
    }

}

bool CLuaFile::FunctionExist(const char *pFunctionName)
{
    bool Ret = false;
    if (m_pLua == 0)
        return false;
    lua_getglobal(m_pLua, ToLower(pFunctionName));
    Ret = lua_isfunction(m_pLua, -1);
    lua_pop(m_pLua, 1);
    return Ret;
}

void CLuaFile::FunctionPrepare(const char *pFunctionName)
{
    if (m_pLua == 0)
        return;

    //lua_pushstring (m_pLua, pFunctionName);
    //lua_gettable (m_pLua, LUA_GLOBALSINDEX);
    lua_getglobal(m_pLua, ToLower(pFunctionName));
    m_FunctionVarNum = 0;
}

int CLuaFile::FunctionExec(const char *pFunctionName)
{
    if (m_pLua == 0)
        return 0;

    if (pFunctionName)
    {
        if (FunctionExist(pFunctionName) == false)
            return 0;
        FunctionPrepare(pFunctionName);
    }
    int Ret = lua_pcall(m_pLua, m_FunctionVarNum, LUA_MULTRET, 0);
    if (Ret)
        ErrorFunc(m_pLua);
    m_FunctionVarNum = 0;
    return Ret;
}



//functions

int CLuaFile::Include(lua_State *L)
{
    LUA_FUNCTION_HEADER

    //check if file exists
    if (!lua_isstring(L, 1))
        return 0;

    char aBuffer[1024] = {0};
    IOHANDLE CheckFile = pSelf->m_pClient->Storage()->OpenFile(lua_tostring(L, 1), IOFLAG_READ, IStorage::TYPE_ALL, aBuffer, sizeof(aBuffer));
    if (CheckFile)
    {
        io_close(CheckFile);
    }
    else
    {
        return 0;
    }

    if (luaL_loadfile(L, aBuffer) == 0)
    {
        lua_pcall(L, 0, LUA_MULTRET, 0);
    }
    else
    {
        lua_error(L);
    }

    return 0;
}

int CLuaFile::SetScriptUseSettingPage(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isnumber(L, 1))
        return 0;
    pSelf->m_HaveSettings = lua_tointeger(L, 1);
    return 0;
}

int CLuaFile::SetScriptTitle(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isstring(L, 1))
        return 0;
    str_copy(pSelf->m_aTitle, lua_tostring(L, 1), sizeof(pSelf->m_aTitle));
    return 0;
}

int CLuaFile::SetScriptInfo(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isstring(L, 1))
        return 0;
    str_copy(pSelf->m_aInfo, lua_tostring(L, 1), sizeof(pSelf->m_aInfo));
    return 0;
}

int CLuaFile::AddEventListener(lua_State *L)
{
    LUA_FUNCTION_HEADER


    if (!lua_isstring(L, 1) && !lua_isstring(L, 2))
        return 0;
    pSelf->m_pLuaHandler->m_pEventListener->AddEventListener(pSelf, (char *)lua_tostring(L, 1), (char *)lua_tostring(L, 2));
    return 0;
}

int CLuaFile::RemoveEventListener(lua_State *L)
{
    LUA_FUNCTION_HEADER


    if (!lua_isstring(L, 1))
        return 0;
    pSelf->m_pLuaHandler->m_pEventListener->RemoveEventListener(pSelf, (char *)lua_tostring(L, 1));
    return 0;
}

int CLuaFile::GetPlayerName(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (lua_isnumber(L, 1))
    {
        if (lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS)
        {
            if (pSelf->m_pClient->m_aClients[lua_tointeger(L, 1)].m_aName[0])
                lua_pushstring(L, pSelf->m_pClient->m_aClients[lua_tointeger(L, 1)].m_aName);
            else
                lua_pushnil(L);
        }
        else
            lua_pushnil(L);
    }
    else
        lua_pushnil(L);
    return 1;
}

int CLuaFile::GetPlayerClan(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (lua_isnumber(L, 1))
    {
        if (lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS)
        {
            lua_pushstring(L, pSelf->m_pClient->m_aClients[lua_tointeger(L, 1)].m_aClan);
            return 1;
        }
    }
    return 0;
}

int CLuaFile::GetPlayerCountry(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (lua_isnumber(L, 1))
    {
        if (lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS)
        {
            pSelf->m_pClient->m_UpdateScoreboard = true;
            lua_pushinteger(L, pSelf->m_pClient->m_aClients[lua_tointeger(L, 1)].m_Country);
            return 1;
        }
    }
    return 0;
}

int CLuaFile::GetPlayerScore(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (lua_isnumber(L, 1))
    {
        if (lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS)
        {
            const CNetObj_PlayerInfo *pInfo = pSelf->m_pClient->m_Snap.m_paPlayerInfos[lua_tointeger(L, 1)];
            if (pInfo)
            {
                pSelf->m_pClient->m_UpdateScoreboard = true;
                lua_pushinteger(L, pInfo->m_Score);
                return 1;
            }
            lua_pushinteger(L, 0);
            return 1;
        }
    }
    return 0;
}

int CLuaFile::GetPlayerPing(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (lua_isnumber(L, 1))
    {
        if (lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS)
        {
            const CNetObj_PlayerInfo *pInfo = pSelf->m_pClient->m_Snap.m_paPlayerInfos[lua_tointeger(L, 1)];
            if (pInfo)
            {
                pSelf->m_pClient->m_UpdateScoreboard = true;
                lua_pushinteger(L, pInfo->m_Latency);
                return 1;
            }
        }
    }
    return 0;
}

int CLuaFile::GetPlayerTeam(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (lua_isnumber(L, 1))
    {
        if (lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS)
        {
            const CNetObj_PlayerInfo *pInfo = pSelf->m_pClient->m_Snap.m_paPlayerInfos[lua_tointeger(L, 1)];
            if (pInfo)
            {
                lua_pushinteger(L, pInfo->m_Team);
                return 1;
            }
        }
    }
    return 0;
}

int CLuaFile::GetPlayerSkin(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (lua_isnumber(L, 1))
    {
        if (lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS)
        {
            lua_pushstring(L, pSelf->m_pClient->m_aClients[lua_tointeger(L, 1)].m_aSkinName);
            return 1;
        }
    }
    return 0;
}

int CLuaFile::GetPlayerColorFeet(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (lua_isnumber(L, 1))
    {
        if (lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS)
        {
            lua_pushnumber(L, pSelf->m_pClient->m_pSkins->GetColorV3(pSelf->m_pClient->m_aClients[lua_tointeger(L, 1)].m_ColorFeet).r);
            lua_pushnumber(L, pSelf->m_pClient->m_pSkins->GetColorV3(pSelf->m_pClient->m_aClients[lua_tointeger(L, 1)].m_ColorFeet).g);
            lua_pushnumber(L, pSelf->m_pClient->m_pSkins->GetColorV3(pSelf->m_pClient->m_aClients[lua_tointeger(L, 1)].m_ColorFeet).b);
            lua_pushnumber(L, 1.0f);
            return 4;
        }
    }
    return 0;
}

int CLuaFile::GetPlayerColorBody(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (lua_isnumber(L, 1))
    {
        if (lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS)
        {
            lua_pushnumber(L, pSelf->m_pClient->m_pSkins->GetColorV3(pSelf->m_pClient->m_aClients[lua_tointeger(L, 1)].m_ColorBody).r);
            lua_pushnumber(L, pSelf->m_pClient->m_pSkins->GetColorV3(pSelf->m_pClient->m_aClients[lua_tointeger(L, 1)].m_ColorBody).g);
            lua_pushnumber(L, pSelf->m_pClient->m_pSkins->GetColorV3(pSelf->m_pClient->m_aClients[lua_tointeger(L, 1)].m_ColorBody).b);
            lua_pushnumber(L, 1.0f);
            return 4;
        }
    }
    return 0;
}

int CLuaFile::GetPlayerColorSkin(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (lua_isnumber(L, 1))
    {
        if (lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS)
        {
            if (pSelf->m_pClient->m_aClients[lua_tointeger(L, 1)].m_UseCustomColor)
            {
                lua_pushnumber(L, pSelf->m_pClient->m_pSkins->GetColorV3(pSelf->m_pClient->m_aClients[lua_tointeger(L, 1)].m_ColorBody).r);
                lua_pushnumber(L, pSelf->m_pClient->m_pSkins->GetColorV3(pSelf->m_pClient->m_aClients[lua_tointeger(L, 1)].m_ColorBody).g);
                lua_pushnumber(L, pSelf->m_pClient->m_pSkins->GetColorV3(pSelf->m_pClient->m_aClients[lua_tointeger(L, 1)].m_ColorBody).b);
                lua_pushnumber(L, 1.0f);
                return 4;
            }
            else
            {
                const CSkins::CSkin *s = pSelf->m_pClient->m_pSkins->Get(pSelf->m_pClient->m_aClients[lua_tointeger(L, 1)].m_SkinID);
                if (s)
                {
                    lua_pushnumber(L, s->m_BloodColor.r);
                    lua_pushnumber(L, s->m_BloodColor.g);
                    lua_pushnumber(L, s->m_BloodColor.b);
                    lua_pushnumber(L, 1.0f);
                    return 4;
                }
            }
        }
    }
    return 0;
}


int CLuaFile::UiGetScreenWidth(lua_State *L)
{
    LUA_FUNCTION_HEADER

    CUIRect Screen = *pSelf->m_pClient->UI()->Screen();
    lua_pushnumber(L, Screen.w);
    return 1;
}

int CLuaFile::UiGetScreenHeight(lua_State *L)
{
    LUA_FUNCTION_HEADER

    CUIRect Screen = *pSelf->m_pClient->UI()->Screen();
    lua_pushnumber(L, Screen.h);
    return 1;
}

int CLuaFile::GetScreenWidth(lua_State *L)
{
    LUA_FUNCTION_HEADER

    float X0;
    float Y0;
    float X1;
    float Y1;
    pSelf->m_pClient->Graphics()->GetScreen(&X0, &Y0, &X1, &Y1);
    lua_pushnumber(L, X1 - X0);
    return 1;
}

int CLuaFile::GetScreenHeight(lua_State *L)
{
    LUA_FUNCTION_HEADER

    float X0;
    float Y0;
    float X1;
    float Y1;
    pSelf->m_pClient->Graphics()->GetScreen(&X0, &Y0, &X1, &Y1);
    lua_pushnumber(L, Y1 - Y0);
    return 1;
}


int CLuaFile::MusicPlay(lua_State *L)
{
    LUA_FUNCTION_HEADER

    pSelf->m_pClient->Sound()->m_MusicPlaying = true;
    pSelf->m_pClient->m_Music->m_MusicListActivated = true;
    return 0;
}

int CLuaFile::MusicPause(lua_State *L)
{
    LUA_FUNCTION_HEADER

    pSelf->m_pClient->Sound()->m_MusicPlaying = pSelf->m_pClient->Sound()->m_MusicPlaying ^ 1;
    if (pSelf->m_pClient->m_Music->m_MusicFirstPlay)
        pSelf->m_pClient->m_Music->m_MusicListActivated = true;
    return 0;
}

int CLuaFile::MusicStop(lua_State *L)
{
    LUA_FUNCTION_HEADER

    pSelf->m_pClient->Sound()->m_MusicPlaying = false;
    pSelf->m_pClient->m_Music->m_MusicListActivated = false;
    return 0;
}

int CLuaFile::MusicNext(lua_State *L)
{
    LUA_FUNCTION_HEADER

    pSelf->m_pClient->Sound()->m_MusicPlayIndex++;
    return 0;
}

int CLuaFile::MusicPrev(lua_State *L)
{
    LUA_FUNCTION_HEADER

    pSelf->m_pClient->Sound()->m_MusicPlayIndex--;
    return 0;
}

int CLuaFile::MusicSetVol(lua_State *L)
{
    LUA_FUNCTION_HEADER

    g_Config.m_SndMusicVolume = clamp((int)lua_tointeger(L, 1), 0, 100);
    return 0;
}

int CLuaFile::MusicGetVol(lua_State *L)
{
    LUA_FUNCTION_HEADER

    lua_pushinteger(L, g_Config.m_SndMusicVolume);
    return 1;
}

int CLuaFile::MusicGetState(lua_State *L)
{
    LUA_FUNCTION_HEADER

    lua_pushinteger(L, pSelf->m_pClient->Sound()->m_MusicPlaying);
    return 1;
}

int CLuaFile::MusicGetPlayedIndex(lua_State *L)
{
    LUA_FUNCTION_HEADER

    lua_pushinteger(L, pSelf->m_pClient->Sound()->m_MusicPlayIndex);
    return 1;
}

int CLuaFile::SendPlayerInfo(lua_State *L)
{
    LUA_FUNCTION_HEADER

    pSelf->m_pClient->SendInfo(false);
    return 0;
}

int CLuaFile::Console(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (lua_isnumber(L, 1) && lua_isstring(L, 2) && lua_isstring(L, 3))
    {
        pSelf->m_pClient->Console()->Print(lua_tointeger(L, 1), lua_tostring(L, 2), lua_tostring(L, 3));
    }
    return 0;
}

int CLuaFile::Emote(lua_State *L)
{
    static CThrottle s_Throttle;
    LUA_FUNCTION_HEADER

    if(s_Throttle.Throttled(5))
    {
        if (lua_isnumber(L, 1))
        {
            CNetMsg_Cl_Emoticon Msg;
            Msg.m_Emoticon = lua_tonumber(L, 1);
            pSelf->m_pClient->Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);
        }
    }
    return 0;
}


int CLuaFile::GetConfigValue(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isstring(L, 1))
        return 0;
    if (str_comp_nocase(lua_tostring(L, 1), "PlayerName") == 0)
    {
        lua_pushstring(L, g_Config.m_PlayerName);
        return 1;
    }
    if (str_comp_nocase(lua_tostring(L, 1), "MouseDeadzone") == 0)
    {
        lua_pushinteger(L, g_Config.m_ClMouseDeadzone);
        return 1;
    }
    if (str_comp_nocase(lua_tostring(L, 1), "MouseFollowfactor") == 0)
    {
        lua_pushinteger(L, g_Config.m_ClMouseFollowfactor);
        return 1;
    }
    if (str_comp_nocase(lua_tostring(L, 1), "MouseMaxDistance") == 0)
    {
        lua_pushinteger(L, g_Config.m_ClMouseMaxDistance);
        return 1;
    }
    if (str_comp_nocase(lua_tostring(L, 1), "PlayerColorBody") == 0)
    {
        lua_pushinteger(L, g_Config.m_PlayerColorBody);
        return 1;
    }
    if (str_comp_nocase(lua_tostring(L, 1), "PlayerColorFeet") == 0)
    {
        lua_pushinteger(L, g_Config.m_PlayerColorFeet);
        return 1;
    }
    if (str_comp_nocase(lua_tostring(L, 1), "Nameplates") == 0)
    {
        lua_pushinteger(L, g_Config.m_ClNameplates);
        return 1;
    }
    if (str_comp_nocase(lua_tostring(L, 1), "WarningTeambalance") == 0)
    {
        lua_pushinteger(L, g_Config.m_ClWarningTeambalance);
        return 1;
    }
    return 0;
}

int CLuaFile::SetConfigValue(lua_State *L)
{
    static CThrottle s_Throttle;
    LUA_FUNCTION_HEADER

    if (!lua_isstring(L, 1))
        return 0;
    if (str_comp_nocase(lua_tostring(L, 1), "PlayerName") == 0 && lua_isstring(L, 2))
    {
        str_copy(g_Config.m_PlayerName, lua_tostring(L, 2), sizeof(g_Config.m_PlayerName));
    }
    if (str_comp_nocase(lua_tostring(L, 1), "MouseDeadzone") == 0 && lua_isnumber(L, 2))
    {
        g_Config.m_ClMouseDeadzone = lua_tointeger(L, 2);
    }
    if (str_comp_nocase(lua_tostring(L, 1), "MouseFollowfactor") == 0 && lua_isnumber(L, 2))
    {
        g_Config.m_ClMouseFollowfactor = lua_tointeger(L, 2);
    }
    if (str_comp_nocase(lua_tostring(L, 1), "MouseMaxDistance") == 0 && lua_isnumber(L, 2))
    {
        g_Config.m_ClMouseMaxDistance = lua_tointeger(L, 2);
    }
    if (str_comp_nocase(lua_tostring(L, 1), "PlayerColorBody") == 0 && lua_isnumber(L, 2))
    {
        if(s_Throttle.Throttled(30))
            g_Config.m_PlayerColorBody = lua_tointeger(L, 2);
    }
    if (str_comp_nocase(lua_tostring(L, 1), "PlayerColorFeet") == 0 && lua_isnumber(L, 2))
    {
        if(s_Throttle.Throttled(30))
            g_Config.m_PlayerColorFeet = lua_tointeger(L, 2);
    }
    if (str_comp_nocase(lua_tostring(L, 1), "Nameplates") == 0 && lua_isnumber(L, 2))
    {
        g_Config.m_ClNameplates = lua_tointeger(L, 2);
    }
    if (str_comp_nocase(lua_tostring(L, 1), "WarningTeambalance") == 0 && lua_isnumber(L, 2))
    {
        g_Config.m_ClWarningTeambalance = lua_tointeger(L, 2);
    }
    return 0;
}

int CLuaFile::GetControlValue(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isstring(L, 1))
        return 0;
    if (str_comp_nocase(lua_tostring(L, 1), "Direction") == 0)
    {
        lua_pushnumber(L, pSelf->m_pClient->m_pLuaBinding->m_ControlDirectionPre);
    }
    if (str_comp_nocase(lua_tostring(L, 1), "Fire") == 0)
    {
        lua_pushnumber(L, pSelf->m_pClient->m_pLuaBinding->m_ControlFirePre);
    }
    if (str_comp_nocase(lua_tostring(L, 1), "Hook") == 0)
    {
        lua_pushnumber(L, pSelf->m_pClient->m_pLuaBinding->m_ControlHookPre);
    }
    if (str_comp_nocase(lua_tostring(L, 1), "Jump") == 0)
    {
        lua_pushnumber(L, pSelf->m_pClient->m_pLuaBinding->m_ControlJumpPre);
    }
    if (str_comp_nocase(lua_tostring(L, 1), "Weapon") == 0)
    {
        lua_pushnumber(L, pSelf->m_pClient->m_pLuaBinding->m_ControlWeaponPre);
    }
    if (str_comp_nocase(lua_tostring(L, 1), "TargetX") == 0)
    {
        lua_pushnumber(L, pSelf->m_pClient->m_pLuaBinding->m_ControlTargetXPre);
    }
    if (str_comp_nocase(lua_tostring(L, 1), "TargetY") == 0)
    {
        lua_pushnumber(L, pSelf->m_pClient->m_pLuaBinding->m_ControlTargetYPre);
    }
    return 1;
}

int CLuaFile::SetControlValue(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isstring(L, 1) && !lua_isnumber(L, 2))
        return 0;
    if (str_comp_nocase(lua_tostring(L, 1), "Direction") == 0)
    {
        pSelf->m_pClient->m_pLuaBinding->m_ControlDirection = lua_tointeger(L, 2);
        pSelf->m_pClient->m_pLuaBinding->m_ControlDirectionIsSet = true;
    }
    if (str_comp_nocase(lua_tostring(L, 1), "Fire") == 0)
    {
        pSelf->m_pClient->m_pLuaBinding->m_ControlFire = lua_tointeger(L, 2);
        pSelf->m_pClient->m_pLuaBinding->m_ControlFireIsSet = true;
    }
    if (str_comp_nocase(lua_tostring(L, 1), "Hook") == 0)
    {
        pSelf->m_pClient->m_pLuaBinding->m_ControlHook = lua_tointeger(L, 2);
        pSelf->m_pClient->m_pLuaBinding->m_ControlHookIsSet = true;
    }
    if (str_comp_nocase(lua_tostring(L, 1), "Jump") == 0)
    {
        pSelf->m_pClient->m_pLuaBinding->m_ControlJump = lua_tointeger(L, 2);
        pSelf->m_pClient->m_pLuaBinding->m_ControlJumpIsSet = true;
    }
    if (str_comp_nocase(lua_tostring(L, 1), "Weapon") == 0)
    {
        pSelf->m_pClient->m_pLuaBinding->m_ControlWeapon = lua_tointeger(L, 2);
        pSelf->m_pClient->m_pLuaBinding->m_ControlWeaponIsSet = true;
    }
    if (str_comp_nocase(lua_tostring(L, 1), "TargetX") == 0)
    {
        pSelf->m_pClient->m_pLuaBinding->m_ControlTargetX = lua_tointeger(L, 2);
        pSelf->m_pClient->m_pLuaBinding->m_ControlTargetXIsSet = true;
    }
    if (str_comp_nocase(lua_tostring(L, 1), "TargetY") == 0)
    {
        pSelf->m_pClient->m_pLuaBinding->m_ControlTargetY = lua_tointeger(L, 2);
        pSelf->m_pClient->m_pLuaBinding->m_ControlTargetYIsSet = true;
    }
    return 0;
}

int CLuaFile::SetControlValuePredicted(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isstring(L, 1) && !lua_isnumber(L, 2))
        return 0;
    if (str_comp_nocase(lua_tostring(L, 1), "Direction") == 0)
    {
        pSelf->m_pClient->m_pLuaBinding->m_ControlDirectionPredicted = lua_tointeger(L, 2);
        pSelf->m_pClient->m_pLuaBinding->m_ControlDirectionPredictedIsSet = true;
    }
    if (str_comp_nocase(lua_tostring(L, 1), "Fire") == 0)
    {
        pSelf->m_pClient->m_pLuaBinding->m_ControlFirePredicted = lua_tointeger(L, 2);
        pSelf->m_pClient->m_pLuaBinding->m_ControlFirePredictedIsSet = true;
    }
    if (str_comp_nocase(lua_tostring(L, 1), "Hook") == 0)
    {
        pSelf->m_pClient->m_pLuaBinding->m_ControlHookPredicted = lua_tointeger(L, 2);
        pSelf->m_pClient->m_pLuaBinding->m_ControlHookPredictedIsSet = true;
    }
    if (str_comp_nocase(lua_tostring(L, 1), "Jump") == 0)
    {
        pSelf->m_pClient->m_pLuaBinding->m_ControlJumpPredicted = lua_tointeger(L, 2);
        pSelf->m_pClient->m_pLuaBinding->m_ControlJumpPredictedIsSet = true;
    }
    if (str_comp_nocase(lua_tostring(L, 1), "Weapon") == 0)
    {
        pSelf->m_pClient->m_pLuaBinding->m_ControlWeaponPredicted = lua_tointeger(L, 2);
        pSelf->m_pClient->m_pLuaBinding->m_ControlWeaponPredictedIsSet = true;
    }
    if (str_comp_nocase(lua_tostring(L, 1), "TargetX") == 0)
    {
        pSelf->m_pClient->m_pLuaBinding->m_ControlTargetXPredicted = lua_tointeger(L, 2);
        pSelf->m_pClient->m_pLuaBinding->m_ControlTargetXPredictedIsSet = true;
    }
    if (str_comp_nocase(lua_tostring(L, 1), "TargetY") == 0)
    {
        pSelf->m_pClient->m_pLuaBinding->m_ControlTargetYPredicted = lua_tointeger(L, 2);
        pSelf->m_pClient->m_pLuaBinding->m_ControlTargetYPredictedIsSet = true;
    }
    return 0;
}

int CLuaFile::UnSetControlValue(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isstring(L, 1))
        return 0;
    if (str_comp_nocase(lua_tostring(L, 1), "Direction") == 0)
    {
        pSelf->m_pClient->m_pLuaBinding->m_ControlDirection = 0;
        pSelf->m_pClient->m_pLuaBinding->m_ControlDirectionPredicted = 0;
        pSelf->m_pClient->m_pLuaBinding->m_ControlDirectionIsSet = false;
        pSelf->m_pClient->m_pLuaBinding->m_ControlDirectionPredictedIsSet = false;
    }
    if (str_comp_nocase(lua_tostring(L, 1), "Fire") == 0)
    {
        pSelf->m_pClient->m_pLuaBinding->m_ControlFire = 0;
        pSelf->m_pClient->m_pLuaBinding->m_ControlFirePredicted = 0;
        pSelf->m_pClient->m_pLuaBinding->m_ControlFireIsSet = false;
        pSelf->m_pClient->m_pLuaBinding->m_ControlFirePredictedIsSet = false;
    }
    if (str_comp_nocase(lua_tostring(L, 1), "Hook") == 0)
    {
        pSelf->m_pClient->m_pLuaBinding->m_ControlHook = 0;
        pSelf->m_pClient->m_pLuaBinding->m_ControlHookPredicted = 0;
        pSelf->m_pClient->m_pLuaBinding->m_ControlHookIsSet = false;
        pSelf->m_pClient->m_pLuaBinding->m_ControlHookPredictedIsSet = false;
    }
    if (str_comp_nocase(lua_tostring(L, 1), "Jump") == 0)
    {
        pSelf->m_pClient->m_pLuaBinding->m_ControlJump = 0;
        pSelf->m_pClient->m_pLuaBinding->m_ControlJumpPredicted = 0;
        pSelf->m_pClient->m_pLuaBinding->m_ControlJumpIsSet = false;
        pSelf->m_pClient->m_pLuaBinding->m_ControlJumpPredictedIsSet = false;
    }
    if (str_comp_nocase(lua_tostring(L, 1), "Weapon") == 0)
    {
        pSelf->m_pClient->m_pLuaBinding->m_ControlWeapon = 0;
        pSelf->m_pClient->m_pLuaBinding->m_ControlWeaponPredicted = 0;
        pSelf->m_pClient->m_pLuaBinding->m_ControlWeaponIsSet = false;
        pSelf->m_pClient->m_pLuaBinding->m_ControlWeaponPredictedIsSet = false;
    }
    if (str_comp_nocase(lua_tostring(L, 1), "TargetX") == 0)
    {
        pSelf->m_pClient->m_pLuaBinding->m_ControlTargetX = 0;
        pSelf->m_pClient->m_pLuaBinding->m_ControlTargetXPredicted = 0;
        pSelf->m_pClient->m_pLuaBinding->m_ControlTargetXIsSet = false;
        pSelf->m_pClient->m_pLuaBinding->m_ControlTargetXPredictedIsSet = false;
    }
    if (str_comp_nocase(lua_tostring(L, 1), "TargetY") == 0)
    {
        pSelf->m_pClient->m_pLuaBinding->m_ControlTargetY = 0;
        pSelf->m_pClient->m_pLuaBinding->m_ControlTargetYPredicted = 0;
        pSelf->m_pClient->m_pLuaBinding->m_ControlTargetYIsSet = false;
        pSelf->m_pClient->m_pLuaBinding->m_ControlTargetYPredictedIsSet = false;
    }
    return 0;
}

int CLuaFile::GetFlow(lua_State *L)
{
    LUA_FUNCTION_HEADER

    vec2 tmp = pSelf->m_pClient->m_pFlow->Get(vec2(lua_tonumber(L, 1), lua_tonumber(L, 2)));
    lua_pushnumber(L, tmp.x);
    lua_pushnumber(L, tmp.y);
    return 2;
}

int CLuaFile::SetFlow(lua_State *L)
{
    LUA_FUNCTION_HEADER

    pSelf->m_pClient->m_pFlow->Add(vec2(lua_tonumber(L, 1), lua_tonumber(L, 2)), vec2(lua_tonumber(L, 3), lua_tonumber(L, 4)), lua_tonumber(L, 5));
    return 0;
}

int CLuaFile::GetCharacterPos(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isnumber(L, 1))
        return 0;
    vec2 Pos = mix(vec2(pSelf->m_pClient->m_Snap.m_aCharacters[lua_tointeger(L, 1)].m_Prev.m_X, pSelf->m_pClient->m_Snap.m_aCharacters[lua_tointeger(L, 1)].m_Prev.m_Y), vec2(pSelf->m_pClient->m_Snap.m_aCharacters[lua_tointeger(L, 1)].m_Cur.m_X, pSelf->m_pClient->m_Snap.m_aCharacters[lua_tointeger(L, 1)].m_Cur.m_Y), pSelf->m_pClient->Client()->IntraGameTick());
    lua_pushnumber(L, Pos.x);
    lua_pushnumber(L, Pos.y);
    return 2;
}

int CLuaFile::SetCharacterPos(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isnumber(L, 1))
        return 0;
    if (!lua_isnumber(L, 2))
        return 0;
    if (!lua_isnumber(L, 3))
        return 0;

    pSelf->m_pClient->m_Snap.m_aCharacters[lua_tointeger(L, 1)].m_Cur.m_X = lua_tonumber(L, 2);
    pSelf->m_pClient->m_Snap.m_aCharacters[lua_tointeger(L, 1)].m_Cur.m_Y = lua_tonumber(L, 3);
    return 0;
}

int CLuaFile::SetCharacterVel(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isnumber(L, 1))
        return 0;
    if (!lua_isnumber(L, 2))
        return 0;
    if (!lua_isnumber(L, 3))
        return 0;

    pSelf->m_pClient->m_Snap.m_aCharacters[lua_tointeger(L, 1)].m_Cur.m_VelX = lua_tonumber(L, 2);
    pSelf->m_pClient->m_Snap.m_aCharacters[lua_tointeger(L, 1)].m_Cur.m_VelY = lua_tonumber(L, 3);
    return 0;
}

int CLuaFile::GetCharacterActiveWeapon(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isnumber(L, 1))
        return 0;
    lua_pushinteger(L, pSelf->m_pClient->m_Snap.m_aCharacters[lua_tointeger(L, 1)].m_Cur.m_Weapon);
    return 1;
}

int CLuaFile::CharacterHasFlag(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isnumber(L, 1))
        return 0;
    lua_pushboolean(L, false); //todo
    return 1;
}

int CLuaFile::GetCharacterHookPos(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isnumber(L, 1))
        return 0;
    lua_pushnumber(L, pSelf->m_pClient->m_Snap.m_aCharacters[lua_tointeger(L, 1)].m_Cur.m_HookX);
    lua_pushnumber(L, pSelf->m_pClient->m_Snap.m_aCharacters[lua_tointeger(L, 1)].m_Cur.m_HookY);
    return 2;
}

int CLuaFile::GetCharacterHookDir(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isnumber(L, 1))
        return 0;
    lua_pushnumber(L, pSelf->m_pClient->m_Snap.m_aCharacters[lua_tointeger(L, 1)].m_Cur.m_HookDx);
    lua_pushnumber(L, pSelf->m_pClient->m_Snap.m_aCharacters[lua_tointeger(L, 1)].m_Cur.m_HookDy);
    return 2;
}

int CLuaFile::GetCharacterHookState(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isnumber(L, 1))
        return 0;
    lua_pushinteger(L, pSelf->m_pClient->m_Snap.m_aCharacters[lua_tointeger(L, 1)].m_Cur.m_HookState);
    return 1;
}

int CLuaFile::GetCharacterHookedPlayer(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isnumber(L, 1))
        return 0;
    lua_pushinteger(L, pSelf->m_pClient->m_Snap.m_aCharacters[lua_tointeger(L, 1)].m_Cur.m_HookedPlayer);
    return 1;
}

int CLuaFile::GetCharacterHookTick(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isnumber(L, 1))
        return 0;
    lua_pushinteger(L, pSelf->m_pClient->m_Snap.m_aCharacters[lua_tointeger(L, 1)].m_Cur.m_HookTick);
    return 1;
}

int CLuaFile::GetCharacterVel(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isnumber(L, 1))
        return 0;
    lua_pushnumber(L, pSelf->m_pClient->m_Snap.m_aCharacters[lua_tointeger(L, 1)].m_Cur.m_X - pSelf->m_pClient->m_Snap.m_aCharacters[lua_tointeger(L, 1)].m_Prev.m_X);
    lua_pushnumber(L, pSelf->m_pClient->m_Snap.m_aCharacters[lua_tointeger(L, 1)].m_Cur.m_Y - pSelf->m_pClient->m_Snap.m_aCharacters[lua_tointeger(L, 1)].m_Prev.m_Y);
    return 2;
}

int CLuaFile::GetLocalCharacterId(lua_State *L)
{
    LUA_FUNCTION_HEADER

    lua_pushnumber(L, pSelf->m_pClient->m_Snap.m_LocalClientID);
    return 1;
}

int CLuaFile::IntersectLine(lua_State *L)
{
    LUA_FUNCTION_HEADER


    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
        return 0;
    if(pSelf->m_pClient->Client()->State() != IClient::STATE_ONLINE || pSelf->m_pClient->Client()->State() == IClient::STATE_CONNECTING)
    {
        return 0;
    }
    if (!pSelf->m_pClient->Collision())
        return 0;

    vec2 Pos1 = vec2(lua_tonumber(L, 1), lua_tonumber(L, 2));
    vec2 Pos2 = vec2(lua_tonumber(L, 3), lua_tonumber(L, 4));
    vec2 Out = vec2(0, 0);
    vec2 OutBefore = vec2(0, 0);
    lua_pushnumber(L, pSelf->m_pClient->Collision()->IntersectLine(Pos1, Pos2, &Out, &OutBefore));
    lua_pushnumber(L, Out.x);
    lua_pushnumber(L, Out.y);
    lua_pushnumber(L, OutBefore.x);
    lua_pushnumber(L, OutBefore.y);
    return 5;
}

int CLuaFile::MovePoint(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isnumber(L, 1) && !lua_isnumber(L, 2) && !lua_isnumber(L, 3) && !lua_isnumber(L, 4))
        return 0;
    vec2 Pos = vec2(lua_tonumber(L, 1), lua_tonumber(L, 2));
    vec2 Dir = vec2(lua_tonumber(L, 3), lua_tonumber(L, 4));
    float Elasticity = lua_isnumber(L, 5) ? lua_tonumber(L, 5) : 0.0f;
    pSelf->m_pClient->Collision()->MovePoint(&Pos, &Dir, Elasticity, 0);
    lua_pushnumber(L, Pos.x);
    lua_pushnumber(L, Pos.y);
    lua_pushnumber(L, Dir.x);
    lua_pushnumber(L, Dir.y);
    return 4;
}

int CLuaFile::MoveBox(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isnumber(L, 1) && !lua_isnumber(L, 2) && !lua_isnumber(L, 3) && !lua_isnumber(L, 4) && !lua_isnumber(L, 5) && !lua_isnumber(L, 6))
        return 0;
    vec2 Pos = vec2(lua_tonumber(L, 1), lua_tonumber(L, 2));
    vec2 Dir = vec2(lua_tonumber(L, 3), lua_tonumber(L, 4));
    vec2 Size = vec2(lua_tonumber(L, 5), lua_tonumber(L, 6));
    float Elasticity = lua_isnumber(L, 7) ? lua_tonumber(L, 7) : 0.0f;
    pSelf->m_pClient->Collision()->MoveBox(&Pos, &Dir, Size, Elasticity);
    lua_pushnumber(L, Pos.x);
    lua_pushnumber(L, Pos.y);
    lua_pushnumber(L, Dir.x);
    lua_pushnumber(L, Dir.y);
    return 4;
}

int CLuaFile::ClosestPointOnLine(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isnumber(L, 1) && !lua_isnumber(L, 2) && !lua_isnumber(L, 3) && !lua_isnumber(L, 4) && !lua_isnumber(L, 5) && !lua_isnumber(L, 6))
        return 0;
    vec2 Pos1 = vec2(lua_tonumber(L, 1), lua_tonumber(L, 2));
    vec2 Pos2 = vec2(lua_tonumber(L, 3), lua_tonumber(L, 4));
    vec2 Pos3 = vec2(lua_tonumber(L, 5), lua_tonumber(L, 6));
    vec2 Ret = closest_point_on_line(Pos1, Pos2, Pos3);
    lua_pushnumber(L, Ret.x);
    lua_pushnumber(L, Ret.y);
    return 2;
}

int CLuaFile::GetTile(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if(!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
        return 0;
    if(pSelf->m_pClient->Client()->State() < IClient::STATE_ONLINE)
    {
        return 0;
    }
    if (!pSelf->m_pClient->Collision())
        return 0;

    lua_pushnumber(L, pSelf->m_pClient->Collision()->GetTileRaw(lua_tonumber(L, 1), lua_tonumber(L, 2)));
    return 1;
}
int CLuaFile::SetTile(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if(!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
        return 0;

    if(pSelf->m_pClient->Client()->State() < IClient::STATE_ONLINE)
    {
        return 0;
    }
    if (!pSelf->m_pClient->Collision())
        return 0;

    pSelf->m_pClient->Collision()->SetTile(lua_tointeger(L, 1), lua_tointeger(L, 2), lua_tointeger(L, 3));
    return 1;
}
int CLuaFile::GetMapWidth(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if(pSelf->m_pClient->Client()->State() < IClient::STATE_ONLINE)
    {
        return 0;
    }
    if (!pSelf->m_pClient->Collision())
        return 0;

    lua_pushnumber(L, pSelf->m_pClient->Collision()->GetWidth());
    return 1;
}

int CLuaFile::GetMapHeight(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if(pSelf->m_pClient->Client()->State() < IClient::STATE_ONLINE)
    {
        return 0;
    }
    if (!pSelf->m_pClient->Collision())
        return 0;

    lua_pushnumber(L, pSelf->m_pClient->Collision()->GetHeight());
    return 1;
}

int CLuaFile::ChatSend(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (lua_isstring(L, 1))
    {
        pSelf->m_pClient->m_pChat->Say(0, lua_tostring(L, 1));
    }
    return 0;
}

int CLuaFile::ChatTeamSend(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (lua_isstring(L, 1))
    {
        pSelf->m_pClient->m_pChat->Say(1, lua_tostring(L, 1));
    }
    return 0;
}

int CLuaFile::CreateParticle(lua_State *L)
{
    LUA_FUNCTION_HEADER

    CParticle p;
    p.SetDefault();
    if (lua_isnumber(L, 1))
        p.m_Spr = lua_tonumber(L, 1);

    if (lua_isnumber(L, 2))
        p.m_Texture = lua_tonumber(L, 2);

    if (lua_isnumber(L, 3) && lua_isnumber(L, 4))
        p.m_Pos = vec2(lua_tonumber(L, 3), lua_tonumber(L, 4));

    if (lua_isnumber(L, 5) && lua_isnumber(L, 6))
        p.m_Vel = vec2(lua_tonumber(L, 5), lua_tonumber(L, 6));

    if (lua_isnumber(L, 7))
        p.m_LifeSpan = lua_tonumber(L, 7);

    if (lua_isnumber(L, 8))
        p.m_Rot = lua_tonumber(L, 8);

    if (lua_isnumber(L, 9))
        p.m_Rotspeed = lua_tonumber(L, 9);

    if (lua_isnumber(L, 10))
        p.m_StartSize = lua_tonumber(L, 10);

    if (lua_isnumber(L, 11))
        p.m_EndSize = lua_tonumber(L, 11);

    if (lua_isnumber(L, 12))
        p.m_Friction = lua_tonumber(L, 12);

    if (lua_isnumber(L, 13))
        p.m_Gravity.x = lua_tonumber(L, 13);

    if (lua_isnumber(L, 14))
        p.m_Gravity.y = lua_tonumber(L, 14);

    if (lua_isnumber(L, 15))
        p.m_FlowAffected = lua_tonumber(L, 15);

    if (lua_isnumber(L, 16) && lua_isnumber(L, 17) && lua_isnumber(L, 18) && lua_isnumber(L, 19))
        p.m_Color = vec4(lua_tonumber(L, 16), lua_tonumber(L, 17), lua_tonumber(L, 18), lua_tonumber(L, 19));

    if (lua_isnumber(L, 20) && lua_isnumber(L, 21) && lua_isnumber(L, 22) && lua_isnumber(L, 23))
        p.m_ColorEnd = vec4(lua_tonumber(L, 20), lua_tonumber(L, 21), lua_tonumber(L, 22), lua_tonumber(L, 23));

    if (pSelf->m_pClient->Client()->GameTick())
        pSelf->m_pClient->m_pParticles->Add(CParticles::GROUP_GENERAL, &p);
    return 0;
}

int CLuaFile::Print(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (lua_isstring(L, 1) && lua_isstring(L, 2))
        dbg_msg(lua_tostring(L, 1), lua_tostring(L, 2));
    return 0;
}

int CLuaFile::RconAuth(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (lua_isstring(L, 1))
        pSelf->m_pClient->Client()->RconAuth("", lua_tostring(L, 1));

    return 0;
}

int CLuaFile::RconAuthed(lua_State *L)
{
    LUA_FUNCTION_HEADER

    lua_pushboolean(L, pSelf->m_pClient->Client()->RconAuthed());

    return 1;
}

int CLuaFile::RconExecute(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (lua_isstring(L, 1))
        pSelf->m_pClient->Client()->Rcon(lua_tostring(L, 1));

    return 0;
}

int CLuaFile::GetGameType(lua_State *L)
{
    LUA_FUNCTION_HEADER

    CServerInfo CurrentServerInfo;
    pSelf->m_pClient->Client()->GetServerInfo(&CurrentServerInfo);
    lua_pushstring(L, CurrentServerInfo.m_aGameType);
    return 1;
}

int CLuaFile::IsTeamplay(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (pSelf->m_pClient->m_Snap.m_pGameInfoObj)
        lua_pushboolean(L, pSelf->m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags&GAMEFLAG_TEAMS);
    else
        lua_pushboolean(L, false);
    return 1;
}

int CLuaFile::GetNetError(lua_State *L)
{
    LUA_FUNCTION_HEADER

    lua_pushstring(L, pSelf->m_pClient->Client()->ErrorString());
    return 1;
}

int CLuaFile::Connect(lua_State *L)
{
    static CThrottle s_Throttle;
    LUA_FUNCTION_HEADER

    if(s_Throttle.Throttled(5))
    {
        if (lua_isstring(L, 1))
            pSelf->m_pClient->Client()->Connect(lua_tostring(L, 1));
        else
            pSelf->m_pClient->Client()->Connect(g_Config.m_UiServerAddress);
    }
    return 0;
}

int CLuaFile::StateGet(lua_State *L)
{
    LUA_FUNCTION_HEADER

    lua_pushboolean(L, pSelf->m_pClient->Client()->State());
    return 1;
}

int CLuaFile::StateOnline(lua_State *L)
{
    LUA_FUNCTION_HEADER

    lua_pushboolean(L, pSelf->m_pClient->Client()->State() == IClient::STATE_ONLINE);
    return 1;
}

int CLuaFile::StateOffline(lua_State *L)
{
    LUA_FUNCTION_HEADER

    lua_pushboolean(L, pSelf->m_pClient->Client()->State() == IClient::STATE_OFFLINE);
    return 1;
}

int CLuaFile::StateConnecting(lua_State *L)
{
    LUA_FUNCTION_HEADER

    lua_pushboolean(L, pSelf->m_pClient->Client()->State() == IClient::STATE_CONNECTING);
    return 1;
}

int CLuaFile::StateLoading(lua_State *L)
{
    LUA_FUNCTION_HEADER

    lua_pushboolean(L, pSelf->m_pClient->Client()->State() == IClient::STATE_LOADING);
    return 1;
}

int CLuaFile::StateDemoplayback(lua_State *L)
{
    LUA_FUNCTION_HEADER

    lua_pushboolean(L, pSelf->m_pClient->Client()->State() == IClient::STATE_DEMOPLAYBACK);
    return 1;
}

int CLuaFile::MenuActive(lua_State *L)
{
    LUA_FUNCTION_HEADER

    lua_pushboolean(L, pSelf->m_pClient->m_pMenus->IsActive());
    return 1;
}

int CLuaFile::MenuGameActive(lua_State *L)
{
    LUA_FUNCTION_HEADER

    lua_pushboolean(L, pSelf->m_pClient->m_pMenus->IsActive() && pSelf->m_pClient->m_pMenus->GetGamePage() == CMenus::PAGE_GAME);
    return 1;
}

int CLuaFile::MenuPlayersActive(lua_State *L)
{
    LUA_FUNCTION_HEADER

    lua_pushboolean(L, pSelf->m_pClient->m_pMenus->IsActive() && pSelf->m_pClient->m_pMenus->GetGamePage() == CMenus::PAGE_PLAYERS);
    return 1;
}

int CLuaFile::MenuServerInfoActive(lua_State *L)
{
    LUA_FUNCTION_HEADER

    lua_pushboolean(L, pSelf->m_pClient->m_pMenus->IsActive() && pSelf->m_pClient->m_pMenus->GetGamePage() == CMenus::PAGE_SERVER_INFO);
    return 1;
}

int CLuaFile::MenuCallVoteActive(lua_State *L)
{
    LUA_FUNCTION_HEADER

    lua_pushboolean(L, pSelf->m_pClient->m_pMenus->IsActive() && pSelf->m_pClient->m_pMenus->GetGamePage() == CMenus::PAGE_CALLVOTE);
    return 1;
}

int CLuaFile::MenuServersActive(lua_State *L)
{
    LUA_FUNCTION_HEADER

    lua_pushboolean(L, pSelf->m_pClient->m_pMenus->IsActive() && pSelf->m_pClient->m_pMenus->GetGamePage() == CMenus::PAGE_SERVERS);
    return 1;
}

int CLuaFile::MenuMusicActive(lua_State *L)
{
    LUA_FUNCTION_HEADER

    lua_pushboolean(L, pSelf->m_pClient->m_pMenus->IsActive() && pSelf->m_pClient->m_pMenus->GetGamePage() == CMenus::PAGE_MUSIC);
    return 1;
}

int CLuaFile::MenuDemosActive(lua_State *L)
{
    LUA_FUNCTION_HEADER

    lua_pushboolean(L, pSelf->m_pClient->m_pMenus->IsActive() && pSelf->m_pClient->m_pMenus->GetGamePage() == CMenus::PAGE_DEMOS);
    return 1;
}

int CLuaFile::GetMousePosMenu(lua_State *L)
{
    LUA_FUNCTION_HEADER

    //convert the pos to a ui mouse pos
    lua_pushnumber(L, pSelf->m_pClient->m_pMenus->GetMousePos().x * pSelf->m_pClient->UI()->Screen()->w / pSelf->m_pClient->Graphics()->ScreenWidth());
    lua_pushnumber(L, pSelf->m_pClient->m_pMenus->GetMousePos().y * pSelf->m_pClient->UI()->Screen()->h / pSelf->m_pClient->Graphics()->ScreenHeight());
    return 2;
}

int CLuaFile::SetMouseModeRelative(lua_State *L)
{
    LUA_FUNCTION_HEADER

    pSelf->m_pClient->m_pLua->m_MouseModeAbsolute = false;
    return 1;
}

int CLuaFile::SetMouseModeAbsolute(lua_State *L)
{
    LUA_FUNCTION_HEADER

    pSelf->m_pClient->m_pLua->m_MouseModeAbsolute = true;
    return 1;
}

int CLuaFile::UiRemoveElement(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isnumber(L, 1))
        return 0;

    int i = lua_tonumber(L, 1);
    if (i >= 0 && i < LUAMAXUIELEMENTS)
    {
        pSelf->m_aUiElements[i].m_Used = false;
    }
    return 0;
}

int CLuaFile::UiGetText(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isnumber(L, 1))
        return 0;

    int i = lua_tonumber(L, 1);
    if (i >= 0 && i < LUAMAXUIELEMENTS)
    {
        lua_pushstring(L, pSelf->m_aUiElements[i].m_pText);
        return 1;
    }
    return 0;
}

int CLuaFile::UiSetText(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isnumber(L, 1))
        return 0;
    if (!lua_isstring(L, 2))
        return 0;

    int i = lua_tonumber(L, 1);
    if (i >= 0 && i < LUAMAXUIELEMENTS)
    {
        str_copy(pSelf->m_aUiElements[i].m_pText, lua_tostring(L, 2), sizeof(pSelf->m_aUiElements[i].m_pText));
    }
    return 0;
}

int CLuaFile::UiGetColor(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isnumber(L, 1))
        return 0;

    int i = lua_tonumber(L, 1);
    if (i >= 0 && i < LUAMAXUIELEMENTS)
    {
        lua_pushnumber(L, pSelf->m_aUiElements[i].m_Color.r);
        lua_pushnumber(L, pSelf->m_aUiElements[i].m_Color.g);
        lua_pushnumber(L, pSelf->m_aUiElements[i].m_Color.b);
        lua_pushnumber(L, pSelf->m_aUiElements[i].m_Color.a);
        return 4;
    }
    return 0;
}

int CLuaFile::UiSetColor(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isnumber(L, 1))
        return 0;
    if (!lua_isnumber(L, 2))
        return 0;
    if (!lua_isnumber(L, 3))
        return 0;
    if (!lua_isnumber(L, 4))
        return 0;
    if (!lua_isnumber(L, 5))
        return 0;

    int i = lua_tonumber(L, 1);
    if (i >= 0 && i < LUAMAXUIELEMENTS)
    {
        pSelf->m_aUiElements[i].m_Color.r = lua_tonumber(L, 2);
        pSelf->m_aUiElements[i].m_Color.g = lua_tonumber(L, 3);
        pSelf->m_aUiElements[i].m_Color.b = lua_tonumber(L, 4);
        pSelf->m_aUiElements[i].m_Color.a = lua_tonumber(L, 5);
    }
    return 0;
}

int CLuaFile::UiGetRect(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isnumber(L, 1))
        return 0;

    int i = lua_tonumber(L, 1);
    if (i >= 0 && i < LUAMAXUIELEMENTS)
    {
        lua_pushnumber(L, pSelf->m_aUiElements[i].m_Rect.x);
        lua_pushnumber(L, pSelf->m_aUiElements[i].m_Rect.y);
        lua_pushnumber(L, pSelf->m_aUiElements[i].m_Rect.w);
        lua_pushnumber(L, pSelf->m_aUiElements[i].m_Rect.h);
        return 4;
    }
    return 0;
}

int CLuaFile::UiSetRect(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isnumber(L, 1))
        return 0;

    int i = lua_tonumber(L, 1);
    if (i >= 0 && i < LUAMAXUIELEMENTS)
    {
        pSelf->m_aUiElements[i].m_Rect.x = lua_isnumber(L, 2) ? lua_tonumber(L, 2) : pSelf->m_aUiElements[i].m_Rect.x;
        pSelf->m_aUiElements[i].m_Rect.y = lua_isnumber(L, 3) ? lua_tonumber(L, 3) : pSelf->m_aUiElements[i].m_Rect.y;
        pSelf->m_aUiElements[i].m_Rect.w = lua_isnumber(L, 4) ? lua_tonumber(L, 4) : pSelf->m_aUiElements[i].m_Rect.w;
        pSelf->m_aUiElements[i].m_Rect.h = lua_isnumber(L, 5) ? lua_tonumber(L, 5) : pSelf->m_aUiElements[i].m_Rect.h;
    }
    return 0;
}

int CLuaFile::UiDoButton(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isnumber(L, 1))
        return 0;
    if (!lua_isnumber(L, 2))
        return 0;
    if (!lua_isnumber(L, 3))
        return 0;
    if (!lua_isnumber(L, 4))
        return 0;
    if (!lua_isnumber(L, 5))
        return 0;
    if (!lua_isstring(L, 6))
        return 0;
    if (!lua_isstring(L, 7))
        return 0;

    int i = 0;
    for (i = 0; i < LUAMAXUIELEMENTS; i++)
    {
        if (pSelf->m_aUiElements[i].m_Used == 0)
        {
            break;
        }
    }
    if (i >= LUAMAXUIELEMENTS)
        return 0;

    pSelf->m_aUiElements[i].m_Used = true;
    pSelf->m_aUiElements[i].m_Rect.x = lua_tonumber(L, 1);
    pSelf->m_aUiElements[i].m_Rect.y = lua_tonumber(L, 2);
    pSelf->m_aUiElements[i].m_Rect.w = lua_tonumber(L, 3);
    pSelf->m_aUiElements[i].m_Rect.h = lua_tonumber(L, 4);
    pSelf->m_aUiElements[i].m_RegPoint = lua_tonumber(L, 5);
    str_copy(pSelf->m_aUiElements[i].m_pText, lua_tostring(L, 6), sizeof(pSelf->m_aUiElements[i].m_pText));
    str_copy(pSelf->m_aUiElements[i].m_pCallback, lua_tostring(L, 7), sizeof(pSelf->m_aUiElements[i].m_pCallback));
    if (lua_isnumber(L, 8))
        pSelf->m_aUiElements[i].m_Checked = lua_tonumber(L, 8);
    else
        pSelf->m_aUiElements[i].m_Checked = 0;

    if (lua_isnumber(L, 9))
        pSelf->m_aUiElements[i].m_Corners = lua_tonumber(L, 9);
    else
        pSelf->m_aUiElements[i].m_Corners = CUI::CORNER_ALL;

    if (lua_isnumber(L, 10) && lua_isnumber(L, 11) && lua_isnumber(L, 12) && lua_isnumber(L, 13))
        pSelf->m_aUiElements[i].m_Color = vec4(lua_tonumber(L, 10), lua_tonumber(L, 11), lua_tonumber(L, 12), lua_tonumber(L, 13));
    else
        pSelf->m_aUiElements[i].m_Color = vec4(1.0f, 1.0f, 1.0f, 0.5f);

    pSelf->m_aUiElements[i].m_pClient = pSelf->m_pClient;
    pSelf->m_aUiElements[i].m_pLuaFile = pSelf;
    pSelf->m_aUiElements[i].m_Type = CLuaUi::LUAUIBUTTON;

    lua_pushinteger(L, i);

    return 1;
}

int CLuaFile::UiDoEditBox(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isnumber(L, 1))
        return 0;
    if (!lua_isnumber(L, 2))
        return 0;
    if (!lua_isnumber(L, 3))
        return 0;
    if (!lua_isnumber(L, 4))
        return 0;
    if (!lua_isnumber(L, 5))
        return 0;

    int i = 0;
    for (i = 0; i < LUAMAXUIELEMENTS; i++)
    {
        if (pSelf->m_aUiElements[i].m_Used == 0)
        {
            break;
        }
    }
    if (i >= LUAMAXUIELEMENTS)
        return 0;

    pSelf->m_aUiElements[i].m_Used = true;
    pSelf->m_aUiElements[i].m_Rect.x = lua_tonumber(L, 1);
    pSelf->m_aUiElements[i].m_Rect.y = lua_tonumber(L, 2);
    pSelf->m_aUiElements[i].m_Rect.w = lua_tonumber(L, 3);
    pSelf->m_aUiElements[i].m_Rect.h = lua_tonumber(L, 4);
    pSelf->m_aUiElements[i].m_RegPoint = lua_tonumber(L, 5);

    if (lua_isstring(L, 6))
        str_copy(pSelf->m_aUiElements[i].m_pText, lua_tostring(L, 6), sizeof(pSelf->m_aUiElements[i].m_pText));
    else
        pSelf->m_aUiElements[i].m_pText[0] = 0;

    if (lua_isnumber(L, 7))
        pSelf->m_aUiElements[i].m_FontSize = lua_tonumber(L, 7);
    else
        pSelf->m_aUiElements[i].m_FontSize = 14.0f;

    if (lua_isnumber(L, 8))
        pSelf->m_aUiElements[i].m_Hidden = lua_toboolean(L, 8);
    else
        pSelf->m_aUiElements[i].m_Hidden = false;

    if (lua_isnumber(L, 9))
        pSelf->m_aUiElements[i].m_Corners = lua_tonumber(L, 9);
    else
        pSelf->m_aUiElements[i].m_Corners = CUI::CORNER_ALL;

    if (lua_isnumber(L, 10) && lua_isnumber(L, 11) && lua_isnumber(L, 12) && lua_isnumber(L, 13))
        pSelf->m_aUiElements[i].m_Color = vec4(lua_tonumber(L, 10), lua_tonumber(L, 11), lua_tonumber(L, 12), lua_tonumber(L, 13));
    else
        pSelf->m_aUiElements[i].m_Color = vec4(1.0f, 1.0f, 1.0f, 0.5f);

    pSelf->m_aUiElements[i].m_pClient = pSelf->m_pClient;
    pSelf->m_aUiElements[i].m_pLuaFile = pSelf;
    pSelf->m_aUiElements[i].m_Type = CLuaUi::LUAUIEDITBOX;
    pSelf->m_aUiElements[i].m_Id = i;

    lua_pushinteger(L, i);

    return 1;
}

int CLuaFile::UiDoLabel(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isnumber(L, 1))
        return 0;
    if (!lua_isnumber(L, 2))
        return 0;
    if (!lua_isnumber(L, 3))
        return 0;
    if (!lua_isnumber(L, 4))
        return 0;
    if (!lua_isnumber(L, 5))
        return 0;


    int i = 0;
    for (i = 0; i < LUAMAXUIELEMENTS; i++)
    {
        if (pSelf->m_aUiElements[i].m_Used == 0)
        {
            break;
        }
    }
    if (i >= LUAMAXUIELEMENTS)
        return 0;

    pSelf->m_aUiElements[i].m_Used = true;
    pSelf->m_aUiElements[i].m_Rect.x = lua_tonumber(L, 1);
    pSelf->m_aUiElements[i].m_Rect.y = lua_tonumber(L, 2);
    pSelf->m_aUiElements[i].m_Rect.w = lua_tonumber(L, 3);
    pSelf->m_aUiElements[i].m_Rect.h = lua_tonumber(L, 4);
    pSelf->m_aUiElements[i].m_RegPoint = lua_tonumber(L, 5);

    if (lua_isstring(L, 6))
        str_copy(pSelf->m_aUiElements[i].m_pText, lua_tostring(L, 6), sizeof(pSelf->m_aUiElements[i].m_pText));
    else
        pSelf->m_aUiElements[i].m_pText[0] = 0;

    if (lua_isnumber(L, 7))
        pSelf->m_aUiElements[i].m_FontSize = lua_tonumber(L, 7);
    else
        pSelf->m_aUiElements[i].m_FontSize = 14.0f;

    if (lua_isnumber(L, 8))
        pSelf->m_aUiElements[i].m_Align = lua_tonumber(L, 8);
    else
        pSelf->m_aUiElements[i].m_Align = 0;

    if (lua_isnumber(L, 9) && lua_isnumber(L, 10) && lua_isnumber(L, 11) && lua_isnumber(L, 12))
        pSelf->m_aUiElements[i].m_Color = vec4(lua_tonumber(L, 9), lua_tonumber(L, 10), lua_tonumber(L, 11), lua_tonumber(L, 12));
    else
        pSelf->m_aUiElements[i].m_Color = vec4(1, 1, 1, 1);

    pSelf->m_aUiElements[i].m_pClient = pSelf->m_pClient;
    pSelf->m_aUiElements[i].m_pLuaFile = pSelf;
    pSelf->m_aUiElements[i].m_Type = CLuaUi::LUAUILABEL;
    pSelf->m_aUiElements[i].m_Id = i;

    lua_pushinteger(L, i);

    return 1;
}

int CLuaFile::UiDoRect(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isnumber(L, 1))
        return 0;
    if (!lua_isnumber(L, 2))
        return 0;
    if (!lua_isnumber(L, 3))
        return 0;
    if (!lua_isnumber(L, 4))
        return 0;
    if (!lua_isnumber(L, 5))
        return 0;


    int i = 0;
    for (i = 0; i < LUAMAXUIELEMENTS; i++)
    {
        if (pSelf->m_aUiElements[i].m_Used == 0)
        {
            break;
        }
    }
    if (i >= LUAMAXUIELEMENTS)
        return 0;

    pSelf->m_aUiElements[i].m_Used = true;
    pSelf->m_aUiElements[i].m_Rect.x = lua_tonumber(L, 1);
    pSelf->m_aUiElements[i].m_Rect.y = lua_tonumber(L, 2);
    pSelf->m_aUiElements[i].m_Rect.w = lua_tonumber(L, 3);
    pSelf->m_aUiElements[i].m_Rect.h = lua_tonumber(L, 4);
    pSelf->m_aUiElements[i].m_RegPoint = lua_tonumber(L, 5);

    if (lua_isnumber(L, 6))
        pSelf->m_aUiElements[i].m_Corners = lua_tonumber(L, 6);
    else
        pSelf->m_aUiElements[i].m_Corners = CUI::CORNER_ALL;

    if (lua_isnumber(L, 7))
        pSelf->m_aUiElements[i].m_Rounding = lua_tonumber(L, 7);
    else
        pSelf->m_aUiElements[i].m_Rounding = 5.0f;

    if (lua_isnumber(L, 8) && lua_isnumber(L, 9) && lua_isnumber(L, 10) && lua_isnumber(L, 11))
        pSelf->m_aUiElements[i].m_Color = vec4(lua_tonumber(L, 8), lua_tonumber(L, 9), lua_tonumber(L, 10), lua_tonumber(L, 11));
    else
        pSelf->m_aUiElements[i].m_Color = vec4(0.0f, 0.0f, 0.0f, 0.5f);

    pSelf->m_aUiElements[i].m_pClient = pSelf->m_pClient;
    pSelf->m_aUiElements[i].m_pLuaFile = pSelf;
    pSelf->m_aUiElements[i].m_Type = CLuaUi::LUAUIRECT;
    pSelf->m_aUiElements[i].m_Id = i;

    lua_pushinteger(L, i);

    return 1;
}

int CLuaFile::UiDoImage(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isnumber(L, 1))
        return 0;
    if (!lua_isnumber(L, 2))
        return 0;
    if (!lua_isnumber(L, 3))
        return 0;
    if (!lua_isnumber(L, 4))
        return 0;
    if (!lua_isnumber(L, 5))
        return 0;
    if (!lua_isnumber(L, 6))
        return 0;
    if (!lua_isnumber(L, 7))
        return 0;
    if (!lua_isstring(L, 8))
        return 0;



    int i = 0;
    for (i = 0; i < LUAMAXUIELEMENTS; i++)
    {
        if (pSelf->m_aUiElements[i].m_Used == 0)
        {
            break;
        }
    }
    if (i >= LUAMAXUIELEMENTS)
        return 0;

    pSelf->m_aUiElements[i].m_Used = true;
    pSelf->m_aUiElements[i].m_Rect.x = lua_tonumber(L, 1);
    pSelf->m_aUiElements[i].m_Rect.y = lua_tonumber(L, 2);
    pSelf->m_aUiElements[i].m_Rect.w = lua_tonumber(L, 3);
    pSelf->m_aUiElements[i].m_Rect.h = lua_tonumber(L, 4);
    pSelf->m_aUiElements[i].m_RegPoint = lua_tonumber(L, 5);

    pSelf->m_aUiElements[i].m_TextureID = lua_tonumber(L, 6);
    pSelf->m_aUiElements[i].m_SpriteID = lua_tonumber(L, 7);
    str_copy(pSelf->m_aUiElements[i].m_pCallback, lua_tostring(L, 8), sizeof(pSelf->m_aUiElements[i].m_pCallback));

    pSelf->m_aUiElements[i].m_pClient = pSelf->m_pClient;
    pSelf->m_aUiElements[i].m_pLuaFile = pSelf;
    pSelf->m_aUiElements[i].m_Type = CLuaUi::LUAUIIMAGE;
    pSelf->m_aUiElements[i].m_Id = i;

    lua_pushinteger(L, i);

    return 1;
}

int CLuaFile::UiDoImageEx(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isnumber(L, 1))
        return 0;
    if (!lua_isnumber(L, 2))
        return 0;
    // 3 to 4 optional
    if (!lua_isnumber(L, 5))
        return 0;
    if (!lua_isnumber(L, 6))
        return 0;
    // 7 to 10 optional (clipping)
    if (!lua_isstring(L, 11))
        return 0;



    int i = 0;
    for (i = 0; i < LUAMAXUIELEMENTS; i++)
    {
        if (pSelf->m_aUiElements[i].m_Used == 0)
        {
            break;
        }
    }
    if (i >= LUAMAXUIELEMENTS)
        return 0;

    pSelf->m_aUiElements[i].m_Used = true;
    pSelf->m_aUiElements[i].m_Rect.x = lua_tonumber(L, 1);
    pSelf->m_aUiElements[i].m_Rect.y = lua_tonumber(L, 2);
    if (lua_isnumber(L, 3))
        pSelf->m_aUiElements[i].m_Rect.w = lua_tonumber(L, 3);
    else
        pSelf->m_aUiElements[i].m_Rect.w = pSelf->m_pClient->Graphics()->GetTextureWidth(lua_tointeger(L, 6));
    if (lua_isnumber(L, 4))
        pSelf->m_aUiElements[i].m_Rect.h = lua_tonumber(L, 4);
    else
        pSelf->m_aUiElements[i].m_Rect.h = pSelf->m_pClient->Graphics()->GetTextureHeight(lua_tointeger(L, 6));
    pSelf->m_aUiElements[i].m_RegPoint = lua_tonumber(L, 5);

    pSelf->m_aUiElements[i].m_TextureID = lua_tonumber(L, 6);

    int ImgWidth = pSelf->m_pClient->Graphics()->GetTextureWidth(lua_tointeger(L, 6));
    int ImgHeight = pSelf->m_pClient->Graphics()->GetTextureHeight(lua_tointeger(L, 6));
    float ClipX1 = 0.0f;
    float ClipY1 = 0.0f;
    float ClipX2 = 1.0f;
    float ClipY2 = 1.0f;
    if (lua_isnumber(L, 7))
        ClipX1 = clamp((float)lua_tointeger(L, 7) / (float)ImgWidth, 0.0f, 1.0f);
    if (lua_isnumber(L, 8))
        ClipY1 = clamp((float)lua_tointeger(L, 8) / (float)ImgHeight, 0.0f, 1.0f);
    if (lua_isnumber(L, 9))
        ClipX2 = clamp((float)lua_tointeger(L, 9) / (float)ImgWidth, 0.0f, 1.0f);
    if (lua_isnumber(L, 10))
        ClipY2 = clamp((float)lua_tointeger(L, 10) / (float)ImgHeight, 0.0f, 1.0f);

    pSelf->m_aUiElements[i].m_ClipX1 = ClipX1;
    pSelf->m_aUiElements[i].m_ClipY1 = ClipY1;
    pSelf->m_aUiElements[i].m_ClipX2 = ClipX2;
    pSelf->m_aUiElements[i].m_ClipY2 = ClipY2;
    str_copy(pSelf->m_aUiElements[i].m_pCallback, lua_tostring(L, 11), sizeof(pSelf->m_aUiElements[i].m_pCallback));

    pSelf->m_aUiElements[i].m_pClient = pSelf->m_pClient;
    pSelf->m_aUiElements[i].m_pLuaFile = pSelf;
    pSelf->m_aUiElements[i].m_Type = CLuaUi::LUAUIIMAGEEX;
    pSelf->m_aUiElements[i].m_Id = i;

    lua_pushinteger(L, i);

    return 1;
}

int CLuaFile::UiDoLine(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isnumber(L, 1)) //x1
        return 0;
    if (!lua_isnumber(L, 2)) //y1
        return 0;
    if (!lua_isnumber(L, 3)) //x2
        return 0;
    if (!lua_isnumber(L, 4)) //y2
        return 0;
    if (!lua_isnumber(L, 5))
        return 0;
    if (!lua_isnumber(L, 6))
        return 0;
    if (!lua_isnumber(L, 7))
        return 0;
    if (!lua_isnumber(L, 8))
        return 0;



    int i = 0;
    for (i = 0; i < LUAMAXUIELEMENTS; i++)
    {
        if (pSelf->m_aUiElements[i].m_Used == 0)
        {
            break;
        }
    }
    if (i >= LUAMAXUIELEMENTS)
        return 0;

    pSelf->m_aUiElements[i].m_Used = true;
    pSelf->m_aUiElements[i].m_Rect.x = lua_tonumber(L, 1);
    pSelf->m_aUiElements[i].m_Rect.y = lua_tonumber(L, 2);
    pSelf->m_aUiElements[i].m_Rect.w = lua_tonumber(L, 3);
    pSelf->m_aUiElements[i].m_Rect.h = lua_tonumber(L, 4);

    pSelf->m_aUiElements[i].m_Color = vec4(lua_tonumber(L, 5), lua_tonumber(L, 6), lua_tonumber(L, 7), lua_tonumber(L, 8));

    pSelf->m_aUiElements[i].m_pClient = pSelf->m_pClient;
    pSelf->m_aUiElements[i].m_pLuaFile = pSelf;
    pSelf->m_aUiElements[i].m_Type = CLuaUi::LUAUILINE;
    pSelf->m_aUiElements[i].m_Id = i;

    lua_pushinteger(L, i);

    return 1;
}

int CLuaFile::UiDoSlider(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isnumber(L, 1))
        return 0;
    if (!lua_isnumber(L, 2))
        return 0;
    if (!lua_isnumber(L, 3))
        return 0;
    if (!lua_isnumber(L, 4))
        return 0;
    if (!lua_isnumber(L, 5))
        return 0;
    if (!lua_isnumber(L, 6))
        return 0;
    if (!lua_isstring(L, 7))
        return 0;


    int i = 0;
    for (i = 0; i < LUAMAXUIELEMENTS; i++)
    {
        if (pSelf->m_aUiElements[i].m_Used == 0)
        {
            break;
        }
    }
    if (i >= LUAMAXUIELEMENTS)
        return 0;

    pSelf->m_aUiElements[i].m_Used = true;
    pSelf->m_aUiElements[i].m_Rect.x = lua_tonumber(L, 1);
    pSelf->m_aUiElements[i].m_Rect.y = lua_tonumber(L, 2);
    pSelf->m_aUiElements[i].m_Rect.w = lua_tonumber(L, 3);
    pSelf->m_aUiElements[i].m_Rect.h = lua_tonumber(L, 4);
    pSelf->m_aUiElements[i].m_RegPoint = lua_tonumber(L, 5);

    pSelf->m_aUiElements[i].m_Value = lua_tonumber(L, 6);
    str_copy(pSelf->m_aUiElements[i].m_pCallback, lua_tostring(L, 7), sizeof(pSelf->m_aUiElements[i].m_pCallback));

    if (lua_isnumber(L, 8) && lua_isnumber(L, 9) && lua_isnumber(L, 10) && lua_isnumber(L, 11))
        pSelf->m_aUiElements[i].m_Color = vec4(lua_tonumber(L, 8), lua_tonumber(L, 9), lua_tonumber(L, 10), lua_tonumber(L, 11));
    else
        pSelf->m_aUiElements[i].m_Color = vec4(1,1,1,0.25f);

    if (lua_isnumber(L, 12))
        pSelf->m_aUiElements[i].m_Direction = lua_tonumber(L, 12);
    else
        pSelf->m_aUiElements[i].m_Direction = 0;

    pSelf->m_aUiElements[i].m_pClient = pSelf->m_pClient;
    pSelf->m_aUiElements[i].m_pLuaFile = pSelf;
    pSelf->m_aUiElements[i].m_Type = CLuaUi::LUAUISLIDER;
    pSelf->m_aUiElements[i].m_Id = i;

    lua_pushinteger(L, i);

    return 1;
}

int CLuaFile::UiDirectRect(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
        return 0;

    CUIRect Rect;
    vec4 Color = vec4(0, 0, 0, 0.5f);
    int Corners = CUI::CORNER_ALL;
    int Rounding = 5.0f;

    Rect.x = lua_tonumber(L, 1);
    Rect.y = lua_tonumber(L, 2);
    Rect.w = lua_tonumber(L, 3);
    Rect.h = lua_tonumber(L, 4);

    if (lua_isnumber(L, 5) && lua_isnumber(L, 6) && lua_isnumber(L, 7) && lua_isnumber(L, 8))
        Color = vec4(lua_tonumber(L, 5), lua_tonumber(L, 6), lua_tonumber(L, 7), lua_tonumber(L, 8));

    if (lua_isnumber(L, 9))
        Corners = lua_tointeger(L, 9);

    if (lua_isnumber(L, 10))
        Rounding = lua_tonumber(L, 10);

    pSelf->m_pClient->RenderTools()->DrawUIRect(&Rect, Color, Corners, Rounding);

    return 0;
}

int CLuaFile::UiDirectEditBox(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
        return 0;

    CUIRect Rect;
    vec4 Color = vec4(0, 0, 0, 0.5f);
    int Corners = CUI::CORNER_ALL;
    int Rounding = 5.0f;

    Rect.x = lua_tonumber(L, 1);
    Rect.y = lua_tonumber(L, 2);
    Rect.w = lua_tonumber(L, 3);
    Rect.h = lua_tonumber(L, 4);

    if (lua_isnumber(L, 5) && lua_isnumber(L, 6) && lua_isnumber(L, 7) && lua_isnumber(L, 8))
        Color = vec4(lua_tonumber(L, 5), lua_tonumber(L, 6), lua_tonumber(L, 7), lua_tonumber(L, 8));

    if (lua_isnumber(L, 9))
        Corners = lua_tointeger(L, 9);

    if (lua_isnumber(L, 10))
        Rounding = lua_tonumber(L, 10);

    pSelf->m_pClient->RenderTools()->DrawUIRect(&Rect, Color, Corners, Rounding);

    return 0;
}

int CLuaFile::UiDirectSlider(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
        return 0;

    CUIRect Rect;
    vec4 Color = vec4(0, 0, 0, 0.5f);
    int Corners = CUI::CORNER_ALL;
    int Rounding = 5.0f;

    Rect.x = lua_tonumber(L, 1);
    Rect.y = lua_tonumber(L, 2);
    Rect.w = lua_tonumber(L, 3);
    Rect.h = lua_tonumber(L, 4);

    if (lua_isnumber(L, 5) && lua_isnumber(L, 6) && lua_isnumber(L, 7) && lua_isnumber(L, 8))
        Color = vec4(lua_tonumber(L, 5), lua_tonumber(L, 6), lua_tonumber(L, 7), lua_tonumber(L, 8));

    if (lua_isnumber(L, 9))
        Corners = lua_tointeger(L, 9);

    if (lua_isnumber(L, 10))
        Rounding = lua_tonumber(L, 10);

    pSelf->m_pClient->RenderTools()->DrawUIRect(&Rect, Color, Corners, Rounding);

    return 0;
}

int CLuaFile::UiDirectButton(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
        return 0;

    CUIRect Rect;
    vec4 Color = vec4(0, 0, 0, 0.5f);
    int Corners = CUI::CORNER_ALL;
    int Rounding = 5.0f;

    Rect.x = lua_tonumber(L, 1);
    Rect.y = lua_tonumber(L, 2);
    Rect.w = lua_tonumber(L, 3);
    Rect.h = lua_tonumber(L, 4);

    if (lua_isnumber(L, 5) && lua_isnumber(L, 6) && lua_isnumber(L, 7) && lua_isnumber(L, 8))
        Color = vec4(lua_tonumber(L, 5), lua_tonumber(L, 6), lua_tonumber(L, 7), lua_tonumber(L, 8));

    if (lua_isnumber(L, 9))
        Corners = lua_tointeger(L, 9);

    if (lua_isnumber(L, 10))
        Rounding = lua_tonumber(L, 10);

    pSelf->m_pClient->RenderTools()->DrawUIRect(&Rect, Color, Corners, Rounding);

    return 0;
}

//high speed rendering
int CLuaFile::UiDirectRectArray(lua_State *L)
{
    LUA_FUNCTION_HEADER


    pSelf->m_pClient->Graphics()->TextureSet(-1);
    pSelf->m_pClient->Graphics()->QuadsBegin();

    int64 Test = time_get();
    int iX = 0;
    lua_pushvalue(L, 1);
    lua_pushnil(L);
    while(lua_next(L, -2) != 0)
    {
        CUIRect Rect;
        mem_zero(&Rect, sizeof(Rect));
        vec4 Color = vec4(0, 0, 0, 0.5f);
        int Corners = CUI::CORNER_ALL;
        float Rounding = 5.0f;

        if (lua_istable(L, -1))
        {
            lua_pushvalue(L, -1);
            lua_pushnil(L);
            while(lua_next(L, -2) != 0)
            {
                if (str_comp(lua_tostring(L, -2), "x") == 0)
                    Rect.x = lua_tonumber(L, -1);
                if (str_comp(lua_tostring(L, -2), "y") == 0)
                    Rect.y = lua_tonumber(L, -1);
                if (str_comp(lua_tostring(L, -2), "h") == 0)
                    Rect.h = lua_tonumber(L, -1);
                if (str_comp(lua_tostring(L, -2), "w") == 0)
                    Rect.w = lua_tonumber(L, -1);
                if (str_comp(lua_tostring(L, -2), "r") == 0)
                    Color.r = lua_tonumber(L, -1);
                if (str_comp(lua_tostring(L, -2), "g") == 0)
                    Color.g = lua_tonumber(L, -1);
                if (str_comp(lua_tostring(L, -2), "b") == 0)
                    Color.b = lua_tonumber(L, -1);
                if (str_comp(lua_tostring(L, -2), "a") == 0)
                    Color.a = lua_tonumber(L, -1);
                if (str_comp(lua_tostring(L, -2), "corners") == 0)
                    Corners = lua_tonumber(L, -1);
                if (str_comp(lua_tostring(L, -2), "round") == 0)
                    Rounding = lua_tonumber(L, -1);
                lua_pop(L, 1);
            }
            lua_pop(L, 1);
        }
        pSelf->m_pClient->Graphics()->SetColor(Color.r, Color.g, Color.b, Color.a);
        {
            if (Rounding && Corners)
            {
                static IGraphics::CFreeformItem ArrayF[32];
                int NumItems = 0;
                for(int i = 0; i < 8; i+=2)
                {
                    float a1 = i/(float)8 * pi/2;
                    float a2 = (i+1)/(float)8 * pi/2;
                    float a3 = (i+2)/(float)8 * pi/2;
                    float Ca1 = cosf(a1);
                    float Ca2 = cosf(a2);
                    float Ca3 = cosf(a3);
                    float Sa1 = sinf(a1);
                    float Sa2 = sinf(a2);
                    float Sa3 = sinf(a3);

                    if(Corners&1) // TL
                        ArrayF[NumItems++] = IGraphics::CFreeformItem(
                                                 Rect.x+Rounding, Rect.y+Rounding,
                                                 Rect.x+(1-Ca1)*Rounding, Rect.y+(1-Sa1)*Rounding,
                                                 Rect.x+(1-Ca3)*Rounding, Rect.y+(1-Sa3)*Rounding,
                                                 Rect.x+(1-Ca2)*Rounding, Rect.y+(1-Sa2)*Rounding);

                    if(Corners&2) // TR
                        ArrayF[NumItems++] = IGraphics::CFreeformItem(
                                                 Rect.x+Rect.w-Rounding, Rect.y+Rounding,
                                                 Rect.x+Rect.w-Rounding+Ca1*Rounding, Rect.y+(1-Sa1)*Rounding,
                                                 Rect.x+Rect.w-Rounding+Ca3*Rounding, Rect.y+(1-Sa3)*Rounding,
                                                 Rect.x+Rect.w-Rounding+Ca2*Rounding, Rect.y+(1-Sa2)*Rounding);

                    if(Corners&4) // BL
                        ArrayF[NumItems++] = IGraphics::CFreeformItem(
                                                 Rect.x+Rounding, Rect.y+Rect.h-Rounding,
                                                 Rect.x+(1-Ca1)*Rounding, Rect.y+Rect.h-Rounding+Sa1*Rounding,
                                                 Rect.x+(1-Ca3)*Rounding, Rect.y+Rect.h-Rounding+Sa3*Rounding,
                                                 Rect.x+(1-Ca2)*Rounding, Rect.y+Rect.h-Rounding+Sa2*Rounding);

                    if(Corners&8) // BR
                        ArrayF[NumItems++] = IGraphics::CFreeformItem(
                                                 Rect.x+Rect.w-Rounding, Rect.y+Rect.h-Rounding,
                                                 Rect.x+Rect.w-Rounding+Ca1*Rounding, Rect.y+Rect.h-Rounding+Sa1*Rounding,
                                                 Rect.x+Rect.w-Rounding+Ca3*Rounding, Rect.y+Rect.h-Rounding+Sa3*Rounding,
                                                 Rect.x+Rect.w-Rounding+Ca2*Rounding, Rect.y+Rect.h-Rounding+Sa2*Rounding);
                }
                pSelf->m_pClient->Graphics()->QuadsDrawFreeform(ArrayF, NumItems);
            }
            static IGraphics::CQuadItem ArrayQ[9];
            int NumItems = 0;
            ArrayQ[NumItems++] = IGraphics::CQuadItem(Rect.x+Rounding, Rect.y+Rounding, Rect.w-Rounding*2, Rect.h-Rounding*2); // center
            ArrayQ[NumItems++] = IGraphics::CQuadItem(Rect.x+Rounding, Rect.y, Rect.w-Rounding*2, Rounding); // top
            ArrayQ[NumItems++] = IGraphics::CQuadItem(Rect.x+Rounding, Rect.y+Rect.h-Rounding, Rect.w-Rounding*2, Rounding); // bottom
            ArrayQ[NumItems++] = IGraphics::CQuadItem(Rect.x, Rect.y+Rounding, Rounding, Rect.h-Rounding*2); // left
            ArrayQ[NumItems++] = IGraphics::CQuadItem(Rect.x+Rect.w-Rounding, Rect.y+Rounding, Rounding, Rect.h-Rounding*2); // right

            if(!(Corners&1)) ArrayQ[NumItems++] = IGraphics::CQuadItem(Rect.x, Rect.y, Rounding, Rounding); // TL
            if(!(Corners&2)) ArrayQ[NumItems++] = IGraphics::CQuadItem(Rect.x+Rect.w, Rect.y, -Rounding, Rounding); // TR
            if(!(Corners&4)) ArrayQ[NumItems++] = IGraphics::CQuadItem(Rect.x, Rect.y+Rect.h, Rounding, -Rounding); // BL
            if(!(Corners&8)) ArrayQ[NumItems++] = IGraphics::CQuadItem(Rect.x+Rect.w, Rect.y+Rect.h, -Rounding, -Rounding); // BR

            pSelf->m_pClient->Graphics()->QuadsDrawTL(ArrayQ, NumItems);
        }
        lua_pop(L, 1);
        iX++;
    }
    char aBuf[128];
    str_format(aBuf, sizeof(aBuf), "%f", 1.0f / ((float)(time_get() - Test) / (float)time_freq()));
    pSelf->m_pClient->Console()->Print(0, "FPS", aBuf);
    lua_pop(L, 1);
    pSelf->m_pClient->Graphics()->QuadsEnd();
    return 0;
}

int CLuaFile::UiDirectLine(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
        return 0;

    vec4 Color = vec4(0, 0, 0, 0.5f);

    float x1 = lua_tonumber(L, 1);
    float y1 = lua_tonumber(L, 2);
    float x2 = lua_tonumber(L, 3);
    float y2 = lua_tonumber(L, 4);

    if (lua_isnumber(L, 5) && lua_isnumber(L, 6) && lua_isnumber(L, 7) && lua_isnumber(L, 8))
        Color = vec4(lua_tonumber(L, 5), lua_tonumber(L, 6), lua_tonumber(L, 7), lua_tonumber(L, 8));

    pSelf->m_pClient->Graphics()->TextureSet(-1);
    pSelf->m_pClient->Graphics()->BlendNormal();
    pSelf->m_pClient->Graphics()->LinesBegin();
    pSelf->m_pClient->Graphics()->SetColor(Color.r, Color.g, Color.b, Color.a);
    IGraphics::CLineItem Line;
    Line = IGraphics::CLineItem(x1, y1, x2, y2);
    pSelf->m_pClient->Graphics()->LinesDraw(&Line, 1);
    pSelf->m_pClient->Graphics()->LinesEnd();

    return 0;
}

int CLuaFile::UiDirectLabel(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isstring(L, 3))
        return 0;

    vec4 Color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    vec4 ColorOutline = vec4(0.0f, 0.0f, 0.0f, 0.5f);

    float x = lua_tonumber(L, 1);
    float y = lua_tonumber(L, 2);
    const char *pText = lua_tostring(L, 3);
    int Align = -1;
    float Size = 14.0f;

    if (lua_isnumber(L, 4))
        Align = lua_tonumber(L, 4);
    if (lua_isnumber(L, 5))
        Size = lua_tonumber(L, 5);
    if (lua_isnumber(L, 6) && lua_isnumber(L, 7) && lua_isnumber(L, 8) && lua_isnumber(L, 9))
        Color = vec4(lua_tonumber(L, 6), lua_tonumber(L, 7), lua_tonumber(L, 8), lua_tonumber(L, 9));
    if (lua_isnumber(L, 10) && lua_isnumber(L, 11) && lua_isnumber(L, 12) && lua_isnumber(L, 13))
        ColorOutline = vec4(lua_tonumber(L, 10), lua_tonumber(L, 11), lua_tonumber(L, 12), lua_tonumber(L, 13));

    pSelf->m_pClient->TextRender()->TextOutlineColor(ColorOutline.r, ColorOutline.g, ColorOutline.b, ColorOutline.a);
    pSelf->m_pClient->TextRender()->TextColor(Color.r, Color.g, Color.b, Color.a);
    float tw = pSelf->m_pClient->TextRender()->TextWidth(0, Size, pText, -1);
    if (Align == -1) //left
        pSelf->m_pClient->TextRender()->Text(0, x-tw, y, Size, pText, -1);
    if (Align == 0) //center
        pSelf->m_pClient->TextRender()->Text(0, x-tw/2.0f, y, Size, pText, -1);
    if (Align == 1) //right
        pSelf->m_pClient->TextRender()->Text(0, x, y, Size, pText, -1);
    pSelf->m_pClient->TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
    pSelf->m_pClient->TextRender()->TextOutlineColor(0.0f, 0.0f, 0.0f, 0.3f);
    return 0;
}

int CLuaFile::BlendNormal(lua_State *L)
{
    LUA_FUNCTION_HEADER

    pSelf->m_pClient->Graphics()->BlendNormal();

    return 0;
}

int CLuaFile::BlendAdditive(lua_State *L)
{
    LUA_FUNCTION_HEADER

    pSelf->m_pClient->Graphics()->BlendAdditive();

    return 0;
}

int CLuaFile::UiGetGameTextureID(lua_State *L)
{
    LUA_FUNCTION_HEADER

    lua_pushinteger(L, g_pData->m_aImages[IMAGE_GAME].m_Id);
    return 1;
}

int CLuaFile::UiGetParticleTextureID(lua_State *L)
{
    LUA_FUNCTION_HEADER

    lua_pushinteger(L, g_pData->m_aImages[IMAGE_PARTICLES].m_Id);
    return 1;
}

int CLuaFile::UiGetFlagTextureID(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isnumber(L, 1))
        return 0;

    lua_pushinteger(L, pSelf->m_pClient->m_pCountryFlags->GetByCountryCode(lua_tonumber(L, 1))->m_Texture);
    return 1;
}

int CLuaFile::TextureLoad(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isstring(L, 1))
        return 0;

    int ID = pSelf->m_pClient->Graphics()->LoadTexture(lua_tostring(L, 1), IStorage::TYPE_ALL, CImageInfo::FORMAT_AUTO, IGraphics::TEXLOAD_NORESAMPLE);
    pSelf->m_lTextures.add(ID);

    lua_pushinteger(L, ID);
    return 1;
}

int CLuaFile::TextureUnload(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isnumber(L, 1))
        return 0;

    pSelf->m_pClient->Graphics()->UnloadTexture(lua_tointeger(L, 1));
    return 0;
}

int CLuaFile::ReplaceGameTexture(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (lua_isnumber(L, 1))
        g_pData->m_aImages[IMAGE_GAME].m_Id = lua_tointeger(L, 1);
    return 0;
}

int CLuaFile::RenderTexture(lua_State *L)
{
    LUA_FUNCTION_HEADER

    //1 texture
    //2 x
    //3 y
    //4 (width)
    //5 (height)
    //6 (clip_x1)
    //7 (clip_y1)
    //8 (clip_x2)
    //9 (clip_y2)
    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
        return 0;

    float x = lua_tonumber(L, 2);
    float y = lua_tonumber(L, 3);
    int ImgWidth = pSelf->m_pClient->Graphics()->GetTextureWidth(lua_tointeger(L, 1));
    int ImgHeight = pSelf->m_pClient->Graphics()->GetTextureHeight(lua_tointeger(L, 1));
    float ClipX1 = 0.0f;
    float ClipY1 = 0.0f;
    float ClipX2 = 1.0f;
    float ClipY2 = 1.0f;
    if (lua_isnumber(L, 6))
        ClipX1 = clamp((float)lua_tonumber(L, 6) / (float)ImgWidth, 0.0f, 1.0f);
    if (lua_isnumber(L, 7))
        ClipY1 = clamp((float)lua_tonumber(L, 7) / (float)ImgHeight, 0.0f, 1.0f);
    if (lua_isnumber(L, 8))
        ClipX2 = clamp((float)lua_tonumber(L, 8) / (float)ImgWidth, 0.0f, 1.0f);
    if (lua_isnumber(L, 9))
        ClipY2 = clamp((float)lua_tonumber(L, 9) / (float)ImgHeight, 0.0f, 1.0f);


    float Width = abs(ClipX2 - ClipX1) * ImgWidth;
    float Height = abs(ClipY2 - ClipY1) * ImgHeight;
    if (lua_isnumber(L, 4))
        Width = lua_tonumber(L, 4);
    if (lua_isnumber(L, 5))
        Height = lua_tonumber(L, 5);

    if (pSelf->m_pClient->Graphics()->OnScreen(x, y, Width, Height) == false)
        return 0;

    pSelf->m_pClient->Graphics()->TextureSet(lua_tointeger(L, 1));
    pSelf->m_pClient->Graphics()->QuadsBegin();
    if (lua_isnumber(L, 10) && lua_isnumber(L, 11) && lua_isnumber(L, 12) && lua_isnumber(L, 13))
        pSelf->m_pClient->Graphics()->SetColor(lua_tonumber(L, 10), lua_tonumber(L, 11), lua_tonumber(L, 12), lua_tonumber(L, 13));
    if (lua_isnumber(L, 14))
        pSelf->m_pClient->Graphics()->QuadsSetRotation(lua_tonumber(L, 14));
    if (lua_isnumber(L, 15) && lua_isnumber(L, 16))
        pSelf->m_pClient->Graphics()->QuadsSetRotationCenter(lua_tonumber(L, 15), lua_tonumber(L, 16));
    pSelf->m_pClient->Graphics()->QuadsSetSubset(ClipX1, ClipY1, ClipX2, ClipY2);
    IGraphics::CQuadItem QuadItem(x, y, Width, Height);
    pSelf->m_pClient->Graphics()->QuadsDrawTL(&QuadItem, 1);
    pSelf->m_pClient->Graphics()->QuadsEnd();
    return 0;
}

int CLuaFile::RenderSprite(lua_State *L)
{
    LUA_FUNCTION_HEADER

    //1 texture
    //2 sprite
    //3 x
    //4 y
    //5 (size)
    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
        return 0;

    float x = lua_tonumber(L, 3);
    float y = lua_tonumber(L, 4);
    float Size = lua_isnumber(L, 5) ? lua_tonumber(L, 5) : 1.0f;
    if (pSelf->m_pClient->Graphics()->OnScreen(x, y, Size, Size) == false)
        return 0;

    pSelf->m_pClient->Graphics()->TextureSet(lua_tointeger(L, 1));
    pSelf->m_pClient->Graphics()->QuadsBegin();
    pSelf->m_pClient->RenderTools()->SelectSprite(lua_tointeger(L, 2));
    pSelf->m_pClient->RenderTools()->DrawSprite(x, y, Size);
    pSelf->m_pClient->Graphics()->QuadsEnd();
    return 0;
}

int CLuaFile::LoadWvFile(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if(!lua_isstring(L, 1))
        return 0;

    int ID = pSelf->m_pClient->Sound()->LoadWV(lua_tostring(L, 1));
    pSelf->m_lSounds.add(ID);
    lua_pushinteger(L, ID);
    return 1;
}

int CLuaFile::PlayWv(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if(!lua_isnumber(L, 1))
        return 0;

    int Channel = CSounds::CHN_WORLD;
    if (lua_isnumber(L, 2) && lua_tointeger(L, 2) == 1)
        Channel = CSounds::CHN_GLOBAL;

    float x = 0;
    float y = 0;

    if (lua_isnumber(L, 3) && lua_isnumber(L, 4))
    {
        x = lua_tonumber(L, 3);
        y = lua_tonumber(L, 4);
    }

    pSelf->m_pClient->Sound()->PlayAt(Channel, lua_tointeger(L, 1), 0, x, y);
    return 0;
}

int CLuaFile::PlaySound(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if(!lua_isnumber(L, 1))
        return 0;


    float x = 0;
    float y = 0;

    if (lua_isnumber(L, 2) && lua_isnumber(L, 3))
    {
        x = lua_tonumber(L, 2);
        y = lua_tonumber(L, 3);
    }

    pSelf->m_pClient->m_pSounds->Play(CSounds::CHN_WORLD, lua_tointeger(L, 1), 1.0f, vec2(x, y));
    return 0;
}

int CLuaFile::SendPacket(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if(lua_isnil(L, 1))
        return 0;

    size_t InSize = 0;
    const char *pData = lua_tolstring(L, 1, &InSize);
    char aData[8192];
    size_t Size = sizeof(aData);
    CMsgPacker P(NETMSG_LUA_DATA);
    if (compress2((Bytef *)aData, (uLongf *)&Size, (Bytef *)pData, InSize, Z_BEST_COMPRESSION) == Z_OK && Size < InSize)
    {
        P.AddInt(Size);
        P.AddRaw(aData, Size);
    }
    else
    {
        P.AddInt(-InSize); //no compression
        P.AddRaw(pData, InSize);
    }
    pSelf->m_pClient->Client()->SendMsgEx(&P, MSGFLAG_VITAL|MSGFLAG_FLUSH, true);

    return 0;
}

int CLuaFile::GetNumGroups(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if(!pSelf->m_pClient->Layers())
    {
        return 0;
    }

    lua_pushnumber(L, pSelf->m_pClient->Layers()->NumGroups());
    return 1;
}
int CLuaFile::GetNumLayers(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if(!pSelf->m_pClient->Layers())
    {
        return 0;
    }

    lua_pushnumber(L, pSelf->m_pClient->Layers()->NumLayers());
    return 1;
}
int CLuaFile::GetGroupNumLayers(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if(!lua_isnumber(L, 1) || !pSelf->m_pClient->Layers() || pSelf->m_pClient->Layers()->NumGroups() < 1)
    {
        return 0;
    }
    if(!pSelf->m_pClient->Layers()->GetGroup(lua_tointeger(L, 1)))
    {
        return 0;
    }
    lua_pushnumber(L, pSelf->m_pClient->Layers()->GetGroup(lua_tointeger(L, 1))->m_NumLayers);
    return 1;
}

int CLuaFile::GetLayerType(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if(!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !pSelf->m_pClient->Layers() || pSelf->m_pClient->Layers()->NumGroups() < 1)
    {
        return 0;
    }
    int Group = clamp((int)lua_tointeger(L, 1), 0, pSelf->m_pClient->Layers()->NumGroups()-1);
    int Index = -1;
    if(pSelf->m_pClient->Layers()->GetGroup(Group))
        Index = clamp((int)lua_tointeger(L, 2), 0, pSelf->m_pClient->Layers()->GetGroup(Group)->m_NumLayers-1);
    else
    {
        return 0;
    }
    if (Index == -1)
    {
        return 0;
    }
    lua_pushnumber(L, pSelf->m_pClient->Layers()->GetLayer(pSelf->m_pClient->Layers()->GetGroup(Group)->m_StartLayer+Index)->m_Type);
    return 1;
}
int CLuaFile::GetLayerFlags(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if(!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !pSelf->m_pClient->Layers() || pSelf->m_pClient->Layers()->NumGroups() < 1)
    {
        return 0;
    }
    int Group = clamp((int)lua_tointeger(L, 1), 0, pSelf->m_pClient->Layers()->NumGroups()-1);
    int Index = -1;
    if(pSelf->m_pClient->Layers()->GetGroup(Group))
        Index = clamp((int)lua_tointeger(L, 2), 0, pSelf->m_pClient->Layers()->GetGroup(Group)->m_NumLayers-1);
    else
    {
        return 0;
    }
    if (Index == -1)
    {
        return 0;
    }

    CMapItemLayerTilemap *pTmap = (CMapItemLayerTilemap *)pSelf->m_pClient->Layers()->GetLayer(pSelf->m_pClient->Layers()->GetGroup(Group)->m_StartLayer+Index);
    lua_pushnumber(L, pTmap->m_Flags);
    return 1;
}
int CLuaFile::GetLayerSize(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if(!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !pSelf->m_pClient->Layers() || pSelf->m_pClient->Layers()->NumGroups() < 1)
    {
        return 0;
    }
    int Group = clamp((int)lua_tointeger(L, 1), 0, pSelf->m_pClient->Layers()->NumGroups()-1);
    int Index = -1;
    if(pSelf->m_pClient->Layers()->GetGroup(Group))
        Index = clamp((int)lua_tointeger(L, 2), 0, pSelf->m_pClient->Layers()->GetGroup(Group)->m_NumLayers-1);
    else
    {
        return 0;
    }
    if (Index == -1)
    {
        return 0;
    }
    CMapItemLayerTilemap *pTmap = (CMapItemLayerTilemap *)pSelf->m_pClient->Layers()->GetLayer(pSelf->m_pClient->Layers()->GetGroup(Group)->m_StartLayer+Index);

    if(!pTmap)
    {
        return 0;
    }

    lua_pushnumber(L, pTmap->m_Width);
    lua_pushnumber(L, pTmap->m_Height);
    return 2;
}

int CLuaFile::GetLayerTileFlags(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if(!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4) || !pSelf->m_pClient->Layers() || pSelf->m_pClient->Layers()->NumGroups() < 1)
    {
        return 0;
    }
    int Group = clamp((int)lua_tointeger(L, 1), 0, pSelf->m_pClient->Layers()->NumGroups()-1);
    int Index = -1;
    if(pSelf->m_pClient->Layers()->GetGroup(Group))
        Index = clamp((int)lua_tointeger(L, 2), 0, pSelf->m_pClient->Layers()->GetGroup(Group)->m_NumLayers-1);
    else
    {
        return 0;
    }
    if (Index == -1)
    {
        return 0;
    }

    int x = lua_tointeger(L, 3);
    int y = lua_tointeger(L, 4);
    CMapItemLayer *pLayer = pSelf->m_pClient->Layers()->GetLayer(pSelf->m_pClient->Layers()->GetGroup(Group)->m_StartLayer+Index);
    if(!pLayer)
    {
        return 0;
    }
    if(pLayer->m_Type != LAYERTYPE_TILES)
    {
        return 0;
    }
    CMapItemLayerTilemap *pTmap = (CMapItemLayerTilemap *)pLayer;
    CTile *pTiles = (CTile *)pSelf->m_pClient->Layers()->Map()->GetData(pTmap->m_Data);


    lua_pushnumber(L, pTiles[y*pTmap->m_Width+x].m_Flags);
    return 1;
}

int CLuaFile::SetLayerTileFlags(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if(!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4) || !lua_isnumber(L, 5) || !pSelf->m_pClient->Layers() || pSelf->m_pClient->Layers()->NumGroups() < 1)
    {
        return 0;
    }
    int Group = clamp((int)lua_tointeger(L, 1), 0, pSelf->m_pClient->Layers()->NumGroups()-1);
    int Index = -1;
    if(pSelf->m_pClient->Layers()->GetGroup(Group))
        Index = clamp((int)lua_tointeger(L, 2), 0, pSelf->m_pClient->Layers()->GetGroup(Group)->m_NumLayers-1);
    else
    {
        return 0;
    }
    if (Index == -1)
    {
        return 0;
    }

    int x = lua_tointeger(L, 3);
    int y = lua_tointeger(L, 4);
    int NewFlags = lua_tointeger(L, 5);
    CMapItemLayer *pLayer = pSelf->m_pClient->Layers()->GetLayer(pSelf->m_pClient->Layers()->GetGroup(Group)->m_StartLayer+Index);
    if(!pLayer)
        return 0;
    if(pLayer->m_Type != LAYERTYPE_TILES)
        return 0;

    CMapItemLayerTilemap *pTmap = (CMapItemLayerTilemap *)pLayer;
    CTile *pTiles = (CTile *)pSelf->m_pClient->Layers()->Map()->GetData(pTmap->m_Data);


    pTiles[y*pTmap->m_Width+x].m_Flags = NewFlags;
    return 0;
}
int CLuaFile::GetLayerTileIndex(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if(!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4) || !pSelf->m_pClient->Layers() || pSelf->m_pClient->Layers()->NumGroups() < 1)
    {
        return 0;
    }
    int Group = clamp((int)lua_tointeger(L, 1), 0, pSelf->m_pClient->Layers()->NumGroups()-1);
    int Index = -1;
    if(pSelf->m_pClient->Layers()->GetGroup(Group))
        Index = clamp((int)lua_tointeger(L, 2), 0, pSelf->m_pClient->Layers()->GetGroup(Group)->m_NumLayers-1);
    else
    {
        return 0;
    }
    if (Index == -1)
    {
        return 0;
    }

    int x = lua_tointeger(L, 3);
    int y = lua_tointeger(L, 4);
    CMapItemLayer *pLayer = pSelf->m_pClient->Layers()->GetLayer(pSelf->m_pClient->Layers()->GetGroup(Group)->m_StartLayer+Index);
    if(!pLayer)
    {
        return 0;
    }
    if(pLayer->m_Type != LAYERTYPE_TILES)
    {
        return 0;
    }
    CMapItemLayerTilemap *pTmap = (CMapItemLayerTilemap *)pLayer;
    CTile *pTiles = (CTile *)pSelf->m_pClient->Layers()->Map()->GetData(pTmap->m_Data);


    lua_pushnumber(L, pTiles[y*pTmap->m_Width+x].m_Index);
    return 1;
}

int CLuaFile::SetLayerTileIndex(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if(!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4) || !lua_isnumber(L, 5) || !pSelf->m_pClient->Layers() || pSelf->m_pClient->Layers()->NumGroups() < 1)
    {
        return 0;
    }
    int Group = clamp((int)lua_tointeger(L, 1), 0, pSelf->m_pClient->Layers()->NumGroups()-1);
    int Index = -1;
    if(pSelf->m_pClient->Layers()->GetGroup(Group))
        Index = clamp((int)lua_tointeger(L, 2), 0, pSelf->m_pClient->Layers()->GetGroup(Group)->m_NumLayers-1);
    else
    {
        return 0;
    }
    if (Index == -1)
    {
        return 0;
    }

    int x = lua_tointeger(L, 3);
    int y = lua_tointeger(L, 4);
    int NewIndex = lua_tointeger(L, 5);
    CMapItemLayer *pLayer = pSelf->m_pClient->Layers()->GetLayer(pSelf->m_pClient->Layers()->GetGroup(Group)->m_StartLayer + Index);
    if(!pLayer)
        return 0;
    if(pLayer->m_Type != LAYERTYPE_TILES)
        return 0;

    CMapItemLayerTilemap *pTmap = (CMapItemLayerTilemap *)pLayer;
    CTile *pTiles = (CTile *)pSelf->m_pClient->Layers()->Map()->GetData(pTmap->m_Data);

    x = clamp(x, 0, pTmap->m_Width - 1);
    y = clamp(y, 0, pTmap->m_Height - 1);

    pTiles[y*pTmap->m_Width+x].m_Index = NewIndex;
    for (int sx = 1; x-sx >= 0 && sx < 255; sx++)
    {
        if (pTiles[y*pTmap->m_Width+x - sx].m_Skip)
        {
            pTiles[y*pTmap->m_Width+x - sx].m_Skip = sx - 1;
            break;
        }
        if (pTiles[y*pTmap->m_Width+x - sx].m_Index)
            break;
    }
    return 0;
}

int CLuaFile::RenderTilemapGenerateSkip(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if(pSelf->m_pClient->Layers() && pSelf->m_pClient->RenderTools())
        thread_create(pSelf->m_pClient->RenderTilemapGenerateSkipThread, pSelf->m_pClient);
    return 0;
}

int CLuaFile::SetLocalCharacterPos(lua_State *L)
{
    LUA_FUNCTION_HEADER

    pSelf->m_pClient->m_LocalCharacterPos = vec2(lua_tonumber(L, 1), lua_tonumber(L, 2));
    return 1;
}

int CLuaFile::DemoStart(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isstring(L, 1))
        return 0;

    pSelf->m_pClient->Client()->DemoRecorder_Stop();
    pSelf->m_pClient->Client()->DemoRecorder_Start(lua_tostring(L, 1), false);
    return 0;
}

int CLuaFile::DemoStop(lua_State *L)
{
    LUA_FUNCTION_HEADER

    pSelf->m_pClient->Client()->DemoRecorder_Stop();
    return 0;
}

int CLuaFile::DemoDelete(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isstring(L, 1))
        return 0;

    char aBuf[1024];
    str_format(aBuf, sizeof(aBuf), "demos/%s", lua_tostring(L, 1));

    pSelf->m_pClient->Storage()->RemoveFile(aBuf, IStorage::TYPE_SAVE);
    return 0;
}

int CLuaFile::StatGetNumber(lua_State *L)
{
    LUA_FUNCTION_HEADER

    IOHANDLE IndexFile = pSelf->m_pClient->Storage()->OpenFile("stats/index.stat", IOFLAG_READ, IStorage::TYPE_ALL);
    if (IndexFile)
    {
        CStats::CStatsIndexRow Row;
        int Num = 0;
        while(io_read(IndexFile, &Row, sizeof(Row)))
        {
            Num++;
        }
        lua_pushinteger(L, Num);
        io_close(IndexFile);
    }
    else
        lua_pushinteger(L, 0);

    return 1;
}

int CLuaFile::StatGetInfo(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isnumber(L, 1))
        return 0;

    IOHANDLE IndexFile = pSelf->m_pClient->Storage()->OpenFile("stats/index.stat", IOFLAG_READ, IStorage::TYPE_ALL);
    if (IndexFile)
    {
        CStats::CStatsIndexRow Row;
        io_seek(IndexFile, lua_tointeger(L, 1) * sizeof(Row), IOSEEK_START);
        io_read(IndexFile, &Row, sizeof(Row));
        char aBuf[256];
        net_addr_str(&Row.m_ServerAddr, aBuf, sizeof(aBuf), 1);
        lua_pushstring(L, aBuf);
        lua_pushstring(L, Row.m_aMap);
        lua_pushstring(L, Row.m_aGameType);
        lua_pushstring(L, Row.m_aServerName);
        lua_pushinteger(L, Row.m_TimeStamp);
        lua_pushinteger(L, Row.m_Uid);

        io_close(IndexFile);
        return 6;
    }

    return 0;
}

int CLuaFile::StatGetRow(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isnumber(L, 1) && !lua_isnumber(L, 2))
        return 0;
    char aBuf[256];
    str_format(aBuf, sizeof(aBuf), "stats/%i.stat", lua_tointeger(L, 1));
    IOHANDLE StatFile = pSelf->m_pClient->Storage()->OpenFile(aBuf, IOFLAG_READ, IStorage::TYPE_ALL);
    int Ret = 0;
    if (StatFile)
    {
        CStatsRecords::CRecordRow Row;
        io_seek(StatFile, lua_tointeger(L, 2) * sizeof(Row), IOSEEK_START);
        io_read(StatFile, &Row, sizeof(Row));
        lua_pushinteger(L, Row.m_aData[0]);
        if (Row.m_aData[0] == CStats::STATROW_SERVER)
        {
            lua_pushinteger(L, Row.m_aData[1]);
            Ret = 2;
        }
        if (Row.m_aData[0] == CStats::STATROW_KILL)
        {
            CNetMsg_Sv_KillMsg *pMsg = (CNetMsg_Sv_KillMsg *)&Row.m_aData[1];
            lua_pushinteger(L, pMsg->m_Killer);
            lua_pushinteger(L, pMsg->m_ModeSpecial);
            lua_pushinteger(L, pMsg->m_Victim);
            lua_pushinteger(L, pMsg->m_Weapon);
            Ret = 5;
        }

        io_close(StatFile);
    }

    return Ret;
}

int CLuaFile::TimeGet(lua_State *L)
{
    LUA_FUNCTION_HEADER

    lua_pushnumber(L, ((float)time_get()) / (float)time_freq());
    return 1;
}

int CLuaFile::FPS(lua_State *L)
{
    LUA_FUNCTION_HEADER

    lua_pushnumber(L, 1.0f / pSelf->m_pClient->Client()->FrameTime());
    return 1;
}

int CLuaFile::CheckVersion(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (lua_isstring(L, 1))
        lua_pushboolean(L, str_comp(GAME_LUA_VERSION, lua_tostring(L, 1)) == 0);
    else
        lua_pushboolean(L, false);
    return 1;
}

int CLuaFile::GetVersion(lua_State *L)
{
    LUA_FUNCTION_HEADER

    lua_pushstring(L, GAME_LUA_VERSION);
    return 1;
}

int CLuaFile::GetWaveFrameSize(lua_State *L)
{
    LUA_FUNCTION_HEADER

    lua_pushinteger(L, pSelf->m_pClient->Sound()->GetWaveFrameSize());
    return 1;
}

int CLuaFile::GetWaveBufferSpace(lua_State *L)
{
    LUA_FUNCTION_HEADER

    lua_pushinteger(L, pSelf->m_pClient->Sound()->GetWaveBufferSpace());
    return 1;
}

int CLuaFile::AddWaveToStream(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isstring(L, 1))
        return 0;

    size_t Size = 0;
    const char *pData = lua_tolstring(L, 1, &Size);
    if ((int)Size != pSelf->m_pClient->Sound()->GetWaveFrameSize())
        dbg_msg("msg", "got: %i - wanted: %i", Size, pSelf->m_pClient->Sound()->GetWaveFrameSize()); //quote: westerwave
    lua_pushinteger(L, pSelf->m_pClient->Sound()->AddWaveToStream(pData));
    return 1;
}

static short Int2Short(int i)
{
    if(i > 0x7fff)
        return 0x7fff;
    else if(i < -0x7fff)
        return -0x7fff;
    return i;
}

int CLuaFile::FloatToShortChars(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isnumber(L, 1))
        return 0;

    float Value = clamp((float)lua_tonumber(L, 1), -1.0f, 1.0f);
    short Ret = Int2Short(Value * 32767);
    swap_endian(&Ret, 2, 1);
    lua_pushlstring(L, (char *)&Ret, 2);
    return 1;
}

int CLuaFile::LoadSkin(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (lua_isstring(L, 1))
    {
        CSkins::CSkin Skin;
        const char *pName = str_find_rev(lua_tostring(L, 1), "\\");
        if (pName == 0)
            pName = str_find_rev(lua_tostring(L, 1), "/");
        if (pName == 0)
            pName = lua_tostring(L, 1);
        int l = str_length(pName);
        str_copy(Skin.m_aName, pName , min((int)sizeof(Skin.m_aName), l-3));
        str_copy(Skin.m_aFilename, lua_tostring(L, 1), (int)sizeof(Skin.m_aFilename));
        Skin.m_Loaded = false;
        pSelf->m_pClient->m_pSkins->LoadSkin(&Skin, false);
    }
    return 0;
}

int CLuaFile::ConsoleActive(lua_State *L)
{
    LUA_FUNCTION_HEADER
    lua_pushboolean(L, (pSelf->m_pClient->m_pGameConsole->GetConsoleState() == CGameConsole::CONSOLE_OPEN || pSelf->m_pClient->m_pGameConsole->GetConsoleState() == CGameConsole::CONSOLE_OPENING));
    return 1;
}

int CLuaFile::ConsoleLocalActive(lua_State *L)
{
    LUA_FUNCTION_HEADER
    lua_pushboolean(L, (pSelf->m_pClient->m_pGameConsole->GetConsoleState() == CGameConsole::CONSOLE_OPEN || pSelf->m_pClient->m_pGameConsole->GetConsoleState() == CGameConsole::CONSOLE_OPENING) && pSelf->m_pClient->m_pGameConsole->GetConsoleType() == CGameConsole::CONSOLETYPE_LOCAL);
    return 1;
}

int CLuaFile::ConsoleRemoteActive(lua_State *L)
{
    LUA_FUNCTION_HEADER
    lua_pushboolean(L, (pSelf->m_pClient->m_pGameConsole->GetConsoleState() == CGameConsole::CONSOLE_OPEN || pSelf->m_pClient->m_pGameConsole->GetConsoleState() == CGameConsole::CONSOLE_OPENING) && pSelf->m_pClient->m_pGameConsole->GetConsoleType() == CGameConsole::CONSOLETYPE_REMOTE);
    return 1;
}

int CLuaFile::GetLocalCharacterPos(lua_State *L)
{
    LUA_FUNCTION_HEADER
    // Can be used for direct rendering on screen

    lua_pushnumber(L, pSelf->m_pClient->m_LocalCharacterPos.x);
    lua_pushnumber(L, pSelf->m_pClient->m_LocalCharacterPos.y);

    return 2;
}

int CLuaFile::GetLocalCharacterWeapon(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (pSelf->m_pClient->m_Snap.m_pLocalCharacter)
    {
        lua_pushnumber(L, pSelf->m_pClient->m_Snap.m_pLocalCharacter->m_Weapon);
        return 1;
    }
    return 0;
}

int CLuaFile::GetLocalCharacterWeaponAmmo(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (pSelf->m_pClient->m_Snap.m_pLocalCharacter)
    {
        lua_pushnumber(L, pSelf->m_pClient->m_Snap.m_pLocalCharacter->m_AmmoCount);
        return 1;
    }
    return 0;
}

int CLuaFile::GetLocalCharacterHealth(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (pSelf->m_pClient->m_Snap.m_pLocalCharacter)
    {
        lua_pushnumber(L, pSelf->m_pClient->m_Snap.m_pLocalCharacter->m_Health);
        return 1;
    }
    return 0;
}

int CLuaFile::GetLocalCharacterArmor(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (pSelf->m_pClient->m_Snap.m_pLocalCharacter)
    {
        lua_pushnumber(L, pSelf->m_pClient->m_Snap.m_pLocalCharacter->m_Armor);
        return 1;
    }
    return 0;
}

int CLuaFile::LocalExecute(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (lua_isstring(L, 1))
        pSelf->m_pClient->Console()->ExecuteLine(lua_tostring(L, 1));

    return 0;
}

int CLuaFile::LocalExecuteStroked(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (lua_isnumber(L, 1) && lua_isstring(L, 2))
        pSelf->m_pClient->Console()->ExecuteLineStroked(lua_tonumber(L, 1), lua_tostring(L, 2));

    return 0;
}

int CLuaFile::GetServerInfo(lua_State *L)
{
    LUA_FUNCTION_HEADER

    CServerInfo CurrentServerInfo;
    pSelf->m_pClient->Client()->GetServerInfo(&CurrentServerInfo);

    lua_pushstring(L, CurrentServerInfo.m_aName);
    lua_pushstring(L, g_Config.m_UiServerAddress);
    lua_pushstring(L, CurrentServerInfo.m_aGameType);
    lua_pushstring(L, CurrentServerInfo.m_aMap);
    lua_pushstring(L, CurrentServerInfo.m_aVersion);

    return 5;
}

int CLuaFile::CreateDamageIndicator(lua_State *L)
{
    LUA_FUNCTION_HEADER
    //--
    if(!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
        return 0;
    vec2 Pos = vec2(lua_tonumber(L, 1), lua_tonumber(L, 2));
    vec2 Dir = vec2(lua_tonumber(L, 3), lua_tonumber(L, 4));

    pSelf->m_pClient->m_pDamageind->Create(Pos, Dir);
    return 0;
}

int CLuaFile::AddChatLine(lua_State *L)
{
    LUA_FUNCTION_HEADER
    // For editing chats (Spam block - Filter)
    if(!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isstring(L, 3))
        return 0;

    pSelf->m_pClient->m_pChat->AddLine(lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tostring(L, 3));
    return 0;
}

int CLuaFile::CreateDirectory(lua_State *L)
{
    LUA_FUNCTION_HEADER
    if(!lua_isstring(L, 1))
        return 0;

    lua_pushboolean(L, fs_makedir(lua_tostring(L, 1)));
    return 1;
}

int CLuaFile::ListDirectoryInternal(const char *pName, int IsDir, int DirType, void *pUser)
{
    CListDirectoryData *pData = (CListDirectoryData *)pUser;

    lua_pushinteger(pData->m_L, pData->m_Number++);
    lua_newtable(pData->m_L);

    lua_pushstring(pData->m_L, "name");
    lua_pushstring(pData->m_L, pName);
    lua_settable(pData->m_L, -3);
    lua_pushstring(pData->m_L, "dir");
    lua_pushboolean(pData->m_L, IsDir);
    lua_settable(pData->m_L, -3);


    lua_settable(pData->m_L, -3);

    return 0;
}

int CLuaFile::ListDirectory(lua_State *L)
{
    LUA_FUNCTION_HEADER
    if(!lua_isstring(L, 1))
        return 0;

    lua_newtable(L);

    CListDirectoryData Data;
    Data.m_L = L;
    Data.m_Number = 0;
    fs_listdir(lua_tostring(L, 1), ListDirectoryInternal, 0, &Data);

    return 1;
}

int CLuaFile::GetDate (lua_State *L) //from loslib.c
{
    const char *s = luaL_optstring(L, 1, "%c");
    time_t t = luaL_opt(L, (time_t)luaL_checknumber, 2, time(NULL));
    struct tm tmr, *stm;
    if (*s == '!')    /* UTC? */
    {
        stm = l_gmtime(&t, &tmr);
        s++;  /* skip `!' */
    }
    else
        stm = l_localtime(&t, &tmr);
    if (stm == NULL)  /* invalid date? */
        lua_pushnil(L);
    else if (str_comp(s, "*t") == 0)
    {
        lua_createtable(L, 0, 9);  /* 9 = number of fields */
        setfield(L, "sec", stm->tm_sec);
        setfield(L, "min", stm->tm_min);
        setfield(L, "hour", stm->tm_hour);
        setfield(L, "day", stm->tm_mday);
        setfield(L, "month", stm->tm_mon+1);
        setfield(L, "year", stm->tm_year+1900);
        setfield(L, "wday", stm->tm_wday+1);
        setfield(L, "yday", stm->tm_yday+1);
        setboolfield(L, "isdst", stm->tm_isdst);
    }
    else
    {
        char cc[4];
        luaL_Buffer b;
        cc[0] = '%';
        luaL_buffinit(L, &b);
        while (*s)
        {
            if (*s != '%')  /* no conversion specifier? */
                luaL_addchar(&b, *s++);
            else
            {
                size_t reslen;
                char buff[200];  /* should be big enough for any conversion result */
                s = checkoption(L, s + 1, cc);
                reslen = strftime(buff, sizeof(buff), cc, stm);
                luaL_addlstring(&b, buff, reslen);
            }
        }
        luaL_pushresult(&b);
    }
    return 1;
}

int CLuaFile::HostLookup(lua_State *L)
{
	LUA_FUNCTION_HEADER

	if(!lua_isstring(L, 1))
		return 0;
	CHostLookup tmp;
	pSelf->m_Lookup = tmp;
	pSelf->m_pClient->Engine()->HostLookup(&pSelf->m_Lookup, lua_tostring(L, 1), NETTYPE_IPV4);
	return 0;
}

int CLuaFile::HostLookupGetResult(lua_State *L)
{
	LUA_FUNCTION_HEADER

	if(pSelf->m_Lookup.m_Job.Status() != CJob::STATE_DONE)
	{
		lua_pushnumber(L, 0);
		return 1;
	}
	else
	{
		if(pSelf->m_Lookup.m_Job.Result() == 0)
		{
			lua_pushnumber(L, 1);
			char aAddrStr[NETADDR_MAXSTRSIZE];
			net_addr_str(&pSelf->m_Lookup.m_Addr, aAddrStr, sizeof(aAddrStr), false);
			lua_pushstring(L, aAddrStr);
			return 2;
		}
		else
		{
			lua_pushnumber(L, -1);
			return 1;
		}
	}
	return 0;
}
