---@type Node
self = nil

function OnStart() end

---@param dt number
function OnUpdate(dt) end

---@enum TransformSpace
TransformSpace = {
	Local = 0,
	World = 1
}

---@enum ImGuiWindowFlags
ImGuiWindowFlags = {
	None = 0,
	NoTitleBar = 1,
	NoResize = 2,
	NoMove = 4,
	NoScrollbar = 8,
	NoScrollWithMouse = 16,
	NoCollapse = 32,
	AlwaysAutoResize = 64,
	NoBackground = 128,
	NoSavedSettings = 256,
	NoMouseInputs = 512,
	MenuBar = 1024,
	HorizontalScrollbar = 2048,
	NoFocusOnAppearing = 4096,
	NoBringToFrontOnFocus = 8192,
	AlwaysVerticalScrollbar = 16384,
	AlwaysHorizontalScrollbar = 32768,
	NoNavInputs = 65536,
	NoNavFocus = 131072,
	UnsavedDocument = 262144,
	NoDocking = 524288,
	NoNav = 196608,
	NoDecoration = 43,
	NoInputs = 197120
}

---@enum ImGuiChildFlags
ImGuiChildFlags = {
	None = 0,
	Borders = 1,
	AlwaysUseWindowPadding = 2,
	ResizeX = 4,
	ResizeY = 8,
	AutoResizeX = 16,
	AutoResizeY = 32,
	AlwaysAutoResize = 64,
	FrameStyle = 128,
	NavFlattened = 256
}

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

---@param space TransformSpace|nil
---@return Vec3
function Transform:GetPosition(space)
    return nil
end

---@param space TransformSpace|nil
---@return Quat
function Transform:GetRotation(space)
    return nil
end

---@param space TransformSpace|nil
---@return Vec3
function Transform:GetEuler(space)
    return nil
end

---@return Vec3
function Transform:GetScale()
    return nil
end

---@param space TransformSpace|nil
---@return Mat4
function Transform:GetMatrix(space)
    return nil
end

---@return boolean
function Transform:IsDirty()
    return false
end

---@param space TransformSpace|nil
---@return Vec3
function Transform:GetUp(space)
    return nil
end

---@param space TransformSpace|nil
---@return Vec3
function Transform:GetRight(space)
    return nil
end

---@param space TransformSpace|nil
---@return Vec3
function Transform:GetForward(space)
    return nil
end

---@param position Vec3
---@param space TransformSpace|nil
function Transform:SetPosition(position, space)
end

---@param rotation Quat
---@param space TransformSpace|nil
function Transform:SetRotation(rotation, space)
end

---@param euler Vec3
---@param space TransformSpace|nil
function Transform:SetEuler(euler, space)
end

---@param scale Vec3
function Transform:SetScale(scale)
end

---@param matrix Mat4
---@param space TransformSpace|nil
function Transform:SetMatrix(matrix, space)
end

function Transform:MakeDirty()
end

function Transform:RecalculateMatrices()
end

---@param amount Vec3
---@param space TransformSpace|nil
function Transform:Move(amount, space)
end

---@param rotation Quat
---@param space TransformSpace|nil
function Transform:Rotate(rotation, space)
end

---@param amount Vec3
function Transform:MoveRelative(amount)
end

---@param euler Vec3
function Transform:RotateEulerRelative(euler)
end

---@param euler Vec3
---@param space TransformSpace|nil
function Transform:RotateEuler(euler, space)
end

---@param scale Vec3
function Transform:AddScale(scale)
end

---@param angle number
---@param axis Vec3
---@param space TransformSpace|nil
function Transform:RotateAxis(angle, axis, space)
end

---@param target Vec3
---@param up Vec3|nil
---@param space TransformSpace|nil
function Transform:LookAt(target, up, space)
end

---@class Vec2
---@field x number
---@field y number
Vec2 = {}

---@class Vec3
---@field x number
---@field y number
---@field z number
Vec3 = {}

---@class Vec4
---@field x number
---@field y number
---@field z number
---@field w number
Vec4 = {}

---@class Quat
---@field w number
---@field x number
---@field y number
---@field z number
Quat = {}

---@class Mat3
Mat3 = {}

---@class Mat4
Mat4 = {}

