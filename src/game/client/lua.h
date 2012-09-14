/* (c) MAP94. See www.n-lvl.com/ndc/nclient/ for more information. */
#ifndef GAME_CLIENT_LUA_H
#define GAME_CLIENT_LUA_H

#include "gameclient.h"
#include <engine/shared/config.h>
#include <engine/config.h>
#include <engine/input.h>
#include <base/tl/array.h>
#include <base/tl/sorted_array.h>
#include <game/luaevent.h>
#include <engine/engine.h>
#include <engine/shared/network.h>

#define NON_HASED_VERSION
#include <game/version.h>
#undef NON_HASED_VERSION

extern "C" { // lua
#define LUA_CORE /* make sure that we don't try to import these functions */
#include <engine/external/lua/lua.h>
#include <engine/external/lua/lualib.h> /* luaL_openlibs */
#include <engine/external/lua/lauxlib.h> /* luaL_loadfile */
}

#include <game/luashared.h>

class CLuaBinding
{
public:
    CGameClient *m_pClient;
    CLuaBinding(CGameClient *pClient);
    ~CLuaBinding();
    //Control binding
    int m_ControlDirectionPre;
    int m_ControlDirection;
    bool m_ControlDirectionIsSet;
    int m_ControlJumpPre;
    int m_ControlJump;
    bool m_ControlJumpIsSet;
    int m_ControlHookPre;
    int m_ControlHook;
    bool m_ControlHookIsSet;
    int m_ControlFirePre;
    int m_ControlFire;
    bool m_ControlFireIsSet;
    int m_ControlWeaponPre;
    int m_ControlWeapon;
    bool m_ControlWeaponIsSet;
    int m_ControlTargetXPre;
    int m_ControlTargetX;
    bool m_ControlTargetXIsSet;
    int m_ControlTargetYPre;
    int m_ControlTargetY;
    bool m_ControlTargetYIsSet;


    int m_ControlDirectionPredicted;
    bool m_ControlDirectionPredictedIsSet;
    int m_ControlJumpPredicted;
    bool m_ControlJumpPredictedIsSet;
    int m_ControlHookPredicted;
    bool m_ControlHookPredictedIsSet;
    int m_ControlFirePredicted;
    bool m_ControlFirePredictedIsSet;
    int m_ControlWeaponPredicted;
    bool m_ControlWeaponPredictedIsSet;
    int m_ControlTargetXPredicted;
    bool m_ControlTargetXPredictedIsSet;
    int m_ControlTargetYPredicted;
    bool m_ControlTargetYPredictedIsSet;
};

class CLuaUi
{
public:
    CLuaUi();
    ~CLuaUi();
    //button
    int m_Checked;
    CUIRect m_Rect;
    //edit box
    float m_FontSize;
    bool m_Hidden;

    //Label
    int m_Align;

    //slider
    float m_Value;
    int m_Direction;

    //images
    float m_ClipX1;
    float m_ClipY1;
    float m_ClipX2;
    float m_ClipY2;

    int m_Corners;
    int m_Rounding;
    vec4 m_Color;

    char m_pText[256];


    //some system things
    int m_Type;
    enum
    {
        LUAUIBUTTON = 1,
        LUAUIEDITBOX,
        LUAUILABEL,
        LUAUIRECT,
        LUAUIIMAGE,
        LUAUILINE,
        LUAUISLIDER,
        LUAUIIMAGEEX,
    };
    int m_Id;
    float m_Offset; //for edit boxes
    char m_pCallback[256];
    CGameClient *m_pClient;
    class CLuaFile *m_pLuaFile;
    int m_RegPoint;

    int m_TextureID;
    int m_SpriteID;

    bool m_Used;
    void Tick();

    vec4 ButtonColorMul(const void *pID);
    int DoButton_Menu(const void *pID, const char *pText, int Checked, const CUIRect *pRect, int Corners, vec4 Color);
    int DoEditBox(void *pID, const CUIRect *pRect, char *pStr, unsigned StrSize, float FontSize, float *Offset, bool Hidden, int Corners, vec4 Color);
    int DoImage(int *pID, int TextureID, int SpriteID, const CUIRect *pRect);
    int DoImageEx(int *pID, int TextureID, const CUIRect *pRect, float ClipX1, float ClipY1, float ClipX2, float ClipY2);
    float DoScrollbarV(const void *pID, const CUIRect *pRect, float Current, vec4 Color);
    float DoScrollbarH(const void *pID, const CUIRect *pRect, float Current, vec4 Color);

};

