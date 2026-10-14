---@type Node
self = nil

function OnStart() end

---@param dt number
function OnUpdate(dt) end

---@class Node
Node = {}

---@return string
function Node:GetName()
    return ""
end

---@return Transform
function Node:GetTransform()
    return nil
end

---@class Transform
Transform = {}

---@return Vec3
function Transform:GetPosition()
    return nil
end

---@return Quat
function Transform:GetRotation()
    return nil
end

---@return Vec3
function Transform:GetEuler()
    return nil
end

---@return Vec3
function Transform:GetScale()
    return nil
end

---@return Mat4
function Transform:GetMatrix()
    return nil
end

---@return boolean
function Transform:IsDirty()
    return false
end

---@return Vec3
function Transform:GetUp()
    return nil
end

---@return Vec3
function Transform:GetRight()
    return nil
end

---@return Vec3
function Transform:GetForward()
    return nil
end

---@param position Vec3
function Transform:SetPosition(position)
end

---@param rotation Quat
function Transform:SetRotation(rotation)
end

---@param euler Vec3
function Transform:SetEuler(euler)
end

---@param scale Vec3
function Transform:SetScale(scale)
end

---@param matrix Mat4
function Transform:SetMatrix(matrix)
end

function Transform:MakeDirty()
end

function Transform:RecalculateMatrices()
end

---@param amount Vec3
function Transform:Move(amount)
end

---@param rotation Quat
function Transform:Rotate(rotation)
end

---@param euler Vec3
function Transform:RotateEuler(euler)
end

---@param scale Vec3
function Transform:AddScale(scale)
end

---@param axis Vec3
---@param angle number
function Transform:RotateAxis(axis, angle)
end

---@param target Vec3
function Transform:LookAt(target)
end

---@class Vec3
---@field x number
---@field y number
---@field z number
Vec3 = {}

---@class Quat
---@field w number
---@field x number
---@field y number
---@field z number
Quat = {}

---@class Mat4
Mat4 = {}

---@class Key
---@field A Key
---@field B Key
---@field C Key
---@field D Key
---@field E Key
---@field F Key
---@field G Key
---@field H Key
---@field I Key
---@field J Key
---@field K Key
---@field L Key
---@field M Key
---@field N Key
---@field O Key
---@field P Key
---@field Q Key
---@field R Key
---@field S Key
---@field T Key
---@field U Key
---@field V Key
---@field W Key
---@field X Key
---@field Y Key
---@field Z Key
---@field Num1 Key
---@field Num2 Key
---@field Num3 Key
---@field Num4 Key
---@field Num5 Key
---@field Num6 Key
---@field Num7 Key
---@field Num8 Key
---@field Num9 Key
---@field Num0 Key
---@field Enter Key
---@field Escape Key
---@field Backspace Key
---@field Tab Key
---@field Space Key
---@field F1 Key
---@field F2 Key
---@field F3 Key
---@field F4 Key
---@field F5 Key
---@field F6 Key
---@field F7 Key
---@field F8 Key
---@field F9 Key
---@field F10 Key
---@field F11 Key
---@field F12 Key
---@field ArrowUp Key
---@field ArrowRight Key
---@field ArrowLeft Key
---@field ArrowDown Key
Key = {}

---@class Mouse
---@field LMB Mouse
---@field RMB Mouse
Mouse = {}

---@class Input
Input = {}

---@param key Key
---@return boolean
function Input:KeyPressed(key)
    return false
end

---@param key Key
---@return boolean
function Input:KeyHeld(key)
    return false
end

---@param key Key
---@return boolean
function Input:KeyReleased(key)
    return false
end

---@param key Key
---@return boolean
function Input:KeyToggle(key)
    return false
end

---@param button Mouse
---@return boolean
function Input:MousePressed(button)
    return false
end

---@param button Mouse
---@return boolean
function Input:MouseHeld(button)
    return false
end

---@param button Mouse
---@return boolean
function Input:MouseReleased(button)
    return false
end

---@return number
function Input:GetCursorX()
    return 0
end

---@return number
function Input:GetCursorY()
    return 0
end

---@return number
function Input:GetDeltaMouseX()
    return 0
end

---@return number
function Input:GetDeltaMouseY()
    return 0
end

