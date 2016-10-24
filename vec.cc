
template<class T>
class vec2
{
  T x;
  T y;
}

template<class T>
class vec3
{
  T x;
  T y;
  T z;
}

template<class T, class U>
class vec31
{
  T x;
  T y;
  T z;
  U w;
}

template<class T>
using vec4<T> = vec31<T, T>;

static v