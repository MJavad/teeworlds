-- N-Server documentation by Patafix (c)
--sets the title of the script
SetScriptTitle("New Server-Mod")
--sets information about the Lua-Mod
SetScriptInfo("(c) by  ")


--main functions (needed in order to add the file):
	iTick = 0
	GTime = 0
	function Tick(Time, ServerTick)
		iTick = iTick+1
		GTime = Time
	end
	function TickDefered(Time, ServerTick)

	end
	function PostTick(Time, ServerTick)

	end
	
-- events:
	--OnChat: If someone writes something in the chat
		AddEventListener("OnChat", "OwnOnChat")
		function OwnOnChat()		
			Text	=		ChatGetText()		--get the Text which has been written
			ID		=		ChatGetClientID()	--get the chatter's ID
			Team	=		ChatGetTeam() 		--get the team ID
							ChatHide()			--hide that message
		end

    --Die: If someone dies
		AddEventListener("OnDie", "OwnOnDie")
		function OwnOnDie()			
			KillerID	=		DieGetKillerID() 	--get the ID of the Killer
			ID			=		DieGetVictimID()	--get the ID of the dieing character
			WeaponID 	= 		DieGetWeaponID()	--get the used weapon's ID
		end
		
    --OnNetData: If a Lua-Packet arrives
		AddEventListener("OnNetData", "OwnOnNetData")
		function OwnOnNetData()			
			Data	=		FetchPacket() 		--get the packet
			From	=		GetPacketClientID() --get the ID of the sender		
		end

	--OnWeaponFire: If someone fires with a weapon
		AddEventListener("OnWeaponFire", "OwnOnWeaponFire")
		function OwnOnWeaponFire()			
			DirX,DirY	=		WeaponFireGetDir() 				--get direction
			WeaponID	=		WeaponFireGetWeaponID() 		--get the weapon ID	
			ID			=		WeaponFireGetClientID() 		--get the client ID
								WeaponFireSetReloadTime(100)	--set the reloadtimer to 100 ms
								WeaponFireDisableSound()		--disable the current fire's sound
								WeaponFireAutoFire(true)		--set the autofire to true/false
		end
	--Jump: If someone jumps
		AddEventListener("OnJump", "OwnOnJump")
		function OwnOnJump()			
			ID		=		JumpGetClientID() 		--get the client ID
			Jump	=		JumpGetJumpID()			--get the jump ID (ground/airjump)	
		end	

	--OnExplosion: If a explosion occures
		AddEventListener("OnExplosion", "OwnOnExplosion")
		function OwnOnExplosion()			
			ID			=		ExplosionGetOwner()		--get the client ID
			Damage 		= 		ExplosionGetDamage()	--get the damage
			WeaponID	=		ExplosionGetWeapon()	--get the weapon ID
			PosX, PosY	= 		ExplosionGetPos()		--get the position
								ExplosionAbort()		--abort the explosion
		end
	
	--OnEntity: If gamecontroller's OnEntity(...) is called
		AddEventListener("OnEntity", "OwnOnEntity")
		function OwnOnEntity()			
			Index		=		GetOnEntityIndex()		--get the entity Index			
			PosX, PosY	= 		GetOnEntityPosIndex()	--get the position
								SetOnEntityIndex(2)		--change the current entity index
		end

	--OnConnect: If a player connects to the game
		AddEventListener("OnConnect", "OwnOnConnect")
		function OwnOnConnect()			
			ID		=		GetClientConnectClientID()	--get the client ID			
		end

	--OnEnter: If a player enters the game
		AddEventListener("OnEnter", "OwnOnEnter")
		function OwnOnEnter0()			
			ID		=		GetClientEnterClientID()	--get the client ID			
		end
	--OnCanSpawn: If gamecontroller's CanSpawn(...) is called
		AddEventListener("OnCanSpawn", "OwnOnCanSpawn")
		function OwnOnCanSpawn()			
			Team	=		SpawnGetTeam()	--get the team
							AbortSpawn()	--abort the spawn
		end
	--OnPlayerJoinTeam: If gamecontroller's CanJoinTeam(...) is called
		AddEventListener("OnPlayerJoinTeam", "OwnOnPlayerJoinTeam")
		function OwnOnPlayerJoinTeam()			
			Team	=		GetSelectedTeam()		--the wanted team
			ID		= 		GetJoinTeamClientID() 	--get the client ID
							AbortTeamJoin()			--abort the join
		end

		
		
		
--functions:
	[CID] = int, [x, y] = float, float, [velx, vely] = float, float
		[a]	= int, [b] = boolean, [c] = float, [s] = String, ? = optional, / = Or
	
	--character:
		
		PosX, PosY	=	GetCharacterPos(CID)				--get the position of the character with CID, returns nil if char is null
						SetCharacterPos(CID, x, y)			--sets the position of the charaacter with CID to vec2(x, y)
						
		VelX, VelY	= 	GetCharacterVel(CID)				--get the velocity of the character with CID, returns nil if char is null
						SetCharacterVel(CID, velx, vely)	--sets the vel of the charaacter with CID to vec2(velx, vely)
						
						Emote(CID, EmoteID)					--set the emote of the character
						
						CharacterSpawn(CID, ?posx, ?posy) 	--without position the player will just try to respawn, with position the character spawns at given position
						
		Alive		=	CharacterIsAlive(CID)				--returns true/false
						CharacterKill(CID)					--kills the charracter
		
		
						CharacterSetInputDirection(CID, a)		-- a = {-1, 0, 1}, sets the input direction of the character to a
						CharacterSetInputJump(CID, a)			-- a = {0, 1, 2}, sets the jump input to a
						CharacterSetInputWeapon(CID, a)			-- a = weapon id, sets the wanted weapon to a
						CharacterSetInputTarget(CID, x, y) 		-- sets the relative target to vec2(x, y)
						CharacterSetInputHook(CID, a)			-- a = {0, 1}, sets the hook input to a
						CharacterSetInputFire(CID, a)			-- a = {0, 1}, sets the fire input to a
		
		DirX, DirY 	= 	CharacterGetInputTarget(CID)			--returns the target dir (int)
		
						CharacterDirectInput(CID)				--"OnDirectInput"
						CharacterPredictedInput(CID)			--"OnPredictedInput"
						
		WeaponID	= 	CharacterGetActiveWeapon(CID)			--returns the weapon ID
		
		JumpState	= 	CharacterGetCoreJumped(CID)				--returns the core jump state		
		
		IsGrounded 	= 	CharacterIsGrounded(CID)				--returns true if character is grounded, false if not
		
						CharacterIncreaseHealth(CID, ?Amount, ?Max)	--increases the health by 1 or given amount to 10 or a given Max
						CharacterIncreaseArmor(CID, ?Amount, ?Max)	--increases the armor by 1 or given amount to 10 or a given Max
						
						CharacterSetAmmo(CID, WeaponID, ?a)		--sets the ammo of the weapon to 10 or given value and sets also m_Got to true
		Ammo, Got		CharacterGetAmmo(CID)					--returns the ammo and then got(int)

						
		
	--chat:
		
						SendChatTarget(To, Msg)				--sends the message to To
						SendChat(From, To, Msg)				--sends the message from From to To
						
						SendBroadcast(Msg, ?To)				--broadcasts the message to -1 or goven To
	
	--collision:
	
		IsCollision, OutX, OutY, OutBeforeX, OutBeforeY	=	IntersectLine(x1, y1, x2, y2)	--the regular intersect line function
		TileIndex										=	GetTile(x, y)					--gets the TileIndex at the pos vec2(x, y)	
															SetTile(x, y, a)				--sets the TileINdex at the pos vec2(x, y) to a
		Width											= 	GetMapWidth()					--returns the map width
		Height											= 	GetMapheight()					--return the map height
		
	--config:
	
		Value 											= 	GetConfigValue(s)				--returns the value of the config variable named s= {"Name", "Password", "Map", "RconPassword", "Gametype"}
															SetConfigValue(s1, a/s2)		--sets the value of the config variable named s1= {"Name", "Password", "Map", "RconPassword", "Gametype"} to a or s2
															
	--console:
		
		Console(a, s1, s2)		--prints with level a, name s1 and msg s2
		Print(s1, s2)			--prints a debug msg s2 in s1
		
	--dummy:
		
		Success 	=	DummyCreate(CID) 	--returns true if the dummy creation was successfull, false if not
		IsDummy		= 	IsDummy(CID)		--returns true if the character is a dummy, false if not
	
	--entities:
	
		aIDs 			= 	EntityFind(x, y, a1, a2, a3) 	--searches for entites with type a1 at pos vec2(x, y) and returns a table of IDs, a2=max, a3=radius
		ID 				=	EntityGetCharacterId(a)			--gives the character ID of the entity with a=EntityID, if there is one, returns nil if not
		PosX, PosY		= 	EntityGetPos(a)					--returns the position of the entity with ID=a, if the entity is null, nil will be returned
							EntitySetPos(a, x, y)			--sets the position of the entity with the ID=a to vec2(x, y)
							EntityDestroy(a)				--detroys the entity with the ID==a
						
		aProjectileIDs		ProjectileFind(x, y, a1, a2) 	--searches for projectiles at pos vec2(x, y) and returns a table of IDs, a2=max, a1=radius		
		WeaponID 		= 	ProjectileGetWeapon(a)			--returns the wepaon id of the projectile with ID==a, if the projectile is null, nil will be returned		
		ProjOwner		=	ProjectileGetOwner(a)			--returns the owner id of the projectile with ID==a, if the projectile is null, nil will be returned
		PosX, PosY 		= 	ProjectileGetPos(a)				--returns the position of the projectile with ID==a, if the projectile is null, nil will be returned
		DirX, DirY 		= 	ProjectileGetDir(a)				--returns the direction of the projectile with ID==a, if the projectile is null, nil will be returned
		LifeSpan		= 	ProjectileGetLifespan(a)		--returns the lifespan of the projectile with ID==a, if the projectile is null, nil will be returned
		Explosive		= 	ProjectileGetExplosive(a)		--returns the explosive of the projectile with ID==a, if the projectile is null, nil will be returned
		SoundImpact		= 	ProjectileGetSoundImpact(a)		--returns the sound impact of the projectile with ID==a, if the projectile is null, nil will be returned
		
		ProjectileCreate(x1, y1, ?x2, ?y2, ?a1, ?a2, ?a3, ?a4, ?c1, ?b, ?c2) -- Dir=vec2(x2, y2), a1=ID, a2=LifeSpan, a3=Type/WeaponID, a4=Damage, c1=Force, b=Explosive, c2=ImpactSound
		CharacterTakeDamage(a1, a2, a3, a4)									 --a1=entityID, a2=Dmg, a3=From, a4=Weapon  TODO:	Move these char function to character... |||||||||||||||||||||||||||||||||||||||||||
		
		Health 			= 	CharacterGetHealth(a)			--returns the health of the entity with ID==a, if there is no char with this ID it will return nil
		Armor 			= 	CharacterGetArmor(a)			--returns the armor of the entity with ID==a, if there is no char with this ID it will return nil
		
		LaserCreate(x1, y1, x2, y2, c, a)					--creates a laser with pos=vec(x1, y1), dir=vec2(x2, y2), c=StartEnergy, a=Owner
	
	--game:
	
		GameType			=		GetGameType()			--returns the string of the current gametype
		IsTeamplay			= 		IsTeamplay()			--returns true/false
		
		CreateExplosion(x, y, a1, a2, b, a3)				--creates an explosion at vec2(x, y) from owner a1 with weapon a2, b1=NoDamage, a3=damage
		CreateDeath(x, y, ?a)								--creates a death at vec2(x, y) from -1 or given a
		CreateDamageIndicator(x, y, c, a)					--creates a damage indicator at vec2(x, y) with an angle c and amount a
		SetGametype(s)										--sets the gametype to s
		Win()												--ends the round
		
	--message:
	
		SendPacket(Data, ?a)								--sends a package with Data to -1 or given a
		AddModFile(s1, s2, a1, ?a2)							--adds a modfile, s1=Filedir (/asd.png), s2=Name(healthbar), a1 = type  a1={0:Invalid, 1:Lua, 2:PNG, 3:Wave, 4:WV, 5:Other(name should with file extension [healthbar.txt])},
															-- a2=FileHandleFlag (if not given it's 0) a2={IGNORETYPE = 1, NOCRC = 2, LAUNCH = 4,  SAVE = 8, CHECK = 16, UPDATE = 32}
		DeleteModFile(s)									--deletes the modfile with the path s
		SendFile(a)											--sends the modfiles to client a
	        
	--player:
	
		Name									= 		GetPlayerName(a)				--returns the name of the player a, if the player is null, nil will be returned
		ClanName								= 		GetPlayerClan(a)				--returns the clan of the player a, if the player is null, nil will be returned
		Country									= 		GetPlayerCountry(a)				--returns the country of the player a, if the player is null, nil will be returned
		Score									= 		GetPlayerScore(a)				--returns the score of the player a, if the player is null, nil will be returned
		Ping									= 		GetPlayerPing(a)				--returns the ping of the player a, if the player is null, nil will be returned
		Team									= 		GetPlayerTeam(a)				--returns the Team(int) of the player a, if the player is null, nil will be returned
		Skin									= 		GetPlayerSkin(a)				--returns the skin(string) of the player a, if the player is null, nil will be returned
		FeetColorR, FeetColorG, FeetColorB		= 		GetPlayerColorFeet(a)			--returns the feet color of the player a, if the player is null, nil will be returned
		BodyColorR, BodyColorG, BodyColorB		= 		GetPlayerColorBody(a)			--returns the body color of the player a, if the player is null, nil will be returned
		
		
		SetPlayerScore(a1, a2)				--sets the score of player a1 to a2
		SetPlayerTeam(a1, a2)				--sets the team of player a1 to a2
		SetPlayerName(a, s)					--sets the name of player a to s
		SetPlayerClan(a, s)					--sets the clan of player a to s
		SetPlayerCountry(a, s)				--sets the country of player a to s	
		SetPlayerColorBody(a, c1, c2, c3)	--sets the body color of player a to RGB(c1, c2, c3)
		SetPlayerColorFeet(a, c1, c2, c3)	--sets the feet color of player a to RGB(c1, c2, c3)