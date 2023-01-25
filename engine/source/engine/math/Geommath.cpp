#include "engine-precompiled-header.h"
#include "Geommath.h"

#include "glm/gtx/fast_square_root.hpp"

namespace longmarch
{
	Mat4 Geommath::World2OpenGLTr = Mat4(
		1, 0, 0, 0,
		0, 0, -1, 0,
		0, 1, 0, 0,
		0, 0, 0, 1
	);
	Mat4 Geommath::OpenGL2WorldTr = Mat4(
		1, 0, 0, 0,
		0, 0, 1, 0,
		0, -1, 0, 0,
		0, 0, 0, 1
	);
	Mat4 Geommath::FlipX = Mat4(
		-1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	);
	Mat4 Geommath::FlipY = Mat4(
		1, 0, 0, 0,
		0, -1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	);
	Mat4 Geommath::FlipZ = Mat4(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, -1, 0,
		0, 0, 0, 1
	);
	Mat4 Geommath::ReverseZTr = Mat4(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, -1, 0,
		0, 0, 1, 1
	);
	Mat4 Geommath::ShadowBiasTr = Mat4(
		0.5, 0.0, 0.0, 0.0,
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 1, 0.0,
		0.5, 0.5, 0, 1.0
	);
	Mat4 Geommath::GLTF2WorldTr = Geommath::RotationMat(Geommath::ROT_AXIS::PITCH, PI * 0.5f);
	Mat4 Geommath::WorldToGLTFTr = Geommath::RotationMat(Geommath::ROT_AXIS::PITCH, -PI * 0.5f);

	bool Geommath::IsNaN(const Vec3f& v)
	{
		return glm::any(glm::isnan(v));
	}
	bool Geommath::IsNaN(const Vec4f& v)
	{
		return glm::any(glm::isnan(v));;
	}
	bool Geommath::IsNaN(const Mat3& v)
	{
		return glm::any(glm::isnan(v[0])) || glm::any(glm::isnan(v[1])) || glm::any(glm::isnan(v[2]));
	}
	bool Geommath::IsNaN(const Mat4& v)
	{
		return glm::any(glm::isnan(v[0])) || glm::any(glm::isnan(v[1])) || glm::any(glm::isnan(v[2])) || glm::any(glm::isnan(v[3]));
	}
	bool Geommath::IsInf(const Vec3f& v)
	{
		return glm::any(glm::isinf(v));
	}
	bool Geommath::IsInf(const Vec4f& v)
	{
		return glm::any(glm::isinf(v));
	}
	bool Geommath::IsInf(const Mat3& v)
	{
		return glm::any(glm::isinf(v[0])) || glm::any(glm::isinf(v[1])) || glm::any(glm::isinf(v[2]));
	}
	bool Geommath::IsInf(const Mat4& v)
	{
		return glm::any(glm::isinf(v[0])) || glm::any(glm::isinf(v[1])) || glm::any(glm::isinf(v[2])) || glm::any(glm::isinf(v[3]));
	}
	Vec3f Geommath::Normalize(const Vec3f& v)
	{
		return glm::normalize(v);
	}
	Vec3f Geommath::ToRad(const Vec3f& v)
	{
		return v * DEG2RAD;
	}
	Vec3f Geommath::ToDegree(const Vec3f& v)
	{
		return v * RAD2DEG;
	}
	Mat4 Geommath::RotationMat(ROT_AXIS axis, float angle_rad)
	{
		Mat4 M(1.0f);
		float c = cosf(angle_rad);
		float s = sinf(angle_rad);
		switch (axis)
		{
		case longmarch::Geommath::ROT_AXIS::PITCH:
			M[1][1] = c;
			M[2][1] = -s;
			M[1][2] = s;
			M[2][2] = c;
			break;
		case longmarch::Geommath::ROT_AXIS::ROLL:
			M[0][0] = c;
			M[2][2] = s;
			M[0][2] = -s;
			M[2][2] = c;
			break;
		case longmarch::Geommath::ROT_AXIS::YAW:
			M[0][0] = c;
			M[1][0] = -s;
			M[0][1] = s;
			M[1][1] = c;
			break;
		}
		return M;
	}
	float Geommath::SignedAngle(const Vec3f& u, const Vec3f& v, const Vec3f& axis)
	{
		auto _u = glm::normalize(Vec3f64(u));
		auto _v = glm::normalize(Vec3f64(v));
		auto _axis = glm::cross(_u, _v);
		auto sign = glm::dot(_axis, Vec3f64(axis));
		if (NearlyEqual(sign, 0)) [[unlikely]]
		{
			return 0;
		}
		else [[likely]]
		{
			auto angle = glm::acos(glm::dot(_u, _v));
			return angle * ((sign > 0) ? 1.0f : -1.0f);
		}
	}
	Mat4 Geommath::ToMat4(const Quaternion& q)
	{
		return glm::toMat4(q);
	}
	Quaternion Geommath::Inverse(const Quaternion& q)
	{
		return (glm::inverse(Quaternion64(q)));
	}
	Quaternion Geommath::Conjugate(const Quaternion& q)
	{
		return (glm::conjugate(Quaternion64(q)));
	}
	Quaternion Geommath::Slerp(const Quaternion& q1, const Quaternion& q2, float dt)
	{
		return glm::normalize(glm::slerp(glm::normalize(q1), glm::normalize(q2), glm::clamp(dt, 0.0f, 1.0f)));
	}
	Quaternion Geommath::QuatProd(const Quaternion& q1, const Quaternion& q2)
	{
		return (Quaternion64(q1) * Quaternion64(q2));
	}
	Quaternion Geommath::FromAxisRot(float angle, const Vec3f& axis)
	{
		return (glm::angleAxis(angle, axis));
	}
	Quaternion Geommath::FromVectorPair(const Vec3f& from, const Vec3f& to)
	{
		return (Quaternion(glm::normalize(from), glm::normalize(to)));
	}
	Quaternion Geommath::ToQTangent(const Vec3f& N, const Vec3f& T, const Vec3f& B)
	{
		// Reference: https://www.yosoygames.com.ar/wp/2018/03/vertex-formats-part-1-compression/
		Quaternion qTangent = glm::normalize(Quaternion(Mat3(N, T, B)));
		//Make sure QTangent is always positive
		if (qTangent.w < 0)
		{
			qTangent = -qTangent;
		}
		////If it's reflected, then make sure .w is negative.
		Vec3f naturalBinormal = glm::cross(T, N);
		if (glm::dot(naturalBinormal, B) <= 0)
		{
			qTangent = -qTangent;
		}
		return qTangent;
	}
	Quaternion Geommath::ToQuaternion(const Vec3f& angles)
	{
		return glm::normalize(Quaternion64((angles)));
	}
	Quaternion Geommath::ApplyGLTF2World(const Quaternion& q)
	{
		return q * Quaternion(Geommath::GLTF2WorldTr);
	}
	Quaternion Geommath::ApplyGLTF2WorldInv(const Quaternion& q)
	{
		return q * Quaternion(Geommath::World2OpenGLTr);
	}
	Vec3f Geommath::ToEulerAngles(const Quaternion& q) {
		return glm::eulerAngles(glm::normalize(q));
	}
	Vec4f Geommath::ToVec4(const Quaternion& quat)
	{
		return Vec4f(quat.x, quat.y, quat.z, quat.w);
	}
	bool Geommath::NearlyEqual(double a, double b, double ep)
	{
		return std::fabs(a - b) < ep;
	}
	bool Geommath::NearlyEqual(Vec4f a, Vec4f b, double ep)
	{
		bool ret = true;
		for (int i = 0; i < 4; ++i)
		{
			ret &= Geommath::NearlyEqual(a[i], b[i], ep);
		}
		return ret;
	}

	bool Geommath::NearlyEqual(Vec3f a, Vec3f b, double ep)
	{
		bool ret = true;
		for (int i = 0; i < 3; ++i)
		{
			ret &= Geommath::NearlyEqual(a[i], b[i], ep);
		}
		return ret;
	}

	bool Geommath::NearlyEqual(Vec2f a, Vec2f b, double ep)
	{
		bool ret = true;
		for (int i = 0; i < 2; ++i)
		{
			ret &= Geommath::NearlyEqual(a[i], b[i], ep);
		}
		return ret;
	}

