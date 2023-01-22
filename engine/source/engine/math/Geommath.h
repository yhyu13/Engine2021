#pragma once
#include "MathUtil.h"

//#define GLM_FORCE_MESSAGES
#define GLM_FORCE_CXX2A
#define GLM_FORCE_CTOR_INIT
#define GLM_FORCE_AVX2
#define GLM_FORCE_INLINE
#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/packing.hpp>

namespace longmarch
{
// yuhang : only glm struct with aligned format can enable SIMD optimization
#define DEFAULT_FP glm::aligned_lowp
#define DEFAULT_DP glm::aligned_lowp

    typedef glm::vec<3, glm::u32, glm::packed_lowp> Vec3u;
    typedef glm::vec<2, glm::u32, glm::packed_lowp> Vec2u;

    typedef glm::vec<2, glm::i32, glm::packed_lowp> Vec2i;
    
    typedef glm::vec<2, glm::f32, DEFAULT_FP> Vec2f;
    typedef glm::vec<3, glm::f32, DEFAULT_FP> Vec3f;
    typedef glm::vec<4, glm::f32, DEFAULT_FP> Vec4f;
    typedef glm::vec<3, glm::f64, DEFAULT_DP> Vec3f64;
    typedef glm::vec<4, glm::f64, DEFAULT_DP> Vec4f64;

    typedef glm::vec<4, glm::u16, glm::packed_highp> Vec4hf_pack;
    typedef glm::vec<2, glm::u16, glm::packed_highp> Vec2hf_pack;
    
    typedef glm::qua<glm::f32, DEFAULT_FP> Quaternion;
    typedef glm::qua<glm::f64, DEFAULT_DP> Quaternion64;
    
    typedef glm::mat<3, 3, glm::f32, DEFAULT_FP> Mat3;
    typedef glm::mat<4, 4, glm::f32, DEFAULT_FP> Mat4;
    typedef glm::mat<3, 3, glm::f64, DEFAULT_DP> Mat3f64;
    typedef glm::mat<4, 4, glm::f64, DEFAULT_DP> Mat4f64;

#undef DEFAULT_FP
#undef DEFAULT_DP

    /*
    *   The ViewFrustum is in view space (opengl coordinate)
        Plane equation is:
        a x + b y + c z + d = 0
        The order is:
         near, far, left, right, bottom , top,
    */
    struct ViewFrustum
    {
        enum class Plane
        {
            NEAR_ = 0,
            FAR_,
            LEFT,
            RIGHT,
            BOTTOM,
            TOP,
        };

        Vec4f& operator[](int i) { return planes[i]; }
        const Vec4f& operator[](int i) const { return planes[i]; }
        Vec4f planes[6];
    };

    /*
        The order is:
        nearplane button-right, top-right, top-left, buttom-left
        farplane button-right, top-right, top-left, buttom-left
        The ViewFrustum corner could be in either in clip space, view space, or in world space
    */
    struct ViewFrustumCorners
    {
        Vec3f corners[8];

        ViewFrustumCorners()
        {
            constexpr float delta = 0.000;
            corners[0] = Vec3f(1.f, -1.f, 1.f - delta);
            corners[1] = Vec3f(1.f, 1.f, 1.f - delta);
            corners[2] = Vec3f(-1.f, 1.f, 1.f - delta);
            corners[3] = Vec3f(-1.f, -1.f, 1.f - delta);
            corners[4] = Vec3f(1.f, -1.f, delta);
            corners[5] = Vec3f(1.f, 1.f, delta);
            corners[6] = Vec3f(-1.f, 1.f, delta);
            corners[7] = Vec3f(-1.f, -1.f, delta);
        }

        Vec3f& operator[](int i) { return corners[i]; }
        const Vec3f& operator[](int i) const { return corners[i]; }
    };

    class Geommath
    {
    public:
        inline static Vec3f WorldRight = {Vec3f(1, 0, 0)};
        inline static Vec3f WorldFront = {Vec3f(0, 1, 0)};
        inline static Vec3f WorldUp = {Vec3f(0, 0, 1)};
        inline static Quaternion UnitQuat = {Quaternion(1, 0, 0, 0)};
        /*
            Triplet of trans, rot, and scale
        */
        struct TRS
        {
            explicit TRS(const Vec3f& _trans, const Quaternion& _rot, const Vec3f& _scale)
                :
                trans(_trans),
                rot(_rot),
                scale(_scale)
            {
            }

            inline Mat4 ToTransform(bool apply_trans, bool apply_rot, bool apply_scale)
            {
                auto _pos = (apply_trans) ? trans : Vec3f(0.0f);
                auto _rot = (apply_rot) ? rot : Geommath::UnitQuat;
                auto _scale = (apply_scale) ? scale : Vec3f(1.0f);
                return Geommath::ToTransformMatrix(_pos, _rot, _scale);
            }

            Vec3f trans = {Vec3f(0.0f)};
            Quaternion rot = {Geommath::UnitQuat};
            Vec3f scale = {Vec3f(0.0f)};
        };

        using VQS = TRS;

        enum ROT_AXIS
        {
            PITCH = 0,
            ROLL = 1,
            YAW = 2,
            X = 0,
            Y = 1,
            Z = 2,
        };

        static Mat4 World2OpenGLTr;
        static Mat4 OpenGL2WorldTr;
        static Mat4 FlipX;
        static Mat4 FlipY;
        static Mat4 FlipZ;
        static Mat4 ReverseZTr;
        static Mat4 ShadowBiasTr;
        static Mat4 GLTF2WorldTr;
        static Mat4 WorldToGLTFTr;
    public:
        static bool IsNaN(const Vec3f& v);
        static bool IsNaN(const Vec4f& v);
        static bool IsNaN(const Mat3& v);
        static bool IsNaN(const Mat4& v);
        static bool IsInf(const Vec3f& v);
        static bool IsInf(const Vec4f& v);
        static bool IsInf(const Mat3& v);
        static bool IsInf(const Mat4& v);

        template <typename T>
        static auto Distance(const T& v, const T& u)
        {
            return glm::distance(v, u);
        }

        template <typename T>
        static auto DistanceSquare(const T& v, const T& u)
        {
            return glm::distance2(v, u);
        }

        template <typename T>
        static auto Length(const T& v)
        {
            return glm::length(v);
        }

        template <typename T>
        static auto LengthSquare(const T& v)
        {
            return glm::length2(v);
        }

        template <typename T>
        static auto Clamp(const T& v, const T& lower, const T& higher)
        {
            return glm::clamp(v, lower, higher);
        }

        template <typename T>
        static auto Lerp(const T& v1, const T& v2, float dt)
        {
            dt = glm::clamp(dt, 0.0f, 1.0f);
            return (1.0f - dt) * v1 + dt * v2;
        }

        static Vec3f Normalize(const Vec3f& v);

        static Vec3f ToRad(const Vec3f& v);

        static Vec3f ToDegree(const Vec3f& v);

        static Mat4 RotationMat(ROT_AXIS axis, float angle_rad);

        static float SignedAngle(const Vec3f& u, const Vec3f& v, const Vec3f& axis);

        static Mat4 ToMat4(const Quaternion& q);

        static Quaternion Inverse(const Quaternion& q);

        static Quaternion Conjugate(const Quaternion& q);

        static Quaternion Slerp(const Quaternion& q1, const Quaternion& q2, float dt);

        static Quaternion QuatProd(const Quaternion& q1, const Quaternion& q2);

        static Quaternion FromAxisRot(float angle, const Vec3f& axis);

        static Quaternion FromVectorPair(const Vec3f& from, const Vec3f& to);

        static Quaternion ToQTangent(const Vec3f& N, const Vec3f& T, const Vec3f& B);

        static Quaternion ToQuaternion(const Vec3f& angles);

        static Quaternion ApplyGLTF2World(const Quaternion& q);

        static Quaternion ApplyGLTF2WorldInv(const Quaternion& q);

        static Vec3f ToEulerAngles(const Quaternion& q);

        static Vec4f ToVec4(const Quaternion& quat);

        static bool NearlyEqual(double a, double b, double ep = 1e-8);

        static bool NearlyEqual(Vec4f a, Vec4f b, double ep = 1e-8);

        static bool NearlyEqual(Vec3f a, Vec3f b, double ep = 1e-8);

        static bool NearlyEqual(Vec2f a, Vec2f b, double ep = 1e-8);

        static bool isOrthogonal(const Mat3& mat);

        static bool isOrthogonal(const Mat4& mat);

        static Mat4 Transpose(const Mat4& m);

        static Mat4 SmartInverseTranspose(const Mat4& m);

        static Mat4 SmartInverse(const Mat4& m);

