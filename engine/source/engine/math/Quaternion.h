#pragma once
#include "BlazeCustom.h"
#include "engine/core/EngineCore.h"
#include "engine/math/MathUtil.h"

namespace AAAAgames
{
	namespace BlazeCustom
	{
		/**
		 * @brief Custom Quat class using (w,x,y,z) format
		 *
		 * Reference : http://danceswithcode.net/engineeringnotes/Quats/Quats.html
		 *
		 * @author Hang Yu (hang.yu@digipen.edu | 60001119)
		 */
		template<typename ft = float, bool matrix_major = blaze::columnMajor>
		struct Quat
		{
			using mat3_type = blaze::StaticMatrix<ft, 3UL, 3UL, matrix_major, blaze::aligned, blaze::padded>;
			using mat4_type = blaze::StaticMatrix<ft, 4UL, 4UL, matrix_major, blaze::aligned, blaze::padded>;
			using vec3_type = blaze::StaticVector<ft, 3UL, blaze::columnVector, blaze::aligned, blaze::padded>;
			using vec4_type = blaze::StaticVector<ft, 4UL, blaze::columnVector, blaze::aligned, blaze::padded>;
			using quat_type = vec4_type;
			constexpr inline static quat_type UnitQuat = { 1.0, 0.0, 0.0, 0.0 };

			Quat() = default;
			~Quat() = default;
			Quat(const Quat&) = default;
			Quat& operator=(const Quat&) = default;
			Quat(Quat&&) = default;
			Quat& operator=(Quat&&) = default;

			constexpr Quat(ft w, ft x, ft y, ft z)
				:
				q(quat_type{ w,x,y,z })
			{}
			explicit constexpr Quat(const quat_type& _q)
				:
				q(_q)
			{}
			constexpr Quat(std::initializer_list<ft> list)
				:
				q(std::move(quat_type(list)))
			{}
			inline static Quat FromEuler(const vec3_type& euler)
			{
				auto roll_half = euler[0] * 0.5f;
				auto pitch_half = euler[1] * 0.5f;
				auto yaw_half = euler[2] * 0.5f;
				auto c_roll_half = cos(roll_half);
				auto c_pitch_half = cos(pitch_half);
				auto c_yaw_half = cos(yaw_half);
				auto s_roll_half = sin(roll_half);
				auto s_pitch_half = sin(pitch_half);
				auto s_yaw_half = sin(yaw_half);
				Quat _q({ c_roll_half * c_pitch_half * c_yaw_half + s_roll_half * s_pitch_half * s_yaw_half
					,s_roll_half * c_pitch_half * c_yaw_half - c_roll_half * s_pitch_half * s_yaw_half
					,c_roll_half * s_pitch_half * c_yaw_half + s_roll_half * c_pitch_half * s_yaw_half
					,c_roll_half * c_pitch_half * s_yaw_half - s_roll_half * s_pitch_half * c_yaw_half
					});
				_q.emInverse();
				return _q;
			}
			inline static Quat FromAxisRotation(const vec4_type& rot_axis)
			{
				auto c = cos(rot_axis[0] * 0.5);
				auto s = sin(rot_axis[0] * 0.5);
				Quat q(c, rot_axis[0] * s, rot_axis[1] * s, rot_axis[2] * s);
				q.emInverse();
				return q;
			}
			inline static Quat FromMat3(const mat3_type& m)
			{
				Quat q;
				auto& q0 = q[0];
				auto& q1 = q[1];
				auto& q2 = q[2];
				auto& q3 = q[3];

				ft r11, r22, r33, r12, r13, r21, r23, r31, r32;
				r11 = m(0, 0);
				r22 = m(1, 1);
				r22 = m(2, 2);

				if constexpr (matrix_major)
				{
					r12 = m(1, 0);
					r13 = m(2, 0);
					r21 = m(0, 1);
					r23 = m(2, 1);
					r31 = m(0, 2);
					r32 = m(1, 2);
				}
				else
				{
					r12 = m(0, 1);
					r13 = m(0, 2);
					r21 = m(1, 0);
					r23 = m(1, 2);
					r31 = m(2, 0);
					r32 = m(2, 1);
				}
				vec4_type norm({ 1.0 + m(0, 0) + m(1, 1) + m(2, 2)
					, 1.0 + m(0, 0) - m(1, 1) - m(2, 2)
					, 1.0 - m(0, 0) + m(1, 1) - m(2, 2)
					, 1.0 - m(0, 0) - m(1, 1) + m(2, 2) });
				switch (auto it = std::max_element(norm.begin(), norm.end()); std::distance(norm.begin(), it))
				{
				case 0:
				{
					q0 = sqrt(norm[0] * 0.25);
					q1 = (r32 - r23) * 0.25 * q0;
					q1 = (r13 - r31) * 0.25 * q0;
					q1 = (r21 - r12) * 0.25 * q0;
				}
				break;
				case 1:
				{
					q1 = sqrt(norm[1] * 0.25);
					q0 = (r32 - r23) * 0.25 * q1;
					q2 = (r12 + r21) * 0.25 * q1;
					q3 = (r13 + r31) * 0.25 * q1;
				}
				break;
				case 2:
				{
					q2 = sqrt(norm[2] * 0.25);
					q0 = (r13 - r31) * 0.25 * q2;
					q1 = (r12 + r21) * 0.25 * q2;
					q3 = (r23 + r32) * 0.25 * q2;
				}
				break;
				case 3:
				{
					q3 = sqrt(norm[3] * 0.25);
					q0 = (r21 - r12) * 0.25 * q3;
					q1 = (r13 + r31) * 0.25 * q3;
					q2 = (r23 + r32) * 0.25 * q3;
				}
				break;
				}
				q.emNormalize();
				return q;
			}
			inline static Quat Slerp(const Quat& q1_, const Quat& q2_, ft t)
			{
				Quat q1(q1_); q1.emNormalize();
				Quat q2(q2_); q2.emNormalize();
				auto cosTheta = blaze::dot(*q1, *q2);
				// If cosTheta < 0, the interpolation will take the long way around the sphere.
				// To fix this, one quat must be negated.
				if (cosTheta < 0.0)
				{
					q2.emConjugate();
					cosTheta = -cosTheta;
				}
				// Perform a linear interpolation when cosTheta is close to 1 to avoid side effect of sin(angle) becoming a zero denominator
				if (auto _t = 1.0 - t; cosTheta > (1.0 - std::numeric_limits<double>::epsilon())) [[unlikely]]
				{
					// Linear interpolation
					return Quat(_t * q1[0] + t * q2[0], _t * q1[1] + t * q2[1], _t * q1[2] + t * q2[2], _t * q1[3] + t * q2[3]);
				}
				else [[likely]]
				{
					// Essential Mathematics, page 467
					auto angle = acos(cosTheta);
				return (sin(_t * angle) * q1 + sin(t * angle) * q2) / sin(angle);
				}
			}

			//! In rad
			inline vec3_type ToEuler() const
			{
				auto& q0 = q[0];
				auto& q1 = q[1];
				auto& q2 = q[2];
				auto& q3 = q[3];
				return vec3_type{ atan2(2.0 * (q0 * q1 + q2 * q3), q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3),
				asin(2.0 * (q0 * q2 + q1 * q3)),
				atan2(2.0 * (q0 * q3 + q1 * q2), q0 * q0 + q1 * q1 - q2 * q2 - q3 * q3) };
			}

			//! In rad
			inline vec4_type ToAxisAngle() const
			{
				auto theta = 2.0 * acos(q[0]);
				auto s = sin(theta * 0.5);
				return vec4_type{ theta , q[1] / s , q[2] / s , q[3] / s };
			}

			inline mat3_type ToMat3() const
			{
				auto q0 = q[0];
				auto q1 = q[1];
				auto q2 = q[2];
				auto q3 = q[3];
				if constexpr (matrix_major)
				{
					return mat3_type({
					   {
						   q0 * q0 + q1 * q1 - q2 * q2 - q3 * q3,
						   (ft)2.0 * q1 * q2 + (ft)2.0 * q2 * q3,
						   (ft)2.0 * q1 * q3 - (ft)2.0 * q0 * q2
					   }
					   ,{
						   (ft)2.0 * q1 * q2 - (ft)2.0 * q0 * q3,
						   q0 * q0 - q1 * q1 + q2 * q2 - q3 * q3,
						   (ft)2.0 * q2 * q3 + (ft)2.0 * q0 * q1
					   }
					   ,{
						   (ft)2.0 * q1 * q3 + (ft)2.0 * q0 * q2,
						   (ft)2.0 * q2 * q3 - (ft)2.0 * q0 * q1,
						   q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3
					   }
						});
				}
				else
				{
					return mat3_type({
						   {
							   q0 * q0 + q1 * q1 - q2 * q2 - q3 * q3,
							   (ft)2.0 * q1 * q2 - (ft)2.0 * q0 * q3,
							   (ft)2.0 * q1 * q3 + (ft)2.0 * q0 * q2
						   }
						   ,{
							   (ft)2.0 * q1 * q2 + (ft)2.0 * q2 * q3,
							   q0 * q0 - q1 * q1 + q2 * q2 - q3 * q3,
							   (ft)2.0 * q2 * q3 - (ft)2.0 * q0 * q1
						   }
						   ,{
							   (ft)2.0 * q1 * q3 - (ft)2.0 * q0 * q2,
							   (ft)2.0 * q2 * q3 + (ft)2.0 * q0 * q1,
							   q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3
						   }
						});
				}
			}

			inline mat4_type ToMat4() const
			{
				mat4_type ret;
				auto mat3 = ToMat3();
				for (int i = 0; i < 3; ++i)
				{
					for (int j = 0; j < 3; ++j)
					{
						ret(i, j) = mat3(i, j);
					}
				}
				ret(3, 3) = 1.0;
				return ret;
			}

			//! In place inverse
			inline Quat& emInverse()
			{
				emConjugate();
				return emNormalize();
			}

			//! In place conjugate
			inline Quat& emConjugate()
			{
				q[1] *= -1.0;
				q[2] *= -1.0;
				q[3] *= -1.0;
				return *this;
			}

			//! In place normalize
			inline Quat& emNormalize()
			{
				q / sqrt(blaze::dot(q, q));
				return *this;
			}

			inline Quat Inverse() const
			{
				return Conjugate().Normalize();
			}

			inline Quat Normalize() const
			{
				Quat _q(*this);
				_q.emNormalize();
				return _q;
			}

			inline Quat Conjugate() const
			{
				Quat _q(*this);
				_q.emConjugate();
				return _q;
			}

			inline const ft operator[](int i) const
			{
				return q[i];
			}

			inline ft& operator[](int i)
			{
				return q[i];
			}

			inline operator vec4_type() const
			{
				return q;
			}

			inline quat_type& operator*()
			{
				return q;
			}

			inline const quat_type& operator*() const
			{
				return q;
			}

			inline friend Quat operator*(double a, const Quat& r)
			{
				return r * a;
			}

			inline friend Quat operator/(double a, const Quat& r)
			{
				return r / a;
			}

			inline friend Quat operator*(const Quat& r, ft a)
			{
				Quat ret(r);
				ret.q *= a;
				return ret;
			}

			inline friend Quat operator/(const Quat& r, ft a)
			{
				Quat ret(r);
				ret.q /= a;
				return ret;
			}

			inline friend Quat operator+(const Quat& r, const Quat& s)
			{
				Quat ret(r);
				ret.q += s.q;
				return ret;
			}

			inline friend bool operator==(const Quat& r, const Quat& s)
			{
				return r.q == s.q;
			}

			inline friend Quat operator*(const Quat& r, const Quat& s)
			{
				return Quat({ r[0] * s[0] - r[1] * s[1] - r[2] * s[2] - r[3] * s[3]
				, r[0] * s[1] + r[1] * s[0] - r[2] * s[3] + r[3] * s[2]
				, r[0] * s[2] + r[1] * s[3] + r[2] * s[0] - r[3] * s[1]
				, r[0] * s[3] - r[1] * s[2] + r[2] * s[1] + r[3] * s[0]
					}).Normalize();
			}

			inline friend vec4_type operator*(const vec4_type& v, const Quat& q)
			{
				return q.Inverse() * v;
			}

			inline friend vec4_type operator*(const Quat& q, const vec4_type& v)
			{
				Quat _pesudo_v({ 0, v[0] / v[3],v[1] / v[3] ,v[2] / v[3] });
				auto _q = q * _pesudo_v * q.Conjugate();
				return vec4_type{ _q[1],_q[2],_q[3], 1.0 };
			}

			inline friend vec3_type operator*(const vec3_type& v, const Quat& q)
			{
				return q.Inverse() * v;
			}

			inline friend vec3_type operator*(const Quat& q, const vec3_type& v)
			{
				Quat _pesudo_v({ 0, v[0],v[1] ,v[2] });
				auto _q = q * _pesudo_v * q.Conjugate();
				return vec3_type{ _q[1],_q[2],_q[3] };
			}

		private:
			quat_type q = { UnitQuat };

		public:
			static void _UNIT_TEST_()
			{
				DEBUG_PRINT("Test Blaze Quaternion");

				// Test 1 : Identity quaternion
				vec3_type X{ 1.0, 0.0, 0.0 };
				vec3_type Y{ 0.0, 1.0, 0.0 };
				vec3_type Z{ 0.0, 0.0, 1.0 };
				{
					Quat q;
					auto ret = q * X;
					ASSERT(ret == X, "failed!");
					ASSERT(q * Y == Y, "failed!");
					ASSERT(q * Z == Z, "failed!");

					mat3_type mat{ { 1,0,0 },{ 0,1,0 },{ 0,0,1 } };
					ASSERT(q.ToMat3() == mat, "failed!");
					mat4_type mat2{ { 1,0,0,0 },{ 0,1,0,0 },{ 0,0,1,0 },{ 0,0,0,1 } };
					ASSERT(q.ToMat4() == mat2, "failed!");
				}
				// Test 2 : PI / 2 around X axis
				{
					Quat q{ sin(PI / 4), sin(PI / 4), 0.0, 0.0 };
					ASSERT(q == Quat::FromAxisRotation(vec4_type{ PI / 2, 1.0, 0.0, 0.0 }), "failed!");
					ASSERT(q * X == X, "failed!");
					ASSERT(q * Y == Z, "failed!");
					ASSERT(q * Z == -Y, "failed!");

					mat3_type mat{ { 1,0,0 },{ 0,0,1 },{ 0,-1,0 } };
					ASSERT(q.ToMat3() == mat, "failed!");
					mat4_type mat2{ { 1,0,0,0 },{ 0,0,1,0 },{ 0,-1,0,0 },{ 0,0,0,1 } };
					ASSERT(q.ToMat4() == mat2, "failed!");
				}
				// Test 3 : 2PI / 3 around  (1,1,1)
				{
					Quat q{ 0.5 , 0.5 , 0.5 , 0.5 };
					ASSERT(q == Quat::FromAxisRotation(vec4_type{ 2 * PI / 3, 1.0, 1.0, 1.0 }), "failed!");
					ASSERT(q * X == Y, "failed!");
					ASSERT(q * Y == Z, "failed!");
					ASSERT(q * Z == X, "failed!");

					mat3_type mat{ { 0,1,0 },{ 0,0,1 },{ 1,0,0 } };
					ASSERT(q.ToMat3() == mat, "failed!");
					mat4_type mat2{ { 0,1,0,0 },{ 0,0,1,0 },{ 1,0,0,0 },{ 0,0,0,1 } };
					ASSERT(q.ToMat4() == mat2, "failed!");
				}
				// Test 3 : 4PI / 3 around  (1,1,1)
				{
					Quat q{ -0.5 , 0.5 , 0.5 , 0.5 };
					ASSERT(q == Quat::FromAxisRotation(vec4_type{ 4 * PI / 3, 1.0, 1.0, 1.0 }), "failed!");
					ASSERT(q * X == Z, "failed!");
					ASSERT(q * Y == X, "failed!");
					ASSERT(q * Z == Y, "failed!");

					mat3_type mat{ { 0,0,1 },{ 1,0,0 },{ 0,1,0 } };
					ASSERT(q.ToMat3() == mat, "failed!");
					mat4_type mat2{ { 0,0,1,0 },{ 1,0,0,0 },{ 0,1,0,0 },{ 0,0,0,1 } };
					ASSERT(q.ToMat4() == mat2, "failed!");
				}

				// Test 4 : Slerp
				{
					Quat q1{ 1.0, 0.0, 0.0, 0.0 };
					Quat q2{ 0.0, 1.0, 0.0, 0.0 };
					{
						auto q3 = Quat::Slerp(q1, q2, 0.0);
						ASSERT(q3 == Quat({ 1.0, 0.0, 0.0, 0.0 }), "failed!");
					}
					{
						auto q3 = Quat::Slerp(q1, q2, .2);
						ASSERT(q3 == Quat({ 0.951056516295154, 0.309016994374947, 0, 0 }), "failed!");
					}
					{
						auto q3 = Quat::Slerp(q1, q2, .4);
						ASSERT(q3 == Quat({ 0.809016994374947, 0.587785252292473, 0, 0 }), "failed!");
					}
					{
						auto q3 = Quat::Slerp(q1, q2, .6);
						ASSERT(q3 == Quat({ 0.587785252292473, 0.809016994374947, 0, 0 }), "failed!");
					}
					{
						auto q3 = Quat::Slerp(q1, q2, .8);
						ASSERT(q3 == Quat({ 0.309016994374947, 0.951056516295154, 0, 0 }), "failed!");
					}
					{
						auto q3 = Quat::Slerp(q1, q2, 1.);
						ASSERT(q3 == Quat({ 0.0, 1.0, 0, 0 }), "failed!");
					}
				}
			}
		};
	}
}