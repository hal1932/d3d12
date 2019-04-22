#pragma once
#include <utility>
#include <cstring>

class Transform
{
public:
	Transform()
		: matrix_(DirectX::XMMatrixIdentity())
	{ }

	DirectX::XMMATRIX& Matrix() { return matrix_; }
	const DirectX::XMMATRIX& Matrix() const { return matrix_; }

	void SetScaling(float x, float y, float z)
	{
		scale_[0] = x;
		scale_[1] = y;
		scale_[2] = z;
	}

	void SetRotation(float x, float y, float z)
	{
		rotation_[0] = x;
		rotation_[1] = y;
		rotation_[2] = z;
	}

	void SetTranslation(float x, float y, float z)
	{
		translation_[0] = x;
		translation_[1] = y;
		translation_[2] = z;
	}

	void UpdateMatrix()
	{
		const auto& s = scale_;
		const auto& r = rotation_;
		const auto& t = translation_;

		const auto
			sx = (float)sin(r[0]), cx = (float)cos(r[0]),
			sy = (float)sin(r[1]), cy = (float)cos(r[1]),
			sz = (float)sin(r[2]), cz = (float)cos(r[2]);

		// XYZèá
		const auto
			r11 = cy * cz,
			r12 = cy * sz,
			r13 = -sy,
			r21 = sx * sy * cz - cx * sz,
			r22 = sx * sy * sz + cx * cz,
			r23 = sx * cy,
			r31 = cx * sy * cz + sx * sz,
			r32 = cx * sy * sz - sx * cz,
			r33 = cx * cy;

		auto& m = matrix_;

		m.r[0].m128_f32[0] = s[0] * r11;
		m.r[0].m128_f32[1] = s[0] * r12;
		m.r[0].m128_f32[2] = s[0] * r13;
		m.r[0].m128_f32[3] = 0.0f;

		m.r[1].m128_f32[0] = s[1] * r21;
		m.r[1].m128_f32[1] = s[1] * r22;
		m.r[1].m128_f32[2] = s[1] * r23;
		m.r[1].m128_f32[3] = 0.0f;

		m.r[2].m128_f32[0] = s[2] * r31;
		m.r[2].m128_f32[1] = s[2] * r32;
		m.r[2].m128_f32[2] = s[2] * r33;
		m.r[2].m128_f32[3] = 0.0f;

		m.r[3].m128_f32[0] = t[0];
		m.r[3].m128_f32[1] = t[1];
		m.r[3].m128_f32[2] = t[2];
		m.r[3].m128_f32[3] = 1.0f;
	}

	Transform Clone()
	{
		Transform other;

		other.matrix_ = matrix_;
		memcpy(other.scale_, scale_, sizeof(scale_));
		memcpy(other.rotation_, rotation_, sizeof(rotation_));
		memcpy(other.translation_, translation_, sizeof(translation_));

		return std::move(other);
	}

private:
	DirectX::XMMATRIX matrix_;
	float scale_[3] = { 1.0f, 1.0f, 1.0f };
	float rotation_[3] = { 0.0f, 0.0f, 0.0f };
	float translation_[3] = { 0.0f, 0.0f, 0.0f };
};

