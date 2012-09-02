SetScriptTitle("Galaxy")
SetScriptInfo("(c) by MAP94")
SetScriptUseSettingPage(0)

AddEventListener("OnRenderBackground", "OnRenderBackground")

BackGround = TextureLoad("background.png")
Nebula01 = TextureLoad("nebula01.png")
Nebula02 = TextureLoad("nebula02.png")
Nebula03 = TextureLoad("nebula03.png")
Nebula04 = TextureLoad("nebula04.png")
Nebula05 = TextureLoad("nebula05.png")
Moon = TextureLoad("moon.png")
Planet = TextureLoad("planet.png")
asp = UiGetScreenWidth() / UiGetScreenHeight()

function RenderNebula(Image, x, y, w, h, r, g, b, a, rot)
    RenderTexture(Image, x - w / 2, y - h / 2, w, h, nil, nil, nil, nil, r, g, b, a, rot)
end

function Clamp(val, min, max)
    if (val < min) then
        return min
    end
    if (val > max) then
        return max
    end
    return val
end


function OnRenderBackground()
    RenderTexture(BackGround, 0, 0, 300 * asp, 300)

    BlendAdditive()
    local t1 = GTime % 1000 / 1000 * math.pi * 2.5 + math.pi * 0.5
    local t2 = GTime % 500 / 500 * math.pi * 3 + math.pi * 0.5
    local t3 = GTime % 5000 / 5000 * math.pi * 4

    RenderNebula(Nebula01, 200 * asp / 2, 150, 300 * asp, 300, 1, 0, 1, math.sin(Clamp(t1, 0, math.pi)), t3)
    RenderNebula(Nebula02, 200 * asp / 2, 150, 300 * asp, 300, 1, 0, 0, math.sin(Clamp(t1 - math.pi * 0.5, 0, math.pi)), t3)
    RenderNebula(Nebula03, 200 * asp / 2, 150, 300 * asp, 300, 1, 1, 0, math.sin(Clamp(t1 - math.pi * 1, 0, math.pi)), t3)
    RenderNebula(Nebula04, 200 * asp / 2, 150, 300 * asp, 300, 0, 1, 0, math.sin(Clamp(t1 - math.pi * 1.5, 0, math.pi)), t3)
    RenderNebula(Nebula05, 200 * asp / 2, 150, 300 * asp, 300, 0, 0, 1, math.sin(Clamp(t1 - math.pi * 2, 0, math.pi)), t3)
    RenderNebula(Nebula01, 200 * asp / 2, 150, 300 * asp, 300, 1, 0, 1, math.sin(Clamp(t1 - math.pi * 2.5, 0, math.pi)), t3)

    RenderNebula(Nebula01, 300 * asp / 2, 150, 300 * asp, 300, 1, 0, 1, math.sin(Clamp(t1, 0, math.pi)), t3)
    RenderNebula(Nebula02, 300 * asp / 2, 150, 300 * asp, 300, 1, 0, 0, math.sin(Clamp(t1 - math.pi * 0.5, 0, math.pi)), t3)
    RenderNebula(Nebula03, 300 * asp / 2, 150, 300 * asp, 300, 1, 1, 0, math.sin(Clamp(t1 - math.pi * 1, 0, math.pi)), t3)
    RenderNebula(Nebula04, 300 * asp / 2, 150, 300 * asp, 300, 0, 1, 0, math.sin(Clamp(t1 - math.pi * 1.5, 0, math.pi)), t3)
    RenderNebula(Nebula05, 300 * asp / 2, 150, 300 * asp, 300, 0, 0, 1, math.sin(Clamp(t1 - math.pi * 2, 0, math.pi)), t3)
    RenderNebula(Nebula01, 300 * asp / 2, 150, 300 * asp, 300, 1, 0, 1, math.sin(Clamp(t1 - math.pi * 2.5, 0, math.pi)), t3)

    RenderNebula(Nebula01, 200 * asp / 2, 150, 150 * asp, 150, 0, 1, 0, math.sin(Clamp(t2, 0, math.pi)) * 0.75, t3 * 0.5)
    RenderNebula(Nebula02, 200 * asp / 2, 150, 150 * asp, 150, 0, 1, 1, math.sin(Clamp(t2 - math.pi * 0.5, 0, math.pi)) * 0.75, t3 * 0.5)
    RenderNebula(Nebula03, 200 * asp / 2, 150, 150 * asp, 150, 0, 0, 1, math.sin(Clamp(t2 - math.pi * 1, 0, math.pi)) * 0.75, t3 * 0.5)
    RenderNebula(Nebula04, 200 * asp / 2, 150, 150 * asp, 150, 0, 1, 1, math.sin(Clamp(t2 - math.pi * 1.5, 0, math.pi)) * 0.75, t3 * 0.5)
    RenderNebula(Nebula05, 200 * asp / 2, 150, 150 * asp, 150, 0, 1, 0, math.sin(Clamp(t2 - math.pi * 2, 0, math.pi)) * 0.75, t3 * 0.5)
    RenderNebula(Nebula04, 200 * asp / 2, 150, 150 * asp, 150, 1, 1, 0, math.sin(Clamp(t2 - math.pi * 2.5, 0, math.pi)) * 0.75, t3 * 0.5)
    RenderNebula(Nebula01, 200 * asp / 2, 150, 150 * asp, 150, 0, 1, 0, math.sin(Clamp(t2 - math.pi * 3, 0, math.pi)) * 0.75, t3 * 0.5)

    BlendNormal()

    RenderNebula(Moon, 300 * asp - 50, 25, 40, 40)
    RenderNebula(Planet, 300 * asp / 2, 150, 200, 200)
end

GTime = 0
function Tick(Time, ServerTick)
GTime = Time / 50

end
