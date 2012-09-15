/* (c) MAP94 and Patafix. See www.n-lvl.com/ndc/nclient/ for more information. */
#ifndef GAME_SERVER_LUA_H
#define GAME_SERVER_LUA_H

#include <game/server/gamecontext.h>
#include <game/server/gameworld.h>
#include <game/server/entities/projectile.h>
#include <game/server/entities/laser.h>
#include <engine/shared/config.h>
#include <engine/shared/array.h> //faster, thread safe array!
#include <engine/external/zlib/zlib.h>
#include <engine/config.h>
#include <base/tl/array.h>
#include <base/tl/sorted_array.h>
#include <game/luaevent.h>

#if defined(CONF_FAMILY_WINDOWS) //mysql -.-
	#include <windows.h>
#endif

extern "C" { // lua
    #define LUA_CORE /* make sure that we don't try to import these functions */
    #include <engine/external/lua/lua.h>
    #include <engine/external/lua/lualib.h> /* luaL_openlibs */
    #include <engine/external/lua/lauxlib.h> /* luaL_loadfile */

    #include <mysql.h>
}

#include <game/luashared.h>

class CLuaFile
{
public:
    CLuaFile();
    ~CLuaFile();
    class CLua *m_pLuaHandler;
    CLuaShared<CLuaFile> *m_pLuaShared;
    CGameContext *m_pServer;
    //void UiTick();
    void Tick();
    void TickDefered();
    void PostTick();
    void End();
    void Close();
    void Init(const char *pFile);

    //Some Error and Lua stuff
    //Error
    static int ErrorFunc(lua_State *L);
    static int Panic(lua_State *L); //lua panic function
    lua_State *m_pLua;

    char *GetScriptName() {return m_aFilename;};

    //Settings
    void ConfigClose(); //Helper function
    bool m_HaveSettings;
    char m_aTitle[64];
    char m_aFilename[256];
    char m_aInfo[256];

    bool FunctionExist(const char *pFunctionName);
    void FunctionExec(const char *pFunctionName = 0);
    void FunctionPrepare(const char *pFunctionName);
    void PushString(const char *pString);
    void PushData(const char *pData, int Size);
    void PushInteger(int value);
    void PushFloat(float value);
    void PushBoolean(bool value);
    void PushParameter(const char *pString);
    int m_FunctionVarNum;

    //Functions:


	//general events:
	static inline int EventGetCID(lua_State *L);

    //Settings
    static inline int SetScriptUseSettingPage(lua_State *L);
    static inline int SetScriptTitle(lua_State *L);
    static inline int SetScriptInfo(lua_State *L);

    //Eventlistener stuff
    static inline int AddEventListener(lua_State *L);
    static inline int RemoveEventListener(lua_State *L);
    //All Eventlistener
    //OnWeaponFire
    //OnJump
    //OnJump
    //OnDie
    //OnExplosion
    //OnClientEnter
    //OnClientConnect
    //OnChat
    //OnPlayerJoinTeam
    //OnNetData
    //OnCanSpawn
    //OnEntity
    //OnConsole



    //
    //Include
    static inline int Include(lua_State *L);

    //emote
    static inline int Emote(lua_State *L);

    //Character
    static inline int GetCharacterPos(lua_State *L);
    static inline int SetCharacterPos(lua_State *L);
    static inline int GetCharacterVel(lua_State *L);
    static inline int SetCharacterVel(lua_State *L);

    //collision
    static inline int IntersectLine(lua_State *L);
    static inline int GetTile(lua_State *L);
    static inline int SetTile(lua_State *L);
    static inline int GetMapWidth(lua_State *L);
    static inline int GetMapHeight(lua_State *L);

    //Console Print
    static inline int Print(lua_State *L);
    static inline int Console(lua_State *L);

    //Serverinfo
    static inline int GetGameType(lua_State *L);
    static inline int IsTeamplay(lua_State *L);

    //Chat
    static inline int SendBroadcast(lua_State *L);
    static inline int SendChat(lua_State *L);
    static inline int SendChatTarget(lua_State *L);

    //Player
    static inline int GetPlayerIP(lua_State *L);
    static inline int GetPlayerName(lua_State *L);
    static inline int GetPlayerClan(lua_State *L);
    static inline int GetPlayerCountry(lua_State *L);
    static inline int GetPlayerScore(lua_State *L);
    static inline int GetPlayerPing(lua_State *L);
    static inline int GetPlayerTeam(lua_State *L);
    static inline int GetPlayerSkin(lua_State *L);
    static inline int GetPlayerColorFeet(lua_State *L);
    static inline int GetPlayerColorBody(lua_State *L);
    static inline int GetPlayerColorSkin(lua_State *L); //Todo: implement me
    static inline int GetPlayerSpectateID(lua_State *L);

    static inline int SetPlayerName(lua_State *L);
    static inline int SetPlayerClan(lua_State *L);
    static inline int SetPlayerCountry(lua_State *L);
    static inline int SetPlayerScore(lua_State *L);
    static inline int SetPlayerTeam(lua_State *L);
    static inline int SetPlayerSkin(lua_State *L);
    static inline int SetPlayerColorFeet(lua_State *L);
    static inline int SetPlayerColorBody(lua_State *L);
    static inline int SetPlayerSpectateID(lua_State *L);


    //Config
    static inline int GetConfigValue(lua_State *L);
    static inline int SetConfigValue(lua_State *L);

    //LuaNetWork
    static inline int SendPacket(lua_State *L);
    static inline int AddModFile(lua_State *L);
    static inline int DeleteModFile(lua_State *L);
    static inline int SendFile(lua_State *L);

    //Entities
    static inline int EntityFind(lua_State *L);
    static inline int EntityGetCharacterId(lua_State *L);
    static inline int EntityGetPos(lua_State *L);
    static inline int EntitySetPos(lua_State *L);
    static inline int EntityDestroy(lua_State *L);

    static inline int ProjectileFind(lua_State *L);
    static inline int ProjectileGetWeapon(lua_State *L);
    static inline int ProjectileGetOwner(lua_State *L);
    static inline int ProjectileGetPos(lua_State *L);
    static inline int ProjectileGetDir(lua_State *L);
    static inline int ProjectileGetLifespan(lua_State *L);
    static inline int ProjectileGetExplosive(lua_State *L);
    static inline int ProjectileGetSoundImpact(lua_State *L);
    static inline int ProjectileCreate(lua_State *L);

	//LaserCreate(Pos.x, Pos.y, Dir.x, Dir.y, StartEnergy, Owner)
	static inline int LaserCreate(lua_State *L);

    //Game
    static inline int CreateExplosion(lua_State *L);
    static inline int CreateDeath(lua_State *L);
    static inline int CreateDamageIndicator(lua_State *L);
    static inline int CreateHammerHit(lua_State *L);
    static inline int CreateSound(lua_State *L);

    //Client join


    //Spawn
    static inline int SetAutoRespawn(lua_State *L);

    static inline int CharacterSpawn(lua_State *L);
    static inline int CharacterIsAlive(lua_State *L);
    static inline int CharacterKill(lua_State *L);
    static inline int CharacterIsGrounded(lua_State *L);
    static inline int CharacterIncreaseHealth(lua_State *L);
    static inline int CharacterIncreaseArmor(lua_State *L);
    static inline int CharacterSetHealth(lua_State *L);
    static inline int CharacterSetArmor(lua_State *L);
    static inline int CharacterGetHealth(lua_State *L);
    static inline int CharacterGetArmor(lua_State *L);
    static inline int CharacterSetAmmo(lua_State *L);
    static inline int CharacterGetAmmo(lua_State *L);
    static inline int CharacterTakeDamage(lua_State *L);
    static inline int SendCharacterInfo(lua_State *L);
    //Input
    static inline int CharacterSetInputDirection(lua_State *L);
    static inline int CharacterSetInputJump(lua_State *L);
    static inline int CharacterSetInputWeapon(lua_State *L);
    static inline int CharacterSetInputTarget(lua_State *L);
    static inline int CharacterSetInputHook(lua_State *L);
    static inline int CharacterSetInputFire(lua_State *L);

    static inline int CharacterGetInputTarget(lua_State *L);
    static inline int CharacterGetActiveWeapon(lua_State *L);
    static inline int CharacterSetActiveWeapon(lua_State *L);

    static inline int CharacterDirectInput(lua_State *L);
    static inline int CharacterPredictedInput(lua_State *L);
    //core
    static inline int CharacterGetCoreJumped(lua_State *L);

    //this function should also tell the client which team
    static inline int Win(lua_State *L);

    static inline int SetGametype(lua_State *L);


    static inline int GetTuning(lua_State *L);
    static inline int SetTuning(lua_State *L);

    //Dummy
    static inline int DummyCreate(lua_State *L);
    static inline int IsDummy(lua_State *L);

    //Version
    static inline int CheckVersion(lua_State *L);
    static inline int GetVersion(lua_State *L);


    static inline int CreateDirectory(lua_State *L);

    static inline int GetDate(lua_State *L);


    //tick stuff
    static inline int GetTick(lua_State *L);
    static inline int GetTickSpeed(lua_State *L);












    //mysql stuff - welcome to the next level

    //lua functions
    static inline int MySQLConnect(lua_State *L);
    static inline int MySQLEscapeString(lua_State *L);
    static inline int MySQLSelectDatabase(lua_State *L);
    static inline int MySQLIsConnected(lua_State *L);
    static inline int MySQLQuery(lua_State *L);
    static inline int MySQLClose(lua_State *L);
    static inline int MySQLFetchResults(lua_State *L);

    // internal stuff
    MYSQL m_MySQL;
    bool m_MySQLConnected;
    inline void MySQLTick();
    inline void MySQLFreeResult(int Id, int QueryId);
    inline void MySQLFreeAll();
    inline void MySQLInit();
    int m_IncrementalQueryId;
    struct CQuery
    {
        int m_QueryId;
        char *m_pQuery;
        int m_Length;
    };
    class CField
    {
    public:
        enum TYPES
        {
            TYPE_INVALID = 0,
            TYPE_INTEGER,
            TYPE_FLOAT,
            TYPE_DATA,
        };
        TYPES m_Type;
        char *m_pData;
        long long m_Number;
        double m_Float;
        char *m_pName;
        long m_Length;

        CField()
        {
            m_Type = TYPE_INVALID;
            m_pData = 0;
            m_pName = 0;
        }
        ~CField()
        {
            if (m_pData)
                delete []m_pData;
            if (m_pName)
                delete []m_pName;
        }

    };
    struct CRow
    {
        CArray<CField *> m_lpFields;
    };
    struct CResults
    {
        int m_QueryId;
        bool m_Error;
        CArray<CRow *> m_lpRows;
        int64 m_Timestamp;
    };
    struct CMySQLThread
    {
        CLuaFile *m_pLua;
        volatile int m_Queries;
        void *pThread;
        bool m_Running;
        volatile LOCK m_MySSQLLock;
    };

    CArray<CQuery *> m_lpQueries;
    CArray<CResults *> m_lpResults;
    CMySQLThread m_MySQLThread;
    static void MySQLWorkerThread(void *pUser);

};

class CLua
{
public:
    CGameContext *m_pServer;
    CLua(CGameContext *pServer);
    ~CLua();
    void Tick();
	void TickDefered();
    void PostTick();
    bool Init(const char *pFile);
    void End();
    void Close();

    bool m_ConsoleInit;

    array<char *>m_lpEvalBuffer;
    void Eval(const char *pCode);
    CLuaFile m_aLuaFiles[MAX_LUA_FILES];
    class CLuaEventListener<CLuaFile> *m_pEventListener;

    //Search the file and execs the function
    void ConfigClose(char *pFilename);

    int GetFileId(char *pFilename);

	char *m_pMapLuaData;

    static void ConsolePrintCallback(const char *pLine, void *pUserData);
};


//helper functions
static int StrIsInteger(const char *pStr)
{
	while(*pStr)
	{
		if(!(*pStr >= '0' && *pStr <= '9'))
			return 0;
		pStr++;
	}
	return 1;
}
static int StrIsFloat(const char *pStr)
{
	while(*pStr)
	{
		if(!((*pStr >= '0' && *pStr <= '9') || *pStr == '.'))
			return 0;
		pStr++;
	}
	return 1;
}

#endif
