enabled = true
speed = 5.0
clickCount = 0

function OnStart()
	print("Axel smells ew today")
end

function OnUpdate(dt)
	local flaot = Input:GetCursorX()

	if Input:KeyPressed(Key.ArrowUp) then
		print("Space is pressed")
	end

	if Input:MousePressed(Mouse.RMB) then
		print("RMB is pressed")
		print(flaot)
	end

	--local position = Vec3.new(100.0, 1.0, 90.0)
	--transform:SetPosition(position, TransformSpace.Local)
end

function RenderUI()
    ImGui.Text("This text comes from Lua")
    ImGui.Separator()

    local changed

    changed, enabled =
        ImGui.Checkbox("Enabled", enabled)

    changed, speed =
        ImGui.DragFloat("Speed", speed, 0.1, 0.0, 20.0)

    if ImGui.Button("Lua button") then
        clickCount = clickCount + 1
        print("Button clicked:", clickCount)
    end

    ImGui.Text("Click count: " .. tostring(clickCount))
end