---@class GLM
GLM = {}

---@param vec Vec2
---@return Vec2
function GLM.V2Abs(vec)
    return nil
end

---@param vec Vec2
---@return Vec2
function GLM.V2Sign(vec)
    return nil
end

---@param vec Vec2
---@return Vec2
function GLM.V2Floor(vec)
    return nil
end

---@param vec Vec2
---@return Vec2
function GLM.V2Trunc(vec)
    return nil
end

---@param vec Vec2
---@return Vec2
function GLM.V2Round(vec)
    return nil
end

---@param vec Vec2
---@return Vec2
function GLM.V2RoundEven(vec)
    return nil
end

---@param vec Vec2
---@return Vec2
function GLM.V2Ceil(vec)
    return nil
end

---@param vec Vec2
---@return Vec2
function GLM.V2Fract(vec)
    return nil
end

---@param vec1 Vec2
---@param vec2 Vec2
---@return Vec2
function GLM.V2Mod(vec1, vec2)
    return nil
end

---@param vec Vec2
---@param float number
---@return Vec2
function GLM.V2ModF(vec, float)
    return nil
end

---@param vec1 Vec2
---@param vec2 Vec2
---@return Vec2
function GLM.V2Min(vec1, vec2)
    return nil
end

---@param vec1 Vec2
---@param vec2 Vec2
---@return Vec2
function GLM.V2Max(vec1, vec2)
    return nil
end

---@param vec Vec2
---@param min Vec2
---@param max Vec2
---@return Vec2
function GLM.V2Clamp(vec, min, max)
    return nil
end

---@param vec Vec2
---@param float number
---@return Vec2
function GLM.V2AddF(vec, float)
    return nil
end

---@param vec Vec2
---@param float number
---@return Vec2
function GLM.V2SubF(vec, float)
    return nil
end

---@param vec Vec2
---@param float number
---@return Vec2
function GLM.V2MulF(vec, float)
    return nil
end

---@param vec Vec2
---@param float number
---@return Vec2
function GLM.V2DivF(vec, float)
    return nil
end

---@param vec1 Vec2
---@param vec2 Vec2
---@return Vec2
function GLM.V2Add(vec1, vec2)
    return nil
end

---@param vec1 Vec2
---@param vec2 Vec2
---@return Vec2
function GLM.V2Sub(vec1, vec2)
    return nil
end

---@param vec1 Vec2
---@param vec2 Vec2
---@return Vec2
function GLM.V2Mul(vec1, vec2)
    return nil
end

---@param vec1 Vec2
---@param vec2 Vec2
---@return Vec2
function GLM.V2Div(vec1, vec2)
    return nil
end

---@param vec1 Vec2
---@param vec2 Vec2
---@return number
function GLM.V2Dot(vec1, vec2)
    return 0
end

---@param vec Vec2
---@return Vec2
function GLM.V2Normalize(vec)
    return nil
end

---@param vec Vec2
---@return number
function GLM.V2Length(vec)
    return 0
end

---@param vec1 Vec2
---@param vec2 Vec2
---@return number
function GLM.V2Distance(vec1, vec2)
    return 0
end

---@param a Vec2
---@param b Vec2
---@param t number
---@return Vec2
function GLM.V2Lerp(a, b, t)
    return nil
end

---@param vec1 Vec2
---@param vec2 Vec2
---@return Vec2
function GLM.V2Reflect(vec1, vec2)
    return nil
end

---@param vec1 Vec2
---@param vec2 Vec2
---@param float number
---@return Vec2
function GLM.V2Refract(vec1, vec2, float)
    return nil
end

---@param vec Vec3
---@return Vec3
function GLM.V3Abs(vec)
    return nil
end

---@param vec Vec3
---@return Vec3
function GLM.V3Sign(vec)
    return nil
end

---@param vec Vec3
---@return Vec3
function GLM.V3Floor(vec)
    return nil
end

---@param vec Vec3
---@return Vec3
function GLM.V3Trunc(vec)
    return nil
end

---@param vec Vec3
---@return Vec3
function GLM.V3Round(vec)
    return nil
end

---@param vec Vec3
---@return Vec3
function GLM.V3RoundEven(vec)
    return nil
end

