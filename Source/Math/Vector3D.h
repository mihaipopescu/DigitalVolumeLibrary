#pragma once

namespace vdb 
{
	namespace math 
	{
		template <typename T>
		class sVector3D
		{
		public:
			sVector3D(T _x=(T)0, T _y=T(0), T _z=(T)0) : x(_x), y(_y), z(_z)
			{
			}

			T operator*(const sVector3D<T>& v) const
			{
				return x * v.x + y * v.y + z * v.z;
			}

			sVector3D<T> operator*(T value) const
			{
				return sVector3D<T>(x * value, y * value, z * value);
			}

			sVector3D<T> operator+(const sVector3D<T>& v) const
			{
				return sVector3D<T>(x + v.x, y + v.y, z + v.z);
			}

			sVector3D<T> operator-(const sVector4D<T>& v) const
			{
				return sVector3D<T>(x + v.x, y + v.y, z + v.z);
			}

			sVector3D<T>& operator-=(const sVector3D<T>& v)
			{
				this->x -= v.x;
				this->y -= v.y;
				this->z -= v.z;
				return *this;
			}

			sVector3D<T>& operator+=(const sVector3D<T>& v)
			{
				this->x += v.x;
				this->y += v.y;
				this->z += v.z;
				return *this;
			}

			T Norm() const
			{
				return sqrt(x*x + y*y + z*z);
			}

			void Normalize()
			{
				*this = *this * T(1/Norm());
			}


		public:
			T x, y, z;
		};

	}
}