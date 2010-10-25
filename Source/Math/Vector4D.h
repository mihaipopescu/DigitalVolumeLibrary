#ifndef _VECTOR4D_H_
#define _VECTOR4D_H_

namespace vdl 
{
	namespace math 
	{
		template <typename T>
		struct sVector4D
		{
			sVector4D(T _x = T(0), T _y = T(0), T _z = T(0), T _w = T(0)) : x(_x), y(_y), z(_z), w(_w)
			{
			}

			sVector4D<T> operator*(const sVector4D<T>& v) const
			{
				return sVector4D<T>(x * v.x, y * v.y, z * v.z, w * v.w);
			}

			sVector4D<T> operator*(T value) const
			{
				return sVector4D<T>(x * value, y * value, z * value, w * value);
			}

			sVector4D<T> operator+(const sVector4D<T>& v) const
			{
				return sVector4D<T>(x + v.x, y + v.y, z + v.z, w + v.w);
			}

			sVector4D<T> operator-(const sVector4D<T>& v) const
			{
				return sVector4D<T>(x - v.x, y - v.y, z - v.z, w - v.w);
			}

			void operator-=(const sVector4D<T>& v)
			{
				this->x -= v.x;
				this->y -= v.y;
				this->z -= v.z;
				this->w -= v.w;
			}

			void operator+=(const sVector4D<T>& v)
			{
				this->x += v.x;
				this->y += v.y;
				this->z += v.z;
				this->w += v.w;
			}
			
			T Norm() const
			{
				return sqrt( x*x + y*y + z*z + w*w );
			}




		public:
			T x, y, z, w;
		};
	}
}


#endif