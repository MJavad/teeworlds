Config = {}

include ("lua/gametexture_changer.config")

SetScriptTitle("Gametexture Changer")
SetScriptInfo("Script by MAP94 - Textures by TheGnaa, Jon, Zatline and Saturn")
SetScriptUseSettingPage(1)

Directory = ListDirectory("data/gameskins/")

Textures = {}
Selected = UiGetGameTextureID()
for k, v in ipairs(Directory) do
    if (v["dir"] == false) then
        Textures[#Textures + 1] = {}
        Textures[#Textures]["name"] = v["name"]
        Textures[#Textures]["id"] = TextureLoad("data/gameskins/" .. v["name"])
        if (v["name"] == Config["Name"]) then
            Selected = Textures[#Textures]["id"]
            ReplaceGameTexture(Selected)
        end
    end
end

function Tick()

end

function SetTexture(State, Id)
    for k, v in ipairs(Textures) do
        if (v["ui_img_id"] == Id) then
            x, y = UiGetRect(Id)
            UiSetRect(UiHighlight, x - 5, y - 5)
            Selected = v["id"]
            Config["Name"] = v["name"]
            ReplaceGameTexture(Selected)
        end
    end

    configout = io.open("lua/gametexture_changer.config", "wb")
    configout:write("--Configfile for Gametexture Changer\n")
    configout:write("Config = {}\n")
    configout:write("Config[\"Name\"] = \"" .. Config["Name"] .. "\"\n")
    configout:close()
    include ("lua/gametexture_changer.config")

end


UiHighlight = nil
function ConfigOpen(x, y, w, h)
    local i = 0
    UiHighlight = UiDoRect(x, y, 160, 100, 0, nil, nil, 1, 1, 1, 0.5)
    for k, v in ipairs(Textures) do
        Textures[k]["ui_img_id"] = UiDoImageEx(x + 5 + 165 * (i % 3), y + 5 + 110 * math.floor(i / 3), 150, 75, 0, v["id"], nil, nil, nil, nil, "SetTexture")
        Textures[k]["ui_txt_id"] = UiDoLabel(x + 5 + 165 * (i % 3), y + 80 + 110 * math.floor(i / 3), 150, 75, 0, v["name"])
        i = i + 1
    end

end

function ConfigClose()
    UiRemoveElement(UiHighlight)
    for k, v in ipairs(Textures) do
        UiRemoveElement(Textures[k]["ui_img_id"])
        UiRemoveElement(Textures[k]["ui_txt_id"])
    end
end