	bool Geommath::isOrthogonal(const Mat3& mat)
	{
		//!Reference : https://www.geeksforgeeks.org/check-whether-given-matrix-orthogonal-not/
		constexpr int N = 3;
		// Multiply A*A^t
		for (int i = 0; i < N; i++)
		{
			for (int j = 0; j < N; j++)
			{
				double sum = 0;
				for (int k = 0; k < N; k++)
				{
					// Since we are multiplying with
					// transpose of itself. We use
					// a[j][k] instead of a[k][j]
					sum = sum + (static_cast<double>(mat[i][k]) * static_cast<double>(mat[j][k]));
				}
				if (i == j && NearlyEqual(sum, 1.0))
					return false;
				if (i != j && !NearlyEqual(sum, 0.0))
					return false;
			}
		}
		return true;
	}

	bool Geommath::isOrthogonal(const Mat4& mat)
	{
		constexpr int N = 4;
		// Multiply A*A^t
		for (int i = 0; i < N; i++)
		{
			for (int j = 0; j < N; j++)
			{
				double sum = 0;
				for (int k = 0; k < N; k++)
				{
					// Since we are multiplying with
					// transpose of itself. We use
					// a[j][k] instead of a[k][j]
					sum = sum + (static_cast<double>(mat[i][k]) * static_cast<double>(mat[j][k]));
				}
				if (i == j && NearlyEqual(sum, 1.0))
					return false;
				if (i != j && !NearlyEqual(sum, 0.0))
					return false;
			}
		}
		return true;
	}
	Mat4 Geommath::Transpose(const Mat4& m)
	{
		return glm::transpose(m);
	}
	Mat4 Geommath::SmartInverseTranspose(const Mat4& m)
	{
		return (isOrthogonal(m)) ? m : glm::transpose(Geommath::HighPrecisionInverse(m));
	}
	Mat4 Geommath::SmartInverse(const Mat4& m)
	{
		return (isOrthogonal(m)) ? glm::transpose(m) : Geommath::HighPrecisionInverse(m);
	}
	Mat4 Geommath::HighPrecisionInverse(const Mat4& m)
	{
		Mat4f64 _m(m);
		return Mat4(glm::inverse(_m));
	}
	Vec3f Geommath::CartesianToSpherical(const Vec3f& v)
	{
		float x = v.x;
		float y = v.y;
		float z = v.z;
		Vec3f ret;
		ret.x = glm::length(v); // Radius
		ret.y = (x == 0.0f) ? (y >= 0.0f) ? PI * 0.5f : -PI * 0.5f : glm::atan(y, x); // Azimuthal
		ret.z = (z == 0.0f) ? PI * 0.5f : glm::atan(sqrt(x * x + y * y), z); // longitudinal
		return ret;
	}
	Vec3f Geommath::SphericalToCartesian(const Vec3f& v)
	{
		float r = v.x;
		float azi = v.y;
		float longi = v.z;
		Vec3f ret;
		ret.x = r * cos(azi) * sin(longi);
		ret.y = r * sin(azi) * sin(longi);
		ret.z = r * cos(longi);
		return ret;
	}
	Vec4hf_pack Geommath::PackVec4ToHVec4(const Vec4f& v)
	{
		return glm::packHalf(v);
	}
	Vec2hf_pack Geommath::PackVec2ToHVec2(const Vec2f& v)
	{
		return glm::packHalf(v);
	}
	Vec4f Geommath::UnPackVec4ToHVec4(const Vec4hf_pack& v)
	{
		return glm::unpackHalf(v);
	}
	Vec2f Geommath::UnPackVec2ToHVec2(const Vec2hf_pack& v)
	{
		return glm::unpackHalf(v);
	}
	Vec3f Geommath::ToVec3(const Vec4f& v)
	{
		return (NearlyEqual(v.w, 0.0)) ? v.xyz : v.xyz * static_cast<float>(1.0 / static_cast<double>(v.w));
	}
	Vec4f Geommath::ToVec4(const Vec3f& v)
	{
		return Vec4f(v, 1.0f);
	}
	Vec2f Geommath::ToVec2(const Vec3f& v)
	{
		return v.xy;
	}
	Vec3f Geommath::Mat4ProdVec3(const Mat4& mat, const Vec3f& v)
	{
		Mat4f64 _mat(mat);
		Vec4f64 _v(ToVec4(v));
		Vec4f _v2(_mat * _v);
		return ToVec3(_v2);
	}

	void Geommath::FromTransformMatrix(const Mat4& m, Vec3f& pos, Quaternion& quat, Vec3f& scale)
	{
		pos = Geommath::GetTranslation(m);
		scale = Vec3f(glm::length(m[0]), glm::length(m[1]), glm::length(m[2]));
		Mat3 _m3(m[0] * (1.0f / scale[0]), m[1] * (1.0f / scale[1]), m[2] * (1.0f / scale[2]));
		quat = glm::normalize(Quaternion(std::move(_m3)));
	}

	Mat4 Geommath::ToTransformMatrix(const Vec3f& pos, const Quaternion& quat, const Vec3f& scale)
	{
		Mat4 mat = Geommath::ToMat4(quat);
		mat[0] *= scale.x;
		mat[1] *= scale.y;
		mat[2] *= scale.z;
		mat[3] = std::move(ToVec4(pos));
		return mat;
	}
	Mat4 Geommath::ToTranslateMatrix(const Vec3f& v)
	{
		return Mat4(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			v.x, v.y, v.z, 1.0f);
	}
	Mat4 Geommath::ToScaleMatrix(const Vec3f& v)
	{
		return Mat4(
			v.x, 0.0f, 0.0f, 0.0f,
			0.0f, v.y, 0.0f, 0.0f,
			0.0f, 0.0f, v.z, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);
	}
	Vec3f Geommath::GetTranslation(const Mat4& m)
	{
		return ToVec3(m[3]);
	}
	Quaternion Geommath::GetRotation(const Mat4& m)
	{
		// const auto& _scale = GetScale(m);
		// Mat4f64 _m(m);
		// Mat3 _m2(_m[0] / static_cast<double>(_scale[0]), _m[1] / static_cast<double>(_scale[1]), _m[2] / static_cast<double>(_scale[2]));
		// return glm::normalize(Quaternion(_m2));
		Mat3 _m3(glm::normalize(m[0]), glm::normalize(m[1]), glm::normalize(m[2]));
		return glm::normalize(Quaternion(_m3));
	}
	Vec3f Geommath::GetScale(const Mat4& m)
	{
		return Vec3f(glm::length(m[0]), glm::length(m[1]), glm::length(m[2]));
	}
	void Geommath::SetTranslation(Mat4& m, const Vec3f& v)
	{
		m[3] = ToVec4(v);
	}
	void Geommath::SetRotation(Mat4& m, const Quaternion& q)
	{
		auto scale = GetScale(m);
		Mat4 mat = Geommath::ToMat4(q);
		m[0] = mat[0] * scale.x;
		m[1] = mat[1] * scale.y;
		m[2] = mat[2] * scale.z;
	}
	void Geommath::SetScale(Mat4& m, const Vec3f& s)
	{
		m[0] = glm::normalize(m[0]) * s[0];
		m[1] = glm::normalize(m[1]) * s[1];
		m[2] = glm::normalize(m[2]) * s[2];
	}
	Vec3f Geommath::GetTranslationParentSpace(const Mat4& m, const Mat4& parent)
	{
		return Geommath::Inverse(Geommath::GetRotation(parent)) * (Geommath::GetTranslation(m) - Geommath::GetTranslation(parent));
	}
	void Geommath::SetTranslationParentSpace(Mat4& m, const Vec3f& rtp_position, const Mat4& parent)
	{
		Geommath::SetTranslation(m, Geommath::GetRotation(parent) * rtp_position + Geommath::GetTranslation(parent));
	}
	Quaternion Geommath::GetRotationParentSpace(const Mat4& m, const Mat4& parent)
	{
		return Geommath::GetRotation(m) * Geommath::Inverse(Geommath::GetRotation(parent));
	}
	void Geommath::SetRotationParentSpace(Mat4& m, const Quaternion& rtp_rotation, const Mat4& parent)
	{
		Geommath::SetRotation(m, rtp_rotation * Geommath::GetRotation(parent));
	}
	Mat4 Geommath::ViewMatrix(const Vec3f& translation, const Quaternion& rotation)
	{
		auto transform = Geommath::ToTranslateMatrix(translation) * Geommath::ToMat4(rotation);
		return Geommath::World2OpenGLTr * Geommath::SmartInverse(transform);
	}
	Mat4 Geommath::LookAtWorld(const Vec3f& src, const Vec3f& dest, const Vec3f& tranlstion)
	{
		Vec3f up = Geommath::WorldUp;
		Vec3f Front, Right, Up;
		Front = glm::normalize(src - dest);
		if (Geommath::NearlyEqual(up, Front)) [[unlikely]] //!< this is onmi-directional shadowing under gimbal lock
		{
			Right = Geommath::WorldRight;
		}
		else if (Geommath::NearlyEqual(up, -Front)) [[unlikely]] //!< this is onmi-directional shadowing under gimbal lock
		{
			Right = -Geommath::WorldRight;
		}
		else [[likely]]
		{
			Right = glm::normalize(glm::cross(up, Front));
		}
		Up = glm::normalize(glm::cross(Front, Right));

		return Geommath::ToTranslateMatrix(tranlstion) *
			glm::transpose(Mat4(
				Vec4f(Right, -glm::dot(Right, src)),
				Vec4f(Up, -glm::dot(Up, src)),
				Vec4f(Front, -glm::dot(Front, src)),
				Vec4f(0, 0, 0, 1)
			));
	}
	Mat4 Geommath::LookAt(const Vec3f& src, const Vec3f& dest, const Vec3f& up, const Vec3f& tranlstion)
	{
		Vec3f Front, Right, Up;
		Front = glm::normalize(src - dest);
		Right = glm::normalize(glm::cross(up, Front));
		Up = glm::normalize(glm::cross(Front, Right));

		return Geommath::ToTranslateMatrix(tranlstion) *
			glm::transpose(Mat4(
				Vec4f(Right, -glm::dot(Right, src)),
				Vec4f(Up, -glm::dot(Up, src)),
				Vec4f(Front, -glm::dot(Front, src)),
				Vec4f(0, 0, 0, 1)
			));
	}
	Mat4 Geommath::ProjectionMatrixNegativeOneOne(double fovY_radians, double aspectWbyH, double zNear, double zFar, bool inf)
	{
		/*
			Refernece : http://dev.theomader.com/depth-precision/
			This is the projection matrix that maps z in [-zFar,-zNear] value to [1,-1]
		*/
		auto f = 1.0 / tan(fovY_radians * .5);
		if (inf)
		{
			return Mat4(
				f / aspectWbyH, 0.0f, 0.0f, 0.0f,
				0.0f, f, 0.0f, 0.0f,
				0.0f, 0.0f, -1.0f, -1.0f,
				0.0f, 0.0f, -2.0 * zNear, 0.0f);
		}
		else
		{
			return Mat4(
				f / aspectWbyH, 0.0f, 0.0f, 0.0f,
				0.0f, f, 0.0f, 0.0f,
				0.0f, 0.0f, (zFar + zNear) / (zNear - zFar), -1.0f,
				0.0f, 0.0f, 2.0 * zNear * zFar / (zNear - zFar), 0.0f);
		}
	}
	Mat4 Geommath::ProjectionMatrixZeroOne(double fovY_radians, double aspectWbyH, double zNear, double zFar, bool inf)
	{
		/*
			Refernece : http://dev.theomader.com/depth-precision/
			This is the projection matrix that maps z in [-zFar,-zNear] value to [1,0]
		*/
		auto f = 1.0 / tan(fovY_radians * .5);
		if (inf)
		{
			return Mat4(
				f / aspectWbyH, 0.0f, 0.0f, 0.0f,
				0.0f, f, 0.0f, 0.0f,
				0.0f, 0.0f, -1.0f, -1.0f,
				0.0f, 0.0f, -zNear, 0.0f);
		}
		else
		{
			return Mat4(
				f / aspectWbyH, 0.0f, 0.0f, 0.0f,
				0.0f, f, 0.0f, 0.0f,
				0.0f, 0.0f, zFar / (zNear - zFar), -1.0f,
				0.0f, 0.0f, zNear * zFar / (zNear - zFar), 0.0f);
		}
	}
	Mat4 Geommath::ReverseZProjectionMatrixZeroOne(double fovY_radians, double aspectWbyH, double zNear, double zFar, bool inf)
	{
		/*
			Refernece : http://dev.theomader.com/depth-precision/
			This is the reverse z projection matrix that maps z in [-inf,-zNear] value to [0,1]
		*/
		auto f = 1.0 / tan(fovY_radians * .5);
		if (inf)
		{
			return Mat4(
				f / aspectWbyH, 0.0f, 0.0f, 0.0f,
				0.0f, f, 0.0f, 0.0f,
				0.0f, 0.0f, 0.0f, -1.0f,
				0.0f, 0.0f, zNear, 0.0f);
		}
		else
		{
			return Mat4(
				f / aspectWbyH, 0.0f, 0.0f, 0.0f,
				0.0f, f, 0.0f, 0.0f,
				0.0f, 0.0f, zFar / (zFar - zNear) - 1.0f, -1.0f,
				0.0f, 0.0f, zNear * zFar / (zFar - zNear), 0.0f);
		}
	}
	Mat4 Geommath::OrthogonalProjectionMatrixZeroOne(double left, double right, double buttom, double top, double zNear, double zFar)
	{
		return Mat4(
			2.0f / (right - left), 0, 0, 0,
			0, 2.0f / (top - buttom), 0, 0,
			0, 0, -1.0f / (zFar - zNear), 0,
			-(right + left) / (right - left), -(top + buttom) / (top - buttom), -zNear / (zFar - zNear), 1.0f
		);
	}
	Mat4 Geommath::OrthogonalReverseZProjectionMatrixZeroOne(double left, double right, double buttom, double top, double zNear, double zFar)
	{
		/*
			Refernece : http://dev.theomader.com/depth-precision/
			This is the reverse z projection matrix that maps z in [-inf,-zNear] value to [0,1]
		*/
		return Mat4(
			2.0f / (right - left), 0, 0, 0,
			0, 2.0f / (top - buttom), 0, 0,
			0, 0, 1.0f / (zFar - zNear), 0,
			-(right + left) / (right - left), -(top + buttom) / (top - buttom), zNear / (zFar - zNear) + 1.0f, 1.0f
		);
	}
	ViewFrustum Geommath::Frustum::FromProjection(const Mat4& m)
	{
		/*!
			Reference http://web.archive.org/web/20120531231005/http://crazyjoke.free.fr/doc/3D/plane%20extraction.pdf
			Caculate VF in view space
		*/
		ViewFrustum m_frustumPlane;
		auto pm_rowmaj = glm::transpose(m);
		m_frustumPlane.planes[0] = pm_rowmaj[2]; // near // pm_rowmaj[3] + pm_rowmaj[2]; if you are using [-1,1] for z clip-plane instead of [0,1]
		m_frustumPlane.planes[1] = pm_rowmaj[3] - pm_rowmaj[2]; // far
		m_frustumPlane.planes[2] = pm_rowmaj[3] + pm_rowmaj[0]; // left
		m_frustumPlane.planes[3] = pm_rowmaj[3] - pm_rowmaj[0]; // right
		m_frustumPlane.planes[4] = pm_rowmaj[3] + pm_rowmaj[1]; // bottom
		m_frustumPlane.planes[5] = pm_rowmaj[3] - pm_rowmaj[1]; // top
		for (auto& plane : m_frustumPlane.planes)
		{
			plane = Geommath::Plane::Normalize(plane);
		}
		return m_frustumPlane;
	}
	void Geommath::Frustum::GetCornersAndCentroid(const Mat4& pv, bool reverse_z, ViewFrustumCorners& out_world_ndc_corners, Vec3f& out_world_center)
	{
		ViewFrustumCorners clip_NDCCorners;
		if (!reverse_z)
		{
			for (int i = 0; i < 8; ++i)
			{
				clip_NDCCorners[i].z = 1.0f - clip_NDCCorners[i].z;
			}
		}
		out_world_center = std::move(Vec3f(0));
		// Transform NDC VF corners to world coordinate
		const auto& pv_inv = Geommath::SmartInverse(pv);
		for (int i = 0; i < 8; ++i)
		{
			auto world_NDCCorners = Geommath::Mat4ProdVec3(pv_inv, clip_NDCCorners[i]);
			out_world_ndc_corners[i] = world_NDCCorners;
			out_world_center += world_NDCCorners;
		}
		out_world_center *= 0.125f;
	}
	Vec4f Geommath::Plane::FromNP(const Vec3f& N, const Vec3f& p)
	{
		auto n = glm::normalize(Vec3f64(N));
		auto d = -glm::dot(n, Vec3f64(p));
		return Vec4f(n.x, n.y, n.z, d);
	}
	Vec4f Geommath::Plane::Normalize(const Vec4f& p)
	{
		auto _p3 = Vec3f(p);
		return p * glm::fastInverseSqrt(glm::dot(_p3, _p3));
	}
	Vec4f Geommath::Plane::NormalizeSlow(const Vec4f& p)
	{
		auto _p4 = Vec4f64(p);
		auto _p3 = Vec3f64(p);
		return Vec4f(_p4 * glm::inversesqrt(glm::dot(_p3, _p3) + 1e-8));
	}
	float Geommath::Plane::Distance(const Vec4f& plane, const Vec3f& point)
	{
		auto p = Plane::Normalize(plane);
		return glm::dot(Vec3f(p.xyz), point) + p.w;
	}
	float Geommath::Plane::DotNormal(const Vec4f& plane, const Vec3f& point)
	{
		return glm::dot(Vec3f(plane.xyz), point);
	}
	Vec3f Geommath::Plane::ProjectedPointOnPlane(const Vec4f& plane, const Vec3f& point)
	{
		if (NearlyEqual(Plane::Distance(plane, point), 0)) [[unlikely]]
		{
			return point;
		}
		else [[likely]]
		{
			auto n = Vec3f(plane.xyz);
			auto t = -(glm::dot(n, point) + plane.w) / Geommath::LengthSquare(n);
			return point + n * t;
		}
	}
}

std::ostream& operator<<(std::ostream& o, const Mat4& n)
{
	o.precision(3);
	o << '\n';
	o << "[ " << n[0][0] << "," << n[1][0] << "," << n[2][0] << "," << n[3][0] << " \n";
	o << n[0][1] << "," << n[1][1] << "," << n[2][1] << "," << n[3][1] << " \n";
	o << n[0][2] << "," << n[1][2] << "," << n[2][2] << "," << n[3][2] << " \n";
	o << n[0][3] << "," << n[1][3] << "," << n[2][3] << "," << n[3][3] << " ]";
	return o;
}

std::ostream& operator<<(std::ostream& o, const glm::mat3& n)
{
	o.precision(3);
	o << '\n';
	o << "[ " << n[0][0] << "," << n[1][0] << "," << n[2][0] << " \n";
	o << n[0][1] << "," << n[1][1] << "," << n[2][1] << " \n";
	o << n[0][2] << "," << n[1][2] << "," << n[2][2] << " ]";
	return o;
}

std::ostream& operator<<(std::ostream& o, const glm::vec4& n)
{
	o.precision(3);
	o << "(" << n.x << "," << n.y << "," << n.z << "," << n.w << ") ";
	return o;
}

std::ostream& operator<<(std::ostream& o, const Vec3f& n)
{
	o.precision(3);
	o << "(" << n.x << "," << n.y << "," << n.z << ") ";
	return o;
}

std::ostream& operator<<(std::ostream& o, const glm::vec2& n)
{
	o.precision(3);
	o << "(" << n.x << "," << n.y << ") ";
	return o;
}

std::ostream& operator<<(std::ostream& o, const glm::quat& n)
{
	o.precision(3);
	o << "(" << n.x << "," << n.y << "," << n.z << "," << n.w << ") ";
	return o;
}
