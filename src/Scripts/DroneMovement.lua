-- DroneMovement.lua

--
-- Member Variables
--

-- Movement settings
local acceleration = 5.0
local maxSpeed = 8.0
local drag = 1.5

-- Runtime state
local trans = nil
local velocity = nil

--
-- Member functions
--

function GetVerticalInput()
    local vertical = 0.0

    if Input:KeyHeld(Key.Space) then
        vertical = vertical + 1.0
    end

    if Input:KeyHeld(Key.LCTRL) then
        vertical = vertical - 1.0
    end

    if vertical ~= 0.0 then
        print("VerticalInput: " .. vertical)
    end

    return vertical
end

function UpdateMovement(dt)
    -- Build the movement input directions
    local inputDirection = Vec3.new(0.0, 0.0, 0.0)

    -- Forward and backward
    if Input:KeyHeld(Key.W) then
        inputDirection = GLM.V3Add(
            inputDirection,
            trans:GetForward(TransformSpace.World)
        )
    end

    if Input:KeyHeld(Key.S) then
        inputDirection = GLM.V3Sub(
            inputDirection,
            trans:GetForward(TransformSpace.World)
        )
    end

    -- Strafe left and right
    if Input:KeyHeld(Key.A) then
        inputDirection = GLM.V3Add(
            inputDirection,
            trans:GetRight(TransformSpace.World)
        )
    end

    if Input:KeyHeld(Key.D) then
        inputDirection = GLM.V3Sub(
            inputDirection,
            trans:GetRight(TransformSpace.World)
        )
    end

    -- Dedicated vertical movement in world space

    if Input:KeyHeld(Key.Space) then
        inputDirection = GLM.V3Add(
            inputDirection,
            trans:GetUp(TransformSpace.World)
        )
    end

    if Input:KeyHeld(Key.LCTRL) then
        inputDirection = GLM.V3Sub(
            inputDirection,
            trans:GetUp(TransformSpace.World)
        )
    end

    -- Normalize only if the direction is longer than 1
    -- This prevents diagonal movement from increasing speed
    local inputLength = GLM.V3Length(inputDirection)

    if inputLength > 1.0 then
        inputDirection = GLM.V3Normalize(inputDirection)
    end

    -- Accelerate while input is held
    if inputLength > 0.0 then 
        local accelerationStep = GLM.V3MulF(
            inputDirection,
            acceleration * dt
        )

        velocity = GLM.V3Add(velocity, accelerationStep)
    else
        local dragFactor = math.max(0.0, 1.0 - drag * dt)

        velocity = GLM.V3MulF(velocity, dragFactor)
    end

    -- Limit the total velocity, regardless of direction
    local speed = GLM.V3Length(velocity)

    if speed > maxSpeed then
        velocity = GLM.V3MulF(
            GLM.V3Normalize(velocity),
            maxSpeed
        )
    end

    -- Apply movement in world space
    local movement = GLM.V3MulF(velocity, dt)

    trans:Move(movement, TransformSpace.World)
end



--
-- Auto Functions
--

function OnStart()
    tracy.ZoneBeginN("Drone_movement OnStart")

    print("New Drone_movement")
	trans = self:GetTransform()
    velocity = Vec3.new(0.0, 0.0, 0.0)
    
    tracy.ZoneEnd()
end

function OnUpdate(dt)
    tracy.ZoneBeginN("Drone_movement OnUpdate")

	UpdateMovement(dt)

    tracy.ZoneEnd()
end

function RenderUI()
    tracy.ZoneBeginN("Drone_movement RenderUI")

    ImGui.Spacing()

    _, acceleration = ImGui.SliderFloat("Acceleration", acceleration, 0.5, 20.0)
    _, maxSpeed = ImGui.SliderFloat("Maximum Speed", maxSpeed, 1.0, 30.0)
    _, drag = ImGui.SliderFloat("Drag", drag, 0.0, 5.0)

    ImGui.Separator()
    ImGui.Text("W/S: Forward / Backward")
    ImGui.Text("A/D: Strafe Left / Right")
    ImGui.Text("Space / Left Ctrl: Up / Down")

    tracy.ZoneEnd()
end