class CLuaFile
{
public:
    CLuaFile();
    ~CLuaFile();
    class CLua *m_pLuaHandler;
    CLuaShared<CLuaFile> *m_pLuaShared;
    CGameClient *m_pClient;
    void UiTick();
    void Tick();
    void End();
    void Close();
    void Init(const char *pFile);


    //Some Error and Lua stuff
    //Error
    static int ErrorFunc(lua_State *L);
    static int Panic(lua_State *L); //lua panic function
    lua_State *m_pLua;

    enum
    {
        LUAMAXUIELEMENTS = 1024, //642 kb
    };

    CLuaUi m_aUiElements[LUAMAXUIELEMENTS];

    char *GetScriptName()
    {
        return m_aFilename;
    };

    array<int> m_lTextures;
    array<int> m_lSounds;

    //Settings
    void ConfigClose(); //Helper function
    bool m_HaveSettings;
    char m_aTitle[64];
    char m_aFilename[256];
    char m_aInfo[256];

    bool FunctionExist(const char *pFunctionName);
    int FunctionExec(const char *pFunctionName = 0);
    void FunctionPrepare(const char *pFunctionName);
    void PushString(const char *pString);
    void PushData(const char *pData, int Size);
    void PushInteger(int value);
    void PushFloat(float value);
    void PushBoolean(bool value);
    void PushParameter(const char *pString);
    int m_FunctionVarNum;

    int m_Error;

    CHostLookup m_Lookup;

    //Functions:
    //Settings
    static inline int SetScriptUseSettingPage(lua_State *L);
    static inline int SetScriptTitle(lua_State *L);
    static inline int SetScriptInfo(lua_State *L);

    //Eventlistener stuff
    static inline int AddEventListener(lua_State *L);
    static inline int RemoveEventListener(lua_State *L);
    //All Eventlistener
    //OnChat
    //OnStateChange
    //OnControlChange
    //OnRenderLevelItem1
    //OnRenderLevelItem2
    //OnKill
    //OnRenderLevel1
    //OnRenderLevel2
    //OnRenderLevel3
    //OnRenderLevel4
    //OnRenderLevel5
    //OnRenderBackground
    //OnServerBrowserGameTypeRender
    //OnScoreboardRender
    //OnKeyEvent
    //OnNetData
    //OnConsoleRemote
    //OnConsole
    //OnMusicChange

    //Menu
    static inline int MenuActive(lua_State *L);
    static inline int MenuGameActive(lua_State *L);
    static inline int MenuPlayersActive(lua_State *L);
    static inline int MenuServerInfoActive(lua_State *L);
    static inline int MenuCallVoteActive(lua_State *L);
    static inline int MenuServersActive(lua_State *L);
    static inline int MenuMusicActive(lua_State *L);
    static inline int MenuDemosActive(lua_State *L);

    //Mouse and Keyboard
    static inline int GetMousePosMenu(lua_State *L);
    static inline int SetMouseModeRelative(lua_State *L);
    static inline int SetMouseModeAbsolute(lua_State *L);

    //Scoreboard
    static inline int ScoreboardAbortRender(lua_State *L);

    //
    //Include
    static inline int Include(lua_State *L);

    //Sendinfo
    static inline int SendPlayerInfo(lua_State *L);

    //emote
    static inline int Emote(lua_State *L);

    //Character
    static inline int GetLocalCharacterId(lua_State *L);
    static inline int GetLocalCharacterPos(lua_State *L);
    static inline int GetLocalCharacterWeapon(lua_State *L);
    static inline int GetLocalCharacterWeaponAmmo(lua_State *L);
    static inline int GetLocalCharacterHealth(lua_State *L);
    static inline int GetLocalCharacterArmor(lua_State *L);

    static inline int GetCharacterPos(lua_State *L);
    static inline int SetCharacterPos(lua_State *L);
    static inline int GetCharacterVel(lua_State *L);
    static inline int SetCharacterVel(lua_State *L);
    static inline int GetCharacterActiveWeapon(lua_State *L);


    static inline int CharacterHasFlag(lua_State *L);
    static inline int GetCharacterHookPos(lua_State *L);
    static inline int GetCharacterHookDir(lua_State *L);
    static inline int GetCharacterHookState(lua_State *L);
    static inline int GetCharacterHookedPlayer(lua_State *L);
    static inline int GetCharacterHookTick(lua_State *L);


    //collision
    static inline int IntersectLine(lua_State *L);
    static inline int MovePoint(lua_State *L);
    static inline int MoveBox(lua_State *L);
    static inline int GetTile(lua_State *L);
    static inline int GetMapWidth(lua_State *L);
    static inline int GetMapHeight(lua_State *L);
    static inline int SetTile(lua_State *L);
    static inline int ClosestPointOnLine(lua_State *L);

    //layers
    static inline int GetNumGroups(lua_State *L);
    static inline int GetNumLayers(lua_State *L);
    static inline int GetGroupNumLayers(lua_State *L);
    static inline int GetLayerType(lua_State *L);
    static inline int GetLayerFlags(lua_State *L);
    static inline int GetLayerTileFlags(lua_State *L);
    static inline int GetLayerTileIndex(lua_State *L);
    static inline int SetLayerTileFlags(lua_State *L);
    static inline int SetLayerTileIndex(lua_State *L);
    static inline int GetLayerSize(lua_State *L);
    static inline int RenderTilemapGenerateSkip(lua_State *L);

    static inline int CreateParticle(lua_State *L);
    static inline int CreateDamageIndicator(lua_State *L);

    //Flow
    static inline int GetFlow(lua_State *L);
    static inline int SetFlow(lua_State *L);

    //Console Print
    static inline int Print(lua_State *L);
    static inline int Console(lua_State *L);
    static inline int ConsoleActive(lua_State *L);
    static inline int ConsoleLocalActive(lua_State *L);
    static inline int ConsoleRemoteActive(lua_State *L);
    static inline int LocalExecute(lua_State *L);
    static inline int LocalExecuteStroked(lua_State *L);

    //Remote console
    static inline int RconAuth(lua_State *L);
    static inline int RconAuthed(lua_State *L);
    static inline int RconExecute(lua_State *L);

    //States
    static inline int StateGet(lua_State *L);
    static inline int StateOnline(lua_State *L);
    static inline int StateOffline(lua_State *L);
    static inline int StateConnecting(lua_State *L);
    static inline int StateLoading(lua_State *L);
    static inline int StateDemoplayback(lua_State *L);

    //Serverinfo
    static inline int GetGameType(lua_State *L);
    static inline int GetServerInfo(lua_State *L);
    static inline int IsTeamplay(lua_State *L);

    //Get Net Error
    static inline int GetNetError(lua_State *L);

    //Connect
    static inline int Connect(lua_State *L);

    //Chat
    static inline int ChatSend(lua_State *L);
    static inline int ChatTeamSend(lua_State *L);
    static inline int AddChatLine(lua_State *L);

    //Player
    static inline int GetPlayerName(lua_State *L);
    static inline int GetPlayerClan(lua_State *L);
    static inline int GetPlayerCountry(lua_State *L);
    static inline int GetPlayerScore(lua_State *L);
    static inline int GetPlayerPing(lua_State *L);
    static inline int GetPlayerTeam(lua_State *L);
    static inline int GetPlayerSkin(lua_State *L);
    static inline int GetPlayerColorFeet(lua_State *L);
    static inline int GetPlayerColorBody(lua_State *L);
    static inline int GetPlayerColorSkin(lua_State *L);

    //Ui
    static inline int UiDoButton(lua_State *L);
    static inline int UiDoEditBox(lua_State *L);
    static inline int UiDoLabel(lua_State *L);
    static inline int UiDoRect(lua_State *L);
    static inline int UiDoImage(lua_State *L);
    static inline int UiDoImageEx(lua_State *L);
    static inline int UiDoLine(lua_State *L);
    static inline int UiDoSlider(lua_State *L);
    static inline int UiGetText(lua_State *L);
    static inline int UiSetText(lua_State *L);
    static inline int UiGetColor(lua_State *L);
    static inline int UiSetColor(lua_State *L);
    static inline int UiGetRect(lua_State *L);
    static inline int UiSetRect(lua_State *L);
    static inline int UiRemoveElement(lua_State *L);
    static inline int UiGetScreenWidth(lua_State *L);
    static inline int UiGetScreenHeight(lua_State *L);
    static inline int UiGetGameTextureID(lua_State *L);
    static inline int UiGetParticleTextureID(lua_State *L);
    static inline int UiGetFlagTextureID(lua_State *L);

