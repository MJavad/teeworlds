SetScriptTitle("Microbes")
SetScriptInfo("(c) by MAP94")
SetScriptUseSettingPage(0)

AddEventListener("OnRenderBackground", "OnRenderBackground")

MicrobesImage = TextureLoad("microbes.png")
FoodImage = TextureLoad("food.png")

a = UiGetScreenWidth() / UiGetScreenHeight()

Microbes = {}
MicrobesCounter = 0
for i = 0, 16 do
    MicrobesCounter = MicrobesCounter + 1
    Microbes[MicrobesCounter] = {}
    Microbes[MicrobesCounter]["x"] = math.random(0, 300 * a)
    Microbes[MicrobesCounter]["y"] = math.random(0, 300)
    Microbes[MicrobesCounter]["vx"] = math.random(-10, 10) / 50
    Microbes[MicrobesCounter]["vy"] = math.random(-10, 10) / 50

    local x = math.random(0, 4)
    if (x == 0) then
        Microbes[MicrobesCounter]["r"] = 1
        Microbes[MicrobesCounter]["g"] = 0
        Microbes[MicrobesCounter]["b"] = 0
    end
    if (x == 1) then
        Microbes[MicrobesCounter]["r"] = 0
        Microbes[MicrobesCounter]["g"] = 0
        Microbes[MicrobesCounter]["b"] = 1
    end
    if (x == 2) then
        Microbes[MicrobesCounter]["r"] = 0
        Microbes[MicrobesCounter]["g"] = 1
        Microbes[MicrobesCounter]["b"] = 0
    end
    if (x == 3) then
        Microbes[MicrobesCounter]["r"] = 1
        Microbes[MicrobesCounter]["g"] = 0.7
        Microbes[MicrobesCounter]["b"] = 0
    end
    Microbes[MicrobesCounter]["a"] = 1
    Microbes[MicrobesCounter]["p"] = 0.5
    Microbes[MicrobesCounter]["hp"] = 1
end

Food = {}
FoodCounter = 0
for i = 0, 10 do
    FoodCounter = FoodCounter + 1
    Food[FoodCounter] = {}
    Food[FoodCounter]["x"] = math.random(0, 300 * a)
    Food[FoodCounter]["y"] = math.random(0, 300)
end

function Clamp(val, max, min)
    if (val < min) then
        return min
    end
    if (val > max) then
        return max
    end
    return val
end

function Normalize(x, y)
    if (x == 0 and y == 0) then
        return x, y
    end
    l = 1 / math.sqrt(x*x + y*y)
	return x*l, y*l
end

function Distance(x1, y1, x2, y2)
    local x = (x1 - x2)
    local y = (y1 - y2)
    return math.sqrt(x * x + y * y)
end

