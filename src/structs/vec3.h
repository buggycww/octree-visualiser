#pragma once

#include <raylib.h>

namespace vec3
{
	struct Vector3
	{
		float x;
		float y;
		float z;

		Vector3() = default;
		Vector3(float n) : x(n), y(n), z(n) {};
		Vector3(float x, float y, float z) : x(x), y(y), z(z) {};

		Vector3& operator+=(const Vector3& vector)
		{
			this->x += vector.x;
			this->y += vector.y;
			this->z += vector.z;

			return *this;
		}

		Vector3& operator-=(const Vector3& vector)
		{
			this->x -= vector.x;
			this->y -= vector.y;
			this->z -= vector.z;

			return *this;
		}

		Vector3& operator*=(float multiplier)
		{
			this->x *= multiplier;
			this->y *= multiplier;
			this->z *= multiplier;

			return *this;
		}

		// opearator keyword can also be used for conversion
		// operator followed by (+, -, =, []) etc is for overloading
		// but if followed by a type, e.g. (float, int, ::Vector3), then it's defining a conversion operator
		// implicit conversion to raylib's global ::Vector3
		operator ::Vector3() const
		{
			return { this->x, this->y, this->z };
		}
	};

	// free function so that implicit conversion would make it work with 2.f + someVector3 instead of only the other way around
	// return copy instead of address because + operator doesn't modify original variables
	inline Vector3 operator+(Vector3 lhs, const Vector3& rhs)
	{
		lhs += rhs;
		return lhs;
	}

	inline Vector3 operator-(Vector3 lhs, const Vector3& rhs)
	{
		lhs -= rhs;
		return lhs;
	}

	inline Vector3 operator*(Vector3 lhs, const float rhs)
	{
		lhs *= rhs;
		return lhs;
	}
}