---@type TestNode
self = nil

function OnStart() end

---@param dt number
function OnUpdate(dt) end

---@class Transform = {}

---@param x number
---@param y number
---@param z number
function :Set_position(x, y, z)
end

---@return number
function :Get_x()
    return 0
end

---@return number
function :Get_y()
    return 0
end

---@return number
function :Get_z()
    return 0
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

