local json = require("../Include/json")

local trans = nil
local isOpen = false
local openAngle = 0.0
local closedAngle = 0.0

local isAnimating = false
local animationDuration = 1.0
local timer = 0.0
local curvePower = 1.0
local editorAnimDuration = 1.0


function Lerp(a, b, t)
	return a + (b - a) * t
end

function SmoothLerp(a, b, t)
	return a + (b - a) * t
end


function OnStart()
    trans = self:GetTransform()
end

function OnUpdate(dt)
    if isAnimating then
        if trans == nil then
            trans = self:GetTransform()
        end

        timer = timer + dt
        local t = math.min(timer / animationDuration, 1.0)
        t = t ^ curvePower -- Apply curve power for easing

        if t >= 1.0 then
            isAnimating = false
            timer = 0.0
            t = 1.0

            -- TODO: Play door fully opened/closed sound here
        end

        local sourceAngle = isOpen and closedAngle or openAngle
        local targetAngle = isOpen and openAngle or closedAngle
        local currentAngle = Lerp(sourceAngle, targetAngle, t)

        local eulerRot = trans:GetEuler(TransformSpace.Local)
        eulerRot.x = 0.0
        eulerRot.y = currentAngle * (math.pi / 180.0) -- Convert degrees to radians
        eulerRot.z = 0.0
        trans:SetEuler(eulerRot, TransformSpace.Local)
    end
end

function OpenDoor(duration)
    if isOpen then
        return
    end

    isOpen = true
    animationDuration = duration or 1.0
    timer = 0.0
    isAnimating = true

    if trans == nil then
        trans = self:GetTransform()
    end

    local eulerRot = trans:GetEuler(TransformSpace.Local)
    eulerRot.x = 0.0
    eulerRot.y = openAngle * (math.pi / 180.0)
    eulerRot.z = 0.0
    trans:SetEuler(eulerRot, TransformSpace.Local)

    -- TODO: Play door opening sound here
end

function CloseDoor(duration)
    if not isOpen then
        return
    end

    isOpen = false
    animationDuration = duration or 1.0
    timer = 0.0
    isAnimating = true

    if trans == nil then
        trans = self:GetTransform()
    end

    local eulerRot = trans:GetEuler(TransformSpace.Local)
    eulerRot.x = 0.0
    eulerRot.y = closedAngle * (math.pi / 180.0)
    eulerRot.z = 0.0
    trans:SetEuler(eulerRot, TransformSpace.Local)

    -- TODO: Play door closing sound here
end

function RenderUI()
    _, isOpen = ImGui.Checkbox("Open", isOpen)
    _, openAngle = ImGui.DragFloat("Open Angle", openAngle, 0.01)
    _, closedAngle = ImGui.DragFloat("Closed Angle", closedAngle, 0.01)
    _, curvePower = ImGui.DragFloat("Curve Power", curvePower, 0.01)

    _, editorAnimDuration = ImGui.DragFloat("Animation Duration", editorAnimDuration, 0.01)
    if ImGui.Button("Play") then        
        if isOpen then
            CloseDoor(editorAnimDuration)
        else
            OpenDoor(editorAnimDuration)
        end
    end
end

function Serialize()
    local data = {
        state = isOpen,
        openAngle = openAngle,
        closedAngle = closedAngle,
        curvePower = curvePower
    }
    return json.encode(data)
end

function Deserialize(data)
    local decoded = json.decode(data)
    if decoded then
        if decoded.state then
            isOpen = decoded.state
        end
        if decoded.openAngle then
            openAngle = decoded.openAngle
        end
        if decoded.closedAngle then
            closedAngle = decoded.closedAngle
        end
        if decoded.curvePower then
            curvePower = decoded.curvePower
        end
    end
end