        static Mat4 HighPrecisionInverse(const Mat4& m);

        static Vec3f CartesianToSpherical(const Vec3f& v);

        static Vec3f SphericalToCartesian(const Vec3f& v);

        static Vec4hf_pack PackVec4ToHVec4(const Vec4f& v);

        static Vec2hf_pack PackVec2ToHVec2(const Vec2f& v);

        static Vec4f UnPackVec4ToHVec4(const Vec4hf_pack& v);

        static Vec2f UnPackVec2ToHVec2(const Vec2hf_pack& v);

        static Vec3f ToVec3(const Vec4f& v);

        static Vec4f ToVec4(const Vec3f& v);

        static Vec2f ToVec2(const Vec3f& v);

        static Vec3f Mat4ProdVec3(const Mat4& mat, const Vec3f& v);

        static Mat4 ToTransformMatrix(const Vec3f& pos, const Quaternion& quat, const Vec3f& scale);

        static Mat4 ToTranslateMatrix(const Vec3f& v);

        static Mat4 ToScaleMatrix(const Vec3f& v);

        static Vec3f GetTranslation(const Mat4& m);

        static Quaternion GetRotation(const Mat4& m);

        static Vec3f GetScale(const Mat4& m);

        static void SetTranslation(Mat4& m, const Vec3f& v);

        static void SetRotation(Mat4& m, const Quaternion& q);

        static void SetScale(Mat4& m, const Vec3f& s);

        static Vec3f GetTranslationParentSpace(const Mat4& m, const Mat4& parent);

        static void SetTranslationParentSpace(Mat4& m, const Vec3f& rtp_position, const Mat4& parent);

        static Quaternion GetRotationParentSpace(const Mat4& m, const Mat4& parent);

        static void SetRotationParentSpace(Mat4& m, const Quaternion& rtp_rotation, const Mat4& parent);

        static Mat4 ViewMatrix(const Vec3f& translation, const Quaternion& rotation);

        //! return a OpenGL view coordinate lookat matrix based on World coordinate system, output a mocking lookat matrix when (src-dest) is close to world's up vector
        static Mat4 LookAtWorld(const Vec3f& src, const Vec3f& dest, const Vec3f& tranlstion = Vec3f(0));

        //! return a OpenGL view coordinate lookat matrix based on World coordinate system
        static Mat4 LookAt(const Vec3f& src, const Vec3f& dest, const Vec3f& up, const Vec3f& tranlstion = Vec3f(0));

        static Mat4 ProjectionMatrixNegativeOneOne(double fovY_radians, double aspectWbyH, double zNear, double zFar,
                                                   bool inf = false);

        static Mat4 ProjectionMatrixZeroOne(double fovY_radians, double aspectWbyH, double zNear, double zFar,
                                            bool inf = false);

        static Mat4 ReverseZProjectionMatrixZeroOne(double fovY_radians, double aspectWbyH, double zNear, double zFar,
                                                    bool inf = false);

        static Mat4 OrthogonalProjectionMatrixZeroOne(double left, double right, double buttom, double top,
                                                      double zNear, double zFar);

        static Mat4 OrthogonalReverseZProjectionMatrixZeroOne(double left, double right, double buttom, double top,
                                                              double zNear, double zFar);

        struct Frustum
        {
            static ViewFrustum FromProjection(const Mat4& m);

            static void GetCornersAndCentroid(const Mat4& pv, bool reverse_z, ViewFrustumCorners& out_world_ndc_corners,
                                              Vec3f& out_world_center);
        };

        struct Plane
        {
            static Vec4f FromNP(const Vec3f& N, const Vec3f& p);

            static Vec4f Normalize(const Vec4f& p);
            static Vec4f NormalizeSlow(const Vec4f& p);

            static float Distance(const Vec4f& plane, const Vec3f& point);

            static float DotNormal(const Vec4f& plane, const Vec3f& point);

            static Vec3f ProjectedPointOnPlane(const Vec4f& plane, const Vec3f& point);
        };
    };
}

std::ostream& operator<<(std::ostream& o, const Mat4& n);
std::ostream& operator<<(std::ostream& o, const glm::mat3& n);
std::ostream& operator<<(std::ostream& o, const glm::vec4& n);
std::ostream& operator<<(std::ostream& o, const Vec3f& n);
std::ostream& operator<<(std::ostream& o, const glm::vec2& n);
std::ostream& operator<<(std::ostream& o, const glm::quat& n);
