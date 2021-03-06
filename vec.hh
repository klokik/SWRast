#pragma once

#include <cmath>


template<class T>
class vec2
{
  public: vec2(const T _x, const T _y):
    x(_x), y(_y)
  {}

  public: constexpr T length() {
    return std::sqrt(x*x + y*y);
  }

  public: T x;
  public: T y;
};

template<class T>
class vec3
{
  public: vec3(const T _x, const T _y, const T _z):
    x(_x), y(_y), z(_z)
  {}

  public: vec3(const vec2<T> _xy, const T _z):
    x(_xy.x), y(_xy.y), z(_z)
  {}

  public: static vec3<T> lerp(const vec3<T> _a, const vec3<T> _b, const float _t)
  {
    assert(_t >= 0 && _t <= 1);

    return ((1-_t)*_a + _t*_b);
  }
 
  public: T x;
  public: T y;
  public: T z;
};

template<class T, class U>
class vec31
{
  public: vec31(const T _x, const T _y, const T _z, const U _w):
    x(_x), y(_y), z(_z), w(_w)
  {}

  public: vec31(const vec2<T> _xy, const T _z, const U _w):
    x(_xy.x), y(_xy.y), z(_z), w(_w)
  {}

  public: vec31(const vec3<T> _xyz, const U _w):
    x(_xyz.x), y(_xyz.y), z(_xyz.z), w(_w)
  {}

  // empty index
  public: vec31(const T _x, const T _y, const T _z):
    x(_x), y(_y), z(_z)
  {}

  // empty index, vector variant
  public: vec31(const vec3<T> _xyz):
    x(_xyz.x), y(_xyz.y), z(_xyz.z)
  {}

  public: vec31(const vec2<T> _xy, const T _z):
    x(_xy.x), y(_xy.y), z(_z)
  {}

  public: vec2<T> xy() const noexcept
  {
    return {this->x, this->y};
  }

  public: vec3<T> xyz() const noexcept
  {
    return {this->x, this->y, this->z};
  }

  public: T x;
  public: T y;
  public: T z;
  public: U w;
};

template<class T>
using vec4 = vec31<T, T>;

using vec2f = vec2<float>;
using vec3f = vec3<float>;
using vec3fi = vec31<float, int>;
using vec4f = vec4<float>;

template<class T>
static vec2<T> operator+(const vec2<T> _a, const vec2<T> _b) {
  return {_a.x+_b.x, _a.y+_b.y};
}

template<class T>
static vec2<T> operator-(const vec2<T> _a, const vec2<T> _b) {
  return {_a.x-_b.x, _a.y-_b.y};
}

template<class T>
static vec2<T> operator*(const vec2<T> _a, const vec2<T> _b) {
  return {_a.x*_b.x, _a.y*_b.y};
}

template<class T>
static vec2<T> operator*(const vec2<T> _a, const T _c) {
  return {_a.x*_c, _a.y*_c};
}

template<class T>
static vec2<T> operator*(const T _c, const vec2<T> _a) {
  return _a*_c;
}

template<class T>
static vec3<T> operator+(const vec3<T> _a, const vec3<T> _b) {
  return {_a.x+_b.x, _a.y+_b.y, _a.z+_b.z};
}

template<class T>
static vec3<T> operator-(const vec3<T> _a, const vec3<T> _b) {
  return {_a.x-_b.x, _a.y-_b.y, _a.z-_b.z};
}

template<class T>
static vec3<T> operator*(const vec3<T> _a, const vec3<T> _b) {
  return {_a.x*_b.x, _a.y*_b.y, _a.z*_b.z};
}

template<class T>
static vec3<T> operator*(const vec3<T> _a, const T _c) {
  return {_a.x*_c, _a.y*_c, _a.z*_c};
}

template<class T>
static vec3<T> operator*(const T _c, const vec3<T> _a) {
  return _a*_c;
}

template<class T, class U>
static vec31<T, U> operator+(const vec31<T, U> _a, const vec31<T, U> _b) {
  return {_a.x+_b.x, _a.y+_b.y, _a.z+_b.z};
}

template<class T, class U>
static vec31<T, U> operator-(const vec31<T, U> _a, const vec31<T, U> _b) {
  return {_a.x-_b.x, _a.y-_b.y, _a.z-_b.z};
}

template<class T, class U>
static vec31<T, U> operator*(const vec31<T, U> _a, const vec31<T, U> _b) {
  return {_a.x*_b.x, _a.y*_b.y, _a.z*_b.z};
}

template<class T, class U>
static vec31<T, U> operator*(const vec31<T, U> _a, const T _c) {
  return {_a.x*_c, _a.y*_c, _a.z*_c};
}

template<class T, class U>
static vec31<T, U> operator*(const T _c, const vec31<T, U> _a) {
  return _a*_c;
}

template<class T>
static vec4<T> operator+(const vec4<T> _a, const vec4<T> _b) {
  return {_a.x+_b.x, _a.y+_b.y, _a.z+_b.z, _a.w+_b.w};
}

template<class T>
static vec4<T> operator-(const vec4<T> _a, const vec4<T> _b) {
  return {_a.x-_b.x, _a.y-_b.y, _a.z-_b.z, _a.w-_b.w};
}

template<class T>
static vec4<T> operator*(const vec4<T> _a, const vec4<T> _b) {
  return {_a.x*_b.x, _a.y*_b.y, _a.z*_b.z, _a.w*_b.w};
}

template<class T>
static vec4<T> operator*(const vec4<T> _a, const T _c) {
  return {_a.x*_c, _a.y*_c, _a.z*_c, _a.w*_c};
}

template<class T>
static vec4<T> operator*(const T _c, const vec4<T> _a) {
  return _a*_c;
}

template<class T>
class Mat3x3
{
  public: Mat3x3(std::array<T, 9> _vals) {
    memcpy(val, &_vals[0], sizeof(T) * 9);
  }

  public: Mat3x3(const vec3<T> _a, const vec3<T> _b, const vec3<T> _c) {
    val[0][0] = _a.x;
    val[0][1] = _b.x;
    val[0][2] = _c.x;
    val[1][0] = _a.y;
    val[1][1] = _b.y;
    val[1][2] = _c.y;
    val[2][0] = _a.z;
    val[2][1] = _b.z;
    val[2][2] = _c.z;
  }

  public: constexpr T det() const
  {
    return (val[0][0]*val[1][1]*val[2][2]+
            val[0][1]*val[1][2]*val[2][0]+
            val[1][0]*val[2][1]*val[0][2])-
           (val[0][2]*val[1][1]*val[2][0]+
            val[1][0]*val[0][1]*val[2][2]+
            val[2][1]*val[1][2]*val[0][0]);
  }

  public: constexpr vec3<T> col(const int _col) const {
    return {val[0][_col], val[1][_col], val[2][_col]};
  }

  public: static vec3<T> linSolve(const Mat3x3<T> &_mat, const vec3<T> _b)
  {
    auto d1 = Mat3x3<T>(_b, _mat.col(1), _mat.col(2)).det();
    auto d2 = Mat3x3<T>(_mat.col(0), _b, _mat.col(2)).det();
    auto d3 = Mat3x3<T>(_mat.col(0), _mat.col(1), _b).det();
    auto D = _mat.det();

    return {d1/D, d2/D, d3/D};
  }

  public: static vec3<T> linSolve(const Mat3x3 &_mat, const vec3<T> _b, int _it);

  T val[3][3];
};

using Mat3f3 = Mat3x3<float>;
