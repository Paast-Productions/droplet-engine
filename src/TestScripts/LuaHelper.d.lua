---@type TestNode
self = nil

function OnStart() end

---@param dt number
function OnUpdate(dt) end

---@class TestNode
TestNode = {}

---@param x number
---@param y number
---@param z number
function TestNode:Set_position(x, y, z)
end

---@return number
function TestNode:Get_x()
    return 0
end

---@return number
function TestNode:Get_y()
    return 0
end

---@return number
function TestNode:Get_z()
    return 0
end