function OnRenderBackground()
    mx, my = GetMousePosMenu()
    mx = mx / UiGetScreenWidth() * 300 * a
    my = my / UiGetScreenHeight() * 300
    UiDirectRect(0, 0, 300 * a, 300, 0, 0, 0, 1, 0)
    --RenderTexture(BackGround, 0, 0, 300 * a, 300)
    BlendAdditive()

    if (math.random(0, 50) == 1) then
        FoodCounter = FoodCounter + 1
        Food[FoodCounter] = {}
        Food[FoodCounter]["x"] = math.random(0, 300 * a)
        Food[FoodCounter]["y"] = math.random(0, 300)
    end

    for k, v in pairs(Food) do
        RenderTexture(FoodImage, v["x"] - 2, v["y"] - 2, 4, 4, nil, nil, nil, nil, 1, 1, 1, 1)
    end
    for k, v in pairs(Microbes) do
        if (v["x"] > 300 * a + 30) then
            v["x"] = -30
        end
        if (v["x"] < -30) then
            v["x"] = 300 * a + 30
        end
        if (v["y"] > 300 + 30) then
            v["y"] = -30
        end
        if (v["y"] < -30) then
            v["y"] = 300 + 30
        end

        local x = v["x"]
        local y = v["y"]

        local Best = 100
        local dxmod = 0
        local dymod = 0
        for kf, vf in pairs(Food) do
            if (Distance(vf["x"], vf["y"], v["x"], v["y"]) < Best) then
                Best = Distance(vf["x"], vf["y"], v["x"], v["y"])
                if (Best < 2) then
                    Food[kf] = nil
                    v["p"] = v["p"] + 0.1
                    v["hp"] = Clamp(v["hp"] + 1, 5, 0)
                end
                if (Best < 100) then
                    dxmod = vf["x"] - v["x"]
                    dymod = vf["y"] - v["y"]
                end
            end
        end

        dxmod, dymod = Normalize(dxmod, dymod)

        v["vx"] = v["vx"] + math.random(-10, 10) / 50
        v["vy"] = v["vy"] + math.random(-10, 10) / 50

        v["vx"] = Clamp(v["vx"] + dxmod / 10, (1.5 - (Best / 100)) * 0.75, -(1.5 - (Best / 100)) * 0.75)
        v["vy"] = Clamp(v["vy"] + dymod / 10, (1.5 - (Best / 100)) * 0.75, -(1.5 - (Best / 100)) * 0.75)

        if (Best == 100) then
            if (v["vx"] > 0.5) then
                v["vx"] = v["vx"] - v["vx"] / 100
            end
            if (v["vx"] < -0.5) then
                v["vx"] = v["vx"] + v["vx"] / 100
            end
            if (v["vy"] > 0.5) then
                v["vy"] = v["vy"] - v["vy"] / 100
            end
            if (v["vy"] < -0.5) then
                v["vy"] = v["vy"] + v["vy"] / 100
            end
        end

        if (Distance(v["x"], v["y"], mx, my) < 30) then
            dx, dy = Normalize(mx - v["x"], my - v["y"])
            v["x"] = v["x"] - dx * (30 - Distance(v["x"], v["y"], mx, my))
            v["y"] = v["y"] - dy * (30 - Distance(v["x"], v["y"], mx, my))
        end

        v["x"] = v["x"] + v["vx"] * 0.3
        v["y"] = v["y"] + v["vy"] * 0.3

        local dx = v["x"] - x
        local dy = v["y"] - y

        local a = math.atan(dy / dx)
        if (dx == 0 and dy == 0) then
            a = 0
        elseif (dx == 0 and dy < 0) then
            a = math.pi * 1.5
        elseif (dx == 0 and dy > 0) then
            a = math.pi * 0.5
        end
        if (dx < 0) then
            a = a + math.pi
        end

        a = a + math.pi * 0.5

        if (v["p"] > 0.9) then
            v["p"] = 0.5
            MicrobesCounter = MicrobesCounter + 1
            Microbes[MicrobesCounter] = {}
            Microbes[MicrobesCounter]["x"] = v["x"]
            Microbes[MicrobesCounter]["y"] = v["y"]
            Microbes[MicrobesCounter]["vx"] = 0
            Microbes[MicrobesCounter]["vy"] = 0
            Microbes[MicrobesCounter]["p"] = v["p"]
            Microbes[MicrobesCounter]["r"] = v["r"]
            Microbes[MicrobesCounter]["g"] = v["g"]
            Microbes[MicrobesCounter]["b"] = v["b"]
            Microbes[MicrobesCounter]["a"] = v["a"]
            Microbes[MicrobesCounter]["hp"] = v["hp"]
        end

        v["hp"] = v["hp"] - 0.0001
        if (v["hp"] < 0) then
            Microbes[k] = nil
        end

        RenderTexture(MicrobesImage, v["x"] - 32 * v["p"] / 2, v["y"] - 32 * v["p"] / 2, 32 * v["p"], 32 * v["p"], nil, nil, nil, nil, v["r"], v["g"], v["b"], v["a"], a)
    end
    BlendNormal()
end


function Tick(Time, ServerTick)


end
