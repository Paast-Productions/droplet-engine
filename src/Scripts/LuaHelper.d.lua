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

---@enum IntersectType
IntersectType = {
	None = 0,
	Intersects = 1,
	Contains = 2
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

function Node:Destroy()
end

---@return HealthComponent
function Node:GetHealthComponent()
    return nil
end

---@return PlayerComponent
function Node:GetPlayerComponent()
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

---@param target Quat
---@param t number
---@param space TransformSpace|nil
function Transform:SlerpRotation(target, t, space)
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
---@return Quat
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

---@class tracy
tracy = {}

---@return nil
function tracy.ZoneBegin()
    return nil
end

---@param name string
---@return nil
function tracy.ZoneBeginN(name)
    return nil
end

---@return nil
function tracy.ZoneEnd()
    return nil
end

---@param text string
---@return nil
function tracy.ZoneText(text)
    return nil
end

---@param text string
---@return nil
function tracy.ZoneName(text)
    return nil
end

---@param text string
---@return nil
function tracy.Message(text)
    return nil
end

---@class Ray
---@field pos Vec3
---@field dir Vec3
Ray = {}

---@return number
function Ray:GetLength()
    return 0
end

---@return Vec3
function Ray:GetEndPoint()
    return nil
end

---@param transform Mat4
---@return Ray
function Ray:Transform(transform)
    return nil
end

---@class Plane
---@field normal Vec3
---@field distance number
Plane = {}

---@return Vec3
function Plane:GetOrigin()
    return nil
end

---@param point Vec3
---@return bool
function Plane:IsBeneath(point)
    return nil
end

---@class AABB
---@field center Vec3
---@field extents Vec3
AABB = {}

---@return Vec3
function AABB:GetSize()
    return nil
end

---@return Vec3
function AABB:GetMin()
    return nil
end

---@return Vec3
function AABB:GetMax()
    return nil
end

---@param transform Mat4
---@return AABB
function AABB:Transform(transform)
    return nil
end

---@class Sphere
---@field center Vec3
---@field radius number
Sphere = {}

---@class Frustum
---@field planes table
Frustum = {}

---@param index number
---@return Plane
function Frustum:GetPlane(index)
    return nil
end

---@return Plane
function Frustum:GetLeft()
    return nil
end

---@return Plane
function Frustum:GetRight()
    return nil
end

---@return Plane
function Frustum:GetTop()
    return nil
end

---@return Plane
function Frustum:GetBottom()
    return nil
end

---@return Plane
function Frustum:GetNear()
    return nil
end

---@return Plane
function Frustum:GetFar()
    return nil
end

---@class RayHit
---@field point Vec3
---@field normal Vec3
---@field distance number
---@field didHit bool
RayHit = {}

---@class Intersection
Intersection = {}

---@param ray Ray
---@param plane Plane
---@return RayHit
function Intersection.RaycastPlane(ray, plane)
    return nil
end

---@param ray Ray
---@param aabb AABB
---@return RayHit
function Intersection.RaycastAABB(ray, aabb)
    return nil
end

---@param ray Ray
---@param obb OBB
---@return RayHit
function Intersection.RaycastOBB(ray, obb)
    return nil
end

---@param ray Ray
---@param sphere Sphere
---@return RayHit
function Intersection.RaycastSphere(ray, sphere)
    return nil
end

---@param ray Ray
---@param frustum Frustum
---@return RayHit
function Intersection.RaycastFrustum(ray, frustum)
    return nil
end

---@param aabb AABB
---@param point Vec3
---@return bool
function Intersection.AABBContains(aabb, point)
    return nil
end

---@param obb OBB
---@param point Vec3
---@return bool
function Intersection.OBBContains(obb, point)
    return nil
end

---@param sphere Sphere
---@param point Vec3
---@return bool
function Intersection.SphereContains(sphere, point)
    return nil
end

---@param frustum Frustum
---@param point Vec3
---@return bool
function Intersection.FrustumContains(frustum, point)
    return nil
end

---@param aabb AABB
---@param plane Plane
---@return IntersectType
function Intersection.AABBIntersectsPlane(aabb, plane)
    return nil
end

---@param aabb1 AABB
---@param aabb2 AABB
---@return IntersectType
function Intersection.AABBIntersectsAABB(aabb1, aabb2)
    return nil
end

---@param aabb AABB
---@param obb OBB
---@return IntersectType
function Intersection.AABBIntersectsOBB(aabb, obb)
    return nil
end

---@param aabb AABB
---@param sphere Sphere
---@return IntersectType
function Intersection.AABBIntersectsSphere(aabb, sphere)
    return nil
end

---@param aabb AABB
---@param frustum Frustum
---@return IntersectType
function Intersection.AABBIntersectsFrustum(aabb, frustum)
    return nil
end

---@param obb OBB
---@param plane Plane
---@return IntersectType
function Intersection.OBBIntersectsPlane(obb, plane)
    return nil
end

---@param obb OBB
---@param aabb AABB
---@return IntersectType
function Intersection.OBBIntersectsAABB(obb, aabb)
    return nil
end

---@param obb1 OBB
---@param obb2 OBB
---@return IntersectType
function Intersection.OBBIntersectsOBB(obb1, obb2)
    return nil
end

---@param obb OBB
---@param sphere Sphere
---@return IntersectType
function Intersection.OBBIntersectsSphere(obb, sphere)
    return nil
end

---@param obb OBB
---@param frustum Frustum
---@return IntersectType
function Intersection.OBBIntersectsFrustum(obb, frustum)
    return nil
end

---@param sphere Sphere
---@param plane Plane
---@return IntersectType
function Intersection.SphereIntersectsPlane(sphere, plane)
    return nil
end

---@param sphere Sphere
---@param aabb AABB
---@return IntersectType
function Intersection.SphereIntersectsAABB(sphere, aabb)
    return nil
end

---@param sphere Sphere
---@param obb OBB
---@return IntersectType
function Intersection.SphereIntersectsOBB(sphere, obb)
    return nil
end

---@param sphere1 Sphere
---@param sphere2 Sphere
---@return IntersectType
function Intersection.SphereIntersectsSphere(sphere1, sphere2)
    return nil
end

---@param sphere Sphere
---@param frustum Frustum
---@return IntersectType
function Intersection.SphereIntersectsFrustum(sphere, frustum)
    return nil
end

---@param frustum Frustum
---@param plane Plane
---@return IntersectType
function Intersection.FrustumIntersectsPlane(frustum, plane)
    return nil
end

---@param frustum Frustum
---@param aabb AABB
---@return IntersectType
function Intersection.FrustumIntersectsAABB(frustum, aabb)
    return nil
end

---@param frustum Frustum
---@param obb OBB
---@return IntersectType
function Intersection.FrustumIntersectsOBB(frustum, obb)
    return nil
end

---@param frustum Frustum
---@param sphere Sphere
---@return IntersectType
function Intersection.FrustumIntersectsSphere(frustum, sphere)
    return nil
end

---@param frustum1 Frustum
---@param frustum2 Frustum
---@return IntersectType
function Intersection.FrustumIntersectsFrustum(frustum1, frustum2)
    return nil
end

---@class DebugDraw
DebugDraw = {}

---@param ray Ray
---@param color Vec4|nil
---@param depth bool|nil
function DebugDraw.DrawRay(ray, color, depth)
end

---@param start Vec3
---@param end Vec3
---@param color Vec4|nil
---@param depth bool|nil
function DebugDraw.DrawLine(start, end, color, depth)
end

---@param p1 Vec3
---@param p2 Vec3
---@param p3 Vec3
---@param color Vec4|nil
---@param depth bool|nil
---@param wire bool|nil
function DebugDraw.DrawTri(p1, p2, p3, color, depth, wire)
end

---@param center Vec3
---@param up Vec3
---@param right Vec3
---@param color Vec4|nil
---@param depth bool|nil
---@param wire bool|nil
function DebugDraw.DrawQuad(center, up, right, color, depth, wire)
end

---@param p1 Vec3
---@param d Vec3
---@param p2 Vec3
---@param color Vec4|nil
---@param segments int|nil
---@param depth bool|nil
function DebugDraw.DrawCurveQuadratic(p1, d, p2, color, segments, depth)
end

---@param p1 Vec3
---@param d1 Vec3
---@param d2 Vec3
---@param p2 Vec3
---@param color Vec4|nil
---@param segments int|nil
---@param depth bool|nil
function DebugDraw.DrawCurveCubic(p1, d1, d2, p2, color, segments, depth)
end

---@param aabb AABB
---@param color Vec4|nil
---@param depth bool|nil
---@param wire bool|nil
function DebugDraw.DrawAABB(aabb, color, depth, wire)
end

---@param obb OBB
---@param color Vec4|nil
---@param depth bool|nil
---@param wire bool|nil
function DebugDraw.DrawOBB(obb, color, depth, wire)
end

---@param sphere Sphere
---@param color Vec4|nil
---@param subdivisions int|nil
---@param depth bool|nil
function DebugDraw.DrawSphere(sphere, color, subdivisions, depth)
end

---@param center Vec3
---@param normal Vec3
---@param radius float
---@param color Vec4|nil
---@param segments int|nil
---@param depth bool|nil
---@param wire bool|nil
function DebugDraw.DrawCircle(center, normal, radius, color, segments, depth, wire)
end

---@param frustum Frustum
---@param color Vec4|nil
---@param depth bool|nil
---@param wire bool|nil
function DebugDraw.DrawFrustum(frustum, color, depth, wire)
end

---@param icon number
---@param position Vec3
---@param size Vec2
---@param color Vec4|nil
---@param depth bool|nil
function DebugDraw.DrawIcon(icon, position, size, color, depth)
end

---@class Scene
Scene = {}

---@param ray Ray
---@return tuple<Node, RayHit>
function Scene:RaycastNode(ray)
    return nil
end

---@param ray Ray
---@return Node[]
function Scene:RaycastNodes(ray)
    return nil
end

---@class SceneManager
SceneManager = {}

---@param scenePath string
function SceneManager:LoadScene(scenePath)
end

---@param scenePath string
function SceneManager:UnloadScene(scenePath)
end

---@param scene Scene
function SceneManager:ActivateScene(scene)
end

---@param scene Scene
function SceneManager:DeactivateScene(scene)
end

---@param sceneName string
---@return Scene
function SceneManager:GetScene(sceneName)
    return nil
end

---@return Scene[]
function SceneManager:GetActiveScenes()
    return nil
end

---@class HealthComponent
HealthComponent = {}

---@return float
function HealthComponent:GetHealth()
    return nil
end

---@return float
function HealthComponent:GetMaxHealth()
    return nil
end

---@param amount number
function HealthComponent:TakeDamage(amount)
end

---@param amount number
function HealthComponent:Heal(amount)
end

---@param amount number
function HealthComponent:SetHealth(amount)
end

---@param amount number
function HealthComponent:SetMaxHealth(amount)
end

---@class PlayerComponent
PlayerComponent = {}

---@return PlayerBehaviour
function PlayerComponent:GetPlayerBehaviour()
    return nil
end

---@class PlayerBehaviour
PlayerBehaviour = {}

---@return float
function PlayerBehaviour:GetMaxHealth()
    return nil
end

---@return float
function PlayerBehaviour:GetHeatResistance()
    return nil
end

---@return float
function PlayerBehaviour:GetHeatLevel()
    return nil
end

---@param amount number
function PlayerBehaviour:IncreaseHeatLevel(amount)
end

---@param amount number
---@return HeatResistance
function PlayerBehaviour:SetHeatLevel(amount)
    return nil
end