---@param vec Vec3
---@return Vec3
function GLM.V3Ceil(vec)
    return nil
end

---@param vec Vec3
---@return Vec3
function GLM.V3Fract(vec)
    return nil
end

---@param vec1 Vec3
---@param vec2 Vec3
---@return Vec3
function GLM.V3Mod(vec1, vec2)
    return nil
end

---@param vec Vec3
---@param float number
---@return Vec3
function GLM.V3ModF(vec, float)
    return nil
end

---@param vec1 Vec3
---@param vec2 Vec3
---@return Vec3
function GLM.V3Min(vec1, vec2)
    return nil
end

---@param vec1 Vec3
---@param vec2 Vec3
---@return Vec3
function GLM.V3Max(vec1, vec2)
    return nil
end

---@param vec Vec3
---@param min Vec3
---@param max Vec3
---@return Vec3
function GLM.V3Clamp(vec, min, max)
    return nil
end

---@param vec Vec3
---@param float number
---@return Vec3
function GLM.V3AddF(vec, float)
    return nil
end

---@param vec Vec3
---@param float number
---@return Vec3
function GLM.V3SubF(vec, float)
    return nil
end

---@param vec Vec3
---@param float number
---@return Vec3
function GLM.V3MulF(vec, float)
    return nil
end

---@param vec Vec3
---@param float number
---@return Vec3
function GLM.V3DivF(vec, float)
    return nil
end

---@param vec1 Vec3
---@param vec2 Vec3
---@return Vec3
function GLM.V3Add(vec1, vec2)
    return nil
end

---@param vec1 Vec3
---@param vec2 Vec3
---@return Vec3
function GLM.V3Sub(vec1, vec2)
    return nil
end

---@param vec1 Vec3
---@param vec2 Vec3
---@return Vec3
function GLM.V3Mul(vec1, vec2)
    return nil
end

---@param vec1 Vec3
---@param vec2 Vec3
---@return Vec3
function GLM.V3Div(vec1, vec2)
    return nil
end

---@param vec1 Vec3
---@param vec2 Vec3
---@return number
function GLM.V3Dot(vec1, vec2)
    return 0
end

---@param vec1 Vec3
---@param vec2 Vec3
---@return Vec3
function GLM.V3Cross(vec1, vec2)
    return nil
end

---@param vec Vec3
---@return Vec3
function GLM.V3Normalize(vec)
    return nil
end

---@param vec Vec3
---@return number
function GLM.V3Length(vec)
    return 0
end

---@param vec1 Vec3
---@param vec2 Vec3
---@return number
function GLM.V3Distance(vec1, vec2)
    return 0
end

---@param a Vec3
---@param b Vec3
---@param t number
---@return Vec3
function GLM.V3Lerp(a, b, t)
    return nil
end

---@param vec1 Vec3
---@param vec2 Vec3
---@return Vec3
function GLM.V3Reflect(vec1, vec2)
    return nil
end

---@param vec1 Vec3
---@param vec2 Vec3
---@param float number
---@return Vec3
function GLM.V3Refract(vec1, vec2, float)
    return nil
end

---@param vec Vec4
---@return Vec4
function GLM.V4Abs(vec)
    return nil
end

---@param vec Vec4
---@return Vec4
function GLM.V4Sign(vec)
    return nil
end

---@param vec Vec4
---@return Vec4
function GLM.V4Floor(vec)
    return nil
end

---@param vec Vec4
---@return Vec4
function GLM.V4Trunc(vec)
    return nil
end

---@param vec Vec4
---@return Vec4
function GLM.V4Round(vec)
    return nil
end

---@param vec Vec4
---@return Vec4
function GLM.V4RoundEven(vec)
    return nil
end

---@param vec Vec4
---@return Vec4
function GLM.V4Ceil(vec)
    return nil
end

---@param vec Vec4
---@return Vec4
function GLM.V4Fract(vec)
    return nil
end

---@param vec1 Vec4
---@param vec2 Vec4
---@return Vec4
function GLM.V4Mod(vec1, vec2)
    return nil
end

---@param vec Vec4
---@param float number
---@return Vec4
function GLM.V4ModF(vec, float)
    return nil
end

---@param vec1 Vec4
---@param vec2 Vec4
---@return Vec4
function GLM.V4Min(vec1, vec2)
    return nil
end

---@param vec1 Vec4
---@param vec2 Vec4
---@return Vec4
function GLM.V4Max(vec1, vec2)
    return nil
end

---@param vec Vec4
---@param min Vec4
---@param max Vec4
---@return Vec4
function GLM.V4Clamp(vec, min, max)
    return nil
end

---@param vec Vec4
---@param float number
---@return Vec4
function GLM.V4AddF(vec, float)
    return nil
end

---@param vec Vec4
---@param float number
---@return Vec4
function GLM.V4SubF(vec, float)
    return nil
end

---@param vec Vec4
---@param float number
---@return Vec4
function GLM.V4MulF(vec, float)
    return nil
end

---@param vec Vec4
---@param float number
---@return Vec4
function GLM.V4DivF(vec, float)
    return nil
end

---@param vec1 Vec4
---@param vec2 Vec4
---@return Vec4
function GLM.V4Add(vec1, vec2)
    return nil
end

---@param vec1 Vec4
---@param vec2 Vec4
---@return Vec4
function GLM.V4Sub(vec1, vec2)
    return nil
end

---@param vec1 Vec4
---@param vec2 Vec4
---@return Vec4
function GLM.V4Mul(vec1, vec2)
    return nil
end

---@param vec1 Vec4
---@param vec2 Vec4
---@return Vec4
function GLM.V4Div(vec1, vec2)
    return nil
end

---@param vec1 Vec4
---@param vec2 Vec4
---@return number
function GLM.V4Dot(vec1, vec2)
    return 0
end

---@param vec1 Vec4
---@param vec2 Vec4
---@return Vec4
function GLM.V4Cross(vec1, vec2)
    return nil
end

---@param vec Vec4
---@return Vec4
function GLM.V4Normalize(vec)
    return nil
end

---@param vec Vec4
---@return number
function GLM.V4Length(vec)
    return 0
end

---@param vec1 Vec4
---@param vec2 Vec4
---@return number
function GLM.V4Distance(vec1, vec2)
    return 0
end

---@param a Vec4
---@param b Vec4
---@param t number
---@return Vec4
function GLM.V4Lerp(a, b, t)
    return nil
end

---@param vec1 Vec4
---@param vec2 Vec4
---@return Vec4
function GLM.V4Reflect(vec1, vec2)
    return nil
end

---@param vec1 Vec4
---@param vec2 Vec4
---@param float number
---@return Vec4
function GLM.V4Refract(vec1, vec2, float)
    return nil
end

---@param quat Quat
---@return number
function GLM.Angle(quat)
    return 0
end

---@param quat Quat
---@return Vec3
function GLM.Axis(quat)
    return nil
end

---@param axis Vec3
---@param angle number
---@return Vec3
function GLM.AngleAxis(axis, angle)
    return nil
end

---@param mat Mat3
---@param float number
---@return Mat3
function GLM.M3MulF(mat, float)
    return nil
end

---@param mat1 Mat3
---@param mat2 Mat3
---@return Mat3
function GLM.M3Mul(mat1, mat2)
    return nil
end

---@param mat Mat3
---@param vec Vec3
---@return Vec3
function GLM.M3MulV(mat, vec)
    return nil
end

---@param mat Mat3
---@return Mat3
function GLM.M3Transpose(mat)
    return nil
end

---@param mat Mat3
---@return Mat3
function GLM.M3Inverse(mat)
    return nil
end

---@param mat Mat3
---@return number
function GLM.M3Determinant(mat)
    return 0
end

---@param mat Mat4
---@param float number
---@return Mat4
function GLM.M4MulF(mat, float)
    return nil
end

---@param mat1 Mat4
---@param mat2 Mat4
---@return Mat4
function GLM.M4Mul(mat1, mat2)
    return nil
end

---@param mat Mat4
---@param vec Vec4
---@return Vec4
function GLM.M4MulV(mat, vec)
    return nil
end

---@param mat Mat4
---@return Mat4
function GLM.M4Transpose(mat)
    return nil
end

---@param mat Mat4
---@return Mat4
function GLM.M4Inverse(mat)
    return nil
end

---@param mat Mat4
---@return number
function GLM.M4Determinant(mat)
    return 0
end

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

