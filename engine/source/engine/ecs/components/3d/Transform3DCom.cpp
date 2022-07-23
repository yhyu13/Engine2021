#include "engine-precompiled-header.h"
#include "Transform3DCom.h"

longmarch::Transform3DCom::Transform3DCom(const EntityDecorator& _this)
	:
	BaseComponent(_this.Volatile().GetWorld()),
	m_this(_this.GetEntity())
{
}

void longmarch::Transform3DCom::Update(double ts)
{
	if (!ts)
	{
		return;
	}
	LOCK_GUARD();
	float dt = static_cast<float>(ts);
	{
		g_total_velocity = (Geommath::GetTranslation(parentTr) - Geommath::GetTranslation(prev_parentTr)) / (dt);
		g_rotational_velocity = (Geommath::ToEulerAngles(Geommath::GetRotation(parentTr)) - Geommath::ToEulerAngles(Geommath::GetRotation(prev_parentTr))) / (dt);
		prev_parentTr = parentTr;
		prev_rtp_pos = rtp_pos;
		rtp_pos += dt * (rtp_velocity + rtp_rotation * l_velocity);
	}
	{
		prev_rtp_rotation = rtp_rotation;
		auto qut_l_rot_v_dt = Geommath::ToQuaternion(dt * l_rotational_velocity);
		rtp_rotation = Geommath::QuatProd(rtp_rotation, qut_l_rot_v_dt);
		auto qut_g_rot_v_dt = Geommath::ToQuaternion(dt * rtp_rotational_velocity);
		rtp_rotation = Geommath::QuatProd(qut_g_rot_v_dt, rtp_rotation);
	}
}

void longmarch::Transform3DCom::SetModelTr(const Mat4& m)
{
	const auto rtp_trans = Geommath::SmartInverse(parentTr) * m;
	rtp_pos = Geommath::GetTranslation(rtp_trans);
	l_scale = Geommath::GetScale(rtp_trans);
	rtp_rotation = Geommath::GetRotation(rtp_trans);
}

Mat4 longmarch::Transform3DCom::GetModelTr() const
{
	LOCK_GUARD();
	return parentTr * Geommath::ToTransformMatrix(rtp_pos, rtp_rotation, l_scale);
}

Mat4 longmarch::Transform3DCom::GetSuccessionModelTr(const Transform3DCom& childCom)  const
{
	return GetSuccessionModelTr(childCom.m_apply_parent_trans,
		childCom.m_apply_parent_rot,
		childCom.m_apply_parent_scale);
}

Mat4 longmarch::Transform3DCom::GetSuccessionModelTr(bool apply_trans, bool apply_rot, bool apply_scale) const
{
	LOCK_GUARD();
	auto _pos = (apply_trans) ? rtp_pos : Vec3f(0.0f);
	auto _rot = (apply_rot) ? rtp_rotation : Geommath::UnitQuat;
	auto _scale = (apply_scale) ? rtp_scale : Vec3f(1.0f);
	return parentTr * Geommath::ToTransformMatrix(_pos, _rot, _scale);
}

Mat4 longmarch::Transform3DCom::GetPrevModelTr() const
{
	LOCK_GUARD();
	return prev_parentTr * Geommath::ToTransformMatrix(prev_rtp_pos, prev_rtp_rotation, l_scale);
}

Mat4 longmarch::Transform3DCom::GetPrevSuccessionModelTr(const Transform3DCom& childCom) const
{
	return GetPrevSuccessionModelTr(childCom.m_apply_parent_trans,
		childCom.m_apply_parent_rot,
		childCom.m_apply_parent_scale);
}

Mat4 longmarch::Transform3DCom::GetPrevSuccessionModelTr(bool apply_trans, bool apply_rot, bool apply_scale) const
{
	LOCK_GUARD();
	auto _pos = (apply_trans) ? prev_rtp_pos : Vec3f(0.0f);
	auto _rot = (apply_rot) ? prev_rtp_rotation : Geommath::UnitQuat;
	auto _scale = (apply_scale) ? rtp_scale : Vec3f(1.0f);
	return prev_parentTr * Geommath::ToTransformMatrix(_pos, _rot, _scale);
}

void longmarch::Transform3DCom::SetParentModelTr(const Mat4& m)
{
	LOCK_GUARD();
	parentTr = m;
}

void longmarch::Transform3DCom::ResetParentModelTr()
{
	LOCK_GUARD();
	parentTr = Mat4(1.0f);
}

void longmarch::Transform3DCom::AddGlobalScale(const Vec3f& v)
{
	LOCK_GUARD();
	const auto& parent_scale = Geommath::GetScale(parentTr);
	rtp_scale = (parent_scale * rtp_scale + v) / parent_scale;
}

void longmarch::Transform3DCom::SetGlobalScale(const Vec3f& v)
{
	LOCK_GUARD();
	rtp_scale = v / Geommath::GetScale(parentTr);
}

Vec3f longmarch::Transform3DCom::GetGlobalScale()
{
	LOCK_GUARD();
	return Geommath::GetScale(parentTr) * rtp_scale;
}

void longmarch::Transform3DCom::AddRelativeToParentScale(const Vec3f& v)
{
	LOCK_GUARD();
	rtp_scale += v;
}

void longmarch::Transform3DCom::SetRelativeToParentScale(const Vec3f& v)
{
	LOCK_GUARD();
	rtp_scale = v;
}

Vec3f longmarch::Transform3DCom::GetRelativeToParentScale()
{
	LOCK_GUARD();
	return rtp_scale;
}

void longmarch::Transform3DCom::AddLocalScale(const Vec3f& v)
{
	LOCK_GUARD();
	l_scale += v;
}

void longmarch::Transform3DCom::SetLocalScale(const Vec3f& v)
{
	LOCK_GUARD();
	l_scale = v;
}

Vec3f longmarch::Transform3DCom::GetLocalScale()
{
	LOCK_GUARD();
	return l_scale;
}

void longmarch::Transform3DCom::AddGlobalPos(const Vec3f& v)
{
	LOCK_GUARD();
	rtp_pos += Geommath::GetRotation(parentTr) * Geommath::GetScale(parentTr) * v;
}

void longmarch::Transform3DCom::SetGlobalPos(const Vec3f& v)
{
	LOCK_GUARD();
	rtp_pos = Geommath::GetRotation(parentTr) * Geommath::GetScale(parentTr) * (v - Geommath::GetTranslation(parentTr));
}

Vec3f longmarch::Transform3DCom::GetGlobalPos()
{
	LOCK_GUARD();
	return Geommath::GetTranslation(parentTr) + Geommath::GetRotation(parentTr) * Geommath::GetScale(parentTr) * rtp_pos;
}

Vec3f longmarch::Transform3DCom::GetPrevGlobalPos()
{
	LOCK_GUARD();
	return Geommath::GetTranslation(prev_parentTr) + Geommath::GetRotation(prev_parentTr) * Geommath::GetScale(parentTr) * prev_rtp_pos;
}

void longmarch::Transform3DCom::AddRelativeToParentPos(const Vec3f& v)
{
	LOCK_GUARD();
	rtp_pos += v;
}

void longmarch::Transform3DCom::SetRelativeToParentPos(const Vec3f& v)
{
	LOCK_GUARD();
	rtp_pos = v;
}

Vec3f longmarch::Transform3DCom::GetRelativeToParentPos()
{
	LOCK_GUARD();
	return rtp_pos;
}

Vec3f longmarch::Transform3DCom::GetPrevRelativeToParentPos()
{
	LOCK_GUARD();
	return prev_rtp_pos;
}

void longmarch::Transform3DCom::AddLocalPos(const Vec3f& v)
{
	LOCK_GUARD();
	rtp_pos += rtp_rotation * v;
}

void longmarch::Transform3DCom::AddGlobalVel(const Vec3f& v)
{
	LOCK_GUARD();
	rtp_velocity += v * Geommath::GetRotation(parentTr);
}

void longmarch::Transform3DCom::SetGlobalVel(const Vec3f& v)
{
	LOCK_GUARD();
	rtp_velocity = (v - g_total_velocity) * Geommath::GetRotation(parentTr);
}

Vec3f longmarch::Transform3DCom::GetGlobalVel()
{
	LOCK_GUARD();
	return g_total_velocity + Geommath::GetRotation(parentTr) * rtp_velocity;
}

void longmarch::Transform3DCom::AddRelativeToParentVel(const Vec3f& v)
{
	rtp_velocity += v;
}

void longmarch::Transform3DCom::SetRelativeToParentVel(const Vec3f& v)
{
	LOCK_GUARD();
	rtp_velocity = v;
}

Vec3f longmarch::Transform3DCom::GetRelativeToParentVel()
{
	LOCK_GUARD();
	return rtp_velocity;
}

void longmarch::Transform3DCom::AddLocalVel(const Vec3f& v)
{
	LOCK_GUARD();
	l_velocity += v;
}

void longmarch::Transform3DCom::SetLocalVel(const Vec3f& v)
{
	LOCK_GUARD();
	l_velocity = v;
}

Vec3f longmarch::Transform3DCom::GetLocalVel()
{
	LOCK_GUARD();
	return l_velocity;
}

void longmarch::Transform3DCom::AddGlobalRot(const Quaternion& r)
{
	LOCK_GUARD();
	const auto& _q = Geommath::QuatProd(r, Geommath::QuatProd(Geommath::GetRotation(parentTr), rtp_rotation));
	rtp_rotation = Geommath::QuatProd(Geommath::Conjugate(Geommath::GetRotation(parentTr)), _q);
}

void longmarch::Transform3DCom::SetGlobalRot(const Quaternion& r)
{
	LOCK_GUARD();
	rtp_rotation = Geommath::QuatProd(Geommath::Conjugate(Geommath::GetRotation(parentTr)), r);
}

Quaternion longmarch::Transform3DCom::GetGlobalRot()
{
	LOCK_GUARD();
	return Geommath::QuatProd(Geommath::GetRotation(parentTr), rtp_rotation);
}

Quaternion longmarch::Transform3DCom::GetPrevGlobalRot()
{
	LOCK_GUARD();
	return Geommath::QuatProd(Geommath::GetRotation(prev_parentTr), prev_rtp_rotation);
}

void longmarch::Transform3DCom::AddRelativeToParentRot(const Quaternion& v)
{
	LOCK_GUARD();
	rtp_rotation = Geommath::QuatProd(v, rtp_rotation);
}

void longmarch::Transform3DCom::SetRelativeToParentRot(const Quaternion& v)
{
	LOCK_GUARD();
	rtp_rotation = v;
}

Quaternion longmarch::Transform3DCom::GetRelativeToParentRot()
{
	LOCK_GUARD();
	return rtp_rotation;
}

void longmarch::Transform3DCom::AddLocalRot(const Quaternion& r)
{
	LOCK_GUARD();
	rtp_rotation = Geommath::QuatProd(rtp_rotation, (r));
}

void longmarch::Transform3DCom::AddLocalRotVel(const Vec3f& r)
{
	LOCK_GUARD();
	l_rotational_velocity += r;
}

void longmarch::Transform3DCom::SetLocalRotVel(const Vec3f& r)
{
	LOCK_GUARD();
	l_rotational_velocity = r;
}

Vec3f longmarch::Transform3DCom::GetLocalRotVel()
{
	LOCK_GUARD();
	return l_rotational_velocity;
}

void longmarch::Transform3DCom::AddGlobalRotVel(const Vec3f& r)
{
	LOCK_GUARD();
	rtp_rotational_velocity += r * Geommath::GetRotation(parentTr);
}

void longmarch::Transform3DCom::SetGlobalRotVel(const Vec3f& r)
{
	LOCK_GUARD();
	rtp_rotational_velocity = (r - g_rotational_velocity) * Geommath::GetRotation(parentTr);
}

Vec3f longmarch::Transform3DCom::GetGlobalRotVel()
{
	LOCK_GUARD();
	return Geommath::GetRotation(parentTr) * rtp_rotational_velocity;
}

void longmarch::Transform3DCom::AddRelativeToParentRotVel(const Vec3f& v)
{
	LOCK_GUARD();
	rtp_rotational_velocity += v;
}

void longmarch::Transform3DCom::SetRelativeToParentRotVel(const Vec3f& v)
{
	LOCK_GUARD();
	rtp_rotational_velocity = v;
}

Vec3f longmarch::Transform3DCom::GetRelativeToParentRotVel()
{
	LOCK_GUARD();
	return rtp_rotational_velocity;
}

void longmarch::Transform3DCom::JsonSerialize(Json::Value& value) const
{
	ENGINE_EXCEPT_IF(value.isNull(), L"Trying to write to a null json value!");
	LOCK_GUARD();
	{
		Json::Value output;
		output["id"] = "Transform3DCom";
		auto& val = output["value"];
		static const auto& _default = Transform3DCom();

		if (l_scale != _default.l_scale)
		{
			val["l_scale"] = LongMarch_ArrayToJsonValue(l_scale, 3);
		}
		if (rtp_scale != _default.rtp_scale)
		{
			val["rtp_scale"] = LongMarch_ArrayToJsonValue(rtp_scale, 3);
		}
		if (rtp_pos != _default.rtp_pos)
		{
			val["rtp_trans"] = LongMarch_ArrayToJsonValue(rtp_pos, 3);
		}
		if (l_velocity != _default.l_velocity)
		{
			val["l_trans_v"] = LongMarch_ArrayToJsonValue(l_velocity, 3);
		}
		if (rtp_velocity != _default.rtp_velocity)
		{
			val["rtp_trans_v"] = LongMarch_ArrayToJsonValue(rtp_velocity, 3);
		}
		if (rtp_rotation != _default.rtp_rotation)
		{
			auto rot = Geommath::ToEulerAngles(rtp_rotation) * RAD2DEG;
			val["rtp_rot"] = LongMarch_ArrayToJsonValue(rot, 3);
		}
		if (l_rotational_velocity != _default.l_rotational_velocity)
		{
			auto rot = l_rotational_velocity * RAD2DEG;
			val["l_rot_v"] = LongMarch_ArrayToJsonValue(rot, 3);
		}
		if (rtp_rotational_velocity != _default.rtp_rotational_velocity)
		{
			auto rot = rtp_rotational_velocity * RAD2DEG;
			val["rtp_rot_v"] = LongMarch_ArrayToJsonValue(rot, 3);
		}
		{
			value.append(std::move(output));
		}
	}
}

void longmarch::Transform3DCom::JsonDeserialize(const Json::Value& value)
{
	if (value.isNull())
	{
		return;
	}
	{
		auto& val = value["l_scale"];
		if (!val.isNull())
		{
			ASSERT(val.size() == 3, "scale must be a vec3!");
			this->SetLocalScale(Vec3f(val[0].asFloat(), val[1].asFloat(), val[2].asFloat()));
		}
	}
	{
		auto& val = value["rtp_scale"];
		if (!val.isNull())
		{
			ASSERT(val.size() == 3, "scale must be a vec3!");
			this->SetRelativeToParentScale(Vec3f(val[0].asFloat(), val[1].asFloat(), val[2].asFloat()));
		}
	}
	{
		auto& val = value["rtp_trans"];
		if (!val.isNull())
		{
			ASSERT(val.size() == 3, "tran must be a vec3!");
			this->SetRelativeToParentPos(Vec3f(val[0].asFloat(), val[1].asFloat(), val[2].asFloat()));
		}
	}
	{
		auto& val = value["l_trans_v"];
		if (!val.isNull())
		{
			ASSERT(val.size() == 3, "l_tran_v must be a vec3!");
			this->SetLocalVel(Vec3f(val[0].asFloat(), val[1].asFloat(), val[2].asFloat()));
		}
	}
	{
		auto& val = value["rtp_trans_v"];
		if (!val.isNull())
		{
			ASSERT(val.size() == 3, "g_tran_v must be a vec3!");
			this->SetRelativeToParentVel(Vec3f(val[0].asFloat(), val[1].asFloat(), val[2].asFloat()));
		}
	}
	{
		auto& val = value["rtp_rot"];
		if (!val.isNull())
		{
			ASSERT(val.size() == 3, "rot must be a vec3!");
			this->SetRelativeToParentRot(Geommath::ToQuaternion(DEG2RAD * (Vec3f(val[0].asFloat(), val[1].asFloat(), val[2].asFloat()))));
		}
	}
	{
		auto& val = value["l_rot_v"];
		if (!val.isNull())
		{
			ASSERT(val.size() == 3, "rot_v must be a vec3!");
			this->SetLocalRotVel((DEG2RAD * (Vec3f(val[0].asFloat(), val[1].asFloat(), val[2].asFloat()))));
		}
	}
	{
		auto& val = value["rtp_rot_v"];
		if (!val.isNull())
		{
			ASSERT(val.size() == 3, "rot_v must be a vec3!");
			this->SetRelativeToParentRotVel((DEG2RAD * (Vec3f(val[0].asFloat(), val[1].asFloat(), val[2].asFloat()))));
		}
	}
}

void longmarch::Transform3DCom::ImGuiRender()
{
	constexpr int yoffset_item = 5;
	constexpr int width_item = 100;

	if (ImGui::TreeNode("Transform"))
	{
#ifdef DEBUG_DRAW
		{
			bool show = debug.showRotation;
			if (ImGui::Checkbox("Rotation Arrow", &show))
			{
				debug.showRotation = show;
			}
		}
		{
			bool show = debug.showVelocity;
			if (ImGui::Checkbox("Velocity Arrow", &show))
			{
				debug.showVelocity = show;
			}
		}
		ImGui::Separator();
		ImGui::Dummy(ImVec2(0, yoffset_item));
#endif
		{
			if (ImGui::TreeNode("Local Pos"))
			{
				auto pos = GetGlobalPos();
				auto pos2 = pos;
				float speed = .25f;
				float range = 1.f;
				{
					ImGui::PushItemWidth(width_item);
					ImGui::DragFloat("Global X", &pos[0], speed, pos[0] - range, pos[0] + range);
					ImGui::DragFloat("Global Y", &pos[1], speed, pos[1] - range, pos[1] + range);
					ImGui::DragFloat("Global Z", &pos[2], speed, pos[2] - range, pos[2] + range);
					ImGui::PopItemWidth();
				}
				if (pos2 != pos)
				{
					AddLocalPos(pos - pos2);
				}
				ImGui::Separator();
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Global Pos"))
			{
				auto pos = GetGlobalPos();
				auto pos2 = pos;
				float speed = .25f;
				float range = 1.f;
				{
					ImGui::PushItemWidth(width_item);
					ImGui::DragFloat("Global X", &pos[0], speed, pos[0] - range, pos[0] + range);
					ImGui::DragFloat("Global Y", &pos[1], speed, pos[1] - range, pos[1] + range);
					ImGui::DragFloat("Global Z", &pos[2], speed, pos[2] - range, pos[2] + range);
					ImGui::PopItemWidth();
				}
				if (pos2 != pos)
				{
					SetGlobalPos(pos);
				}
				ImGui::Separator();
				ImGui::TreePop();
			}
			ImGui::Dummy(ImVec2(0, yoffset_item));
		}
		{
			if (ImGui::TreeNode("Local Rot"))
			{
				auto rot = Geommath::ToEulerAngles(GetGlobalRot()) * RAD2DEG;
				auto rot2 = rot;
				float speed = .25f;
				float range = 2.f;
				{
					ImGui::PushItemWidth(width_item);
					ImGui::DragFloat("Global X (Pitch)", &rot[0], speed, rot[0] - range, rot[0] + range);
					ImGui::DragFloat("Global Y (Roll)", &rot[1], speed, rot[1] - range, rot[1] + range);
					ImGui::DragFloat("Global Z (Yaw)", &rot[2], speed, rot[2] - range, rot[2] + range);
					ImGui::PopItemWidth();
				}
				if (rot2 != rot)
				{
					AddLocalRot(Geommath::ToQuaternion((rot - rot2) * DEG2RAD));
				}
				ImGui::Separator();
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Global Rot"))
			{
				auto rot = Geommath::ToEulerAngles(GetGlobalRot()) * RAD2DEG;
				auto rot2 = rot;
				float speed = .25f;
				float range = 2.f;
				{
					ImGui::PushItemWidth(width_item);
					ImGui::DragFloat("Global X (Pitch)", &rot[0], speed, rot[0] - range, rot[0] + range);
					ImGui::DragFloat("Global Y (Roll)", &rot[1], speed, rot[1] - range, rot[1] + range);
					ImGui::DragFloat("Global Z (Yaw)", &rot[2], speed, rot[2] - range, rot[2] + range);
					ImGui::PopItemWidth();
				}
				if (rot2 != rot)
				{
					AddGlobalRot(Geommath::ToQuaternion((rot - rot2) * DEG2RAD));
				}
				ImGui::Separator();
				ImGui::TreePop();
			}
			ImGui::Dummy(ImVec2(0, yoffset_item));
		}
		{
			if (ImGui::TreeNode("Local Scale"))
			{
				auto scale = GetLocalScale();
				auto scale2 = scale;
				float speed = .25f;
				float range = 1.f;
				{
					ImGui::PushItemWidth(width_item);
					ImGui::DragFloat("Scale X", &scale[0], speed, scale[0] - range, scale[0] + range);
					ImGui::DragFloat("Scale Y", &scale[1], speed, scale[1] - range, scale[1] + range);
					ImGui::DragFloat("Scale Z", &scale[2], speed, scale[2] - range, scale[2] + range);
					ImGui::PopItemWidth();
				}
				if (scale2 != scale)
				{
					SetLocalScale(scale);
				}
				ImGui::Separator();
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("RTP Scale"))
			{
				auto scale = GetRelativeToParentScale();
				auto scale2 = scale;
				float speed = .25f;
				float range = 1.f;
				{
					ImGui::PushItemWidth(width_item);
					ImGui::DragFloat("Scale X", &scale[0], speed, scale[0] - range, scale[0] + range);
					ImGui::DragFloat("Scale Y", &scale[1], speed, scale[1] - range, scale[1] + range);
					ImGui::DragFloat("Scale Z", &scale[2], speed, scale[2] - range, scale[2] + range);
					ImGui::PopItemWidth();
				}
				if (scale2 != scale)
				{
					SetRelativeToParentScale(scale);
				}
				ImGui::Separator();
				ImGui::TreePop();
			}
		}
		ImGui::Separator();
		ImGui::TreePop();
	}
}

void longmarch::Transform3DCom::Copy(BaseComponentInterface* other)
{
	// This Copy function is just an example
	LOCK_GUARD();
	auto com = static_cast<Transform3DCom*>(other);
	prev_parentTr = com->prev_parentTr;
	parentTr = com->parentTr;
	prev_rtp_rotation = com->prev_rtp_rotation;
	rtp_rotation = com->rtp_rotation;
	g_rotational_velocity = com->g_rotational_velocity;
	rtp_rotational_velocity = com->rtp_rotational_velocity;
	l_rotational_velocity = com->l_rotational_velocity;
	prev_rtp_pos = com->prev_rtp_pos;
	rtp_pos = com->rtp_pos;
	l_velocity = com->l_velocity;
	rtp_velocity = com->rtp_velocity;
	g_total_velocity = com->g_total_velocity;
	rtp_scale = com->rtp_scale;
	l_scale = com->l_scale;
	m_apply_parent_trans = com->m_apply_parent_trans;
	m_apply_parent_rot = com->m_apply_parent_rot;
	m_apply_parent_scale = com->m_apply_parent_scale;
}