    static inline int UiDirectEditBox(lua_State *L);
    static inline int UiDirectSlider(lua_State *L);
    static inline int UiDirectButton(lua_State *L);
    static inline int UiDirectRect(lua_State *L);
    static inline int UiDirectLine(lua_State *L);
    static inline int UiDirectLabel(lua_State *L);

    //fast ui functions ;)
    static inline int UiDirectRectArray(lua_State *L);

    static inline int BlendAdditive(lua_State *L);
    static inline int BlendNormal(lua_State *L);

    //Texture
    static inline int TextureLoad(lua_State *L);
    static inline int TextureUnload(lua_State *L);
    static inline int RenderTexture(lua_State *L);
    static inline int RenderSprite(lua_State *L);


    static inline int GetScreenWidth(lua_State *L);
    static inline int GetScreenHeight(lua_State *L);

    //Music
    static inline int MusicPlay(lua_State *L);
    static inline int MusicPause(lua_State *L);
    static inline int MusicStop(lua_State *L);
    static inline int MusicNext(lua_State *L);
    static inline int MusicPrev(lua_State *L);
    static inline int MusicSetVol(lua_State *L);
    static inline int MusicGetVol(lua_State *L);
    static inline int MusicGetState(lua_State *L);
    static inline int MusicGetPlayedIndex(lua_State *L);

    //Config
    static inline int GetConfigValue(lua_State *L);
    static inline int SetConfigValue(lua_State *L);

    //Control
    static inline int GetControlValue(lua_State *L);
    static inline int SetControlValue(lua_State *L);
    static inline int SetControlValuePredicted(lua_State *L);
    static inline int UnSetControlValue(lua_State *L);

    //Sound
    static inline int LoadWvFile(lua_State *L);
    static inline int PlayWv(lua_State *L);
    static inline int PlaySound(lua_State *L);

    //LuaNetWork
    static inline int SendPacket(lua_State *L);

    //Replace Texture
    static inline int ReplaceGameTexture(lua_State *L);

    static inline int GetKeyUnicode(lua_State *L);

    //demo
    static inline int DemoStart(lua_State *L);
    static inline int DemoStop(lua_State *L);
    static inline int DemoDelete(lua_State *L);

    //stats
    static inline int StatGetNumber(lua_State *L);
    static inline int StatGetInfo(lua_State *L);
    static inline int StatGetRow(lua_State *L);


    static inline int SetLocalCharacterPos(lua_State *L);

    static inline int TimeGet(lua_State *L);
    static inline int GetDate(lua_State *L);

    static inline int FPS(lua_State *L);

    //Version
    static inline int CheckVersion(lua_State *L);
    static inline int GetVersion(lua_State *L);

    //sound hook
    static inline int GetWaveFrameSize(lua_State *L);
    static inline int AddWaveToStream(lua_State *L);
    static inline int FloatToShortChars(lua_State *L);
    static inline int GetWaveBufferSpace(lua_State *L);

    //load skin
    static inline int LoadSkin(lua_State *L);

    //filesystem
    static inline int CreateDirectory(lua_State *L);
    static inline int ListDirectory(lua_State *L);
    struct CListDirectoryData
    {
        lua_State *m_L;
        int m_Number;
    };
    static inline int ListDirectoryInternal(const char *pName, int IsDir, int DirType, void *pUser);

    static inline int HostLookup(lua_State *L);
    static inline int HostLookupGetResult(lua_State *L);
};

class CLua
{
public:
    CGameClient *m_pClient;
    CLua(CGameClient *pClient);
    ~CLua();
    void Tick();
    bool Init(const char *pFile);
    void End();
    void Close();

    CLuaFile m_aLuaFiles[MAX_LUA_FILES];
    class CLuaEventListener<CLuaFile> *m_pEventListener;

    //Mouse
    bool m_MouseModeAbsolute;

    //Search the file and execs the function
    void ConfigClose(char *pFilename);

    int GetFileId(char *pFilename);

    int m_OriginalGameTexture;

    static void ConsolePrintCallback(const char *pLine, void *pUserData);
};


//helper functions
int StrIsInteger(const char *pStr);
int StrIsFloat(const char *pStr);

/*static char *ToLower(const char *str)
{
    static char saTmp[8192];
    str_copy(saTmp, str, sizeof(saTmp));
    return str_tolower(saTmp);
}*/

#endif
