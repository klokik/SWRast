
template<class T>
class vec2
{
  public: vec2(const T _x, const T _y):
    x(_x), y(_y)
  {}

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

  public: vec31(const T _x, const T _y, const T _z):
    x(_x), y(_y), z(_z)
  {}

  public: vec31(const vec2<T> _xy, const T _z):
    x(_xy.x), y(_xy.y), z(_z)
  {}

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
static vec31<T, U> operator*(const T _c, const vec3i<T, U> _a) {
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

