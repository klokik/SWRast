#include <chrono>
#include <iostream>
#include <fstream>
#include <random>
#include <sstream>
#include <string>
#include <tuple>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "vec.hh"

class Buffer
{
  public: size_t length;
  public: std::string name;

  public: enum class Role : int {COLOR = 1, DEPTH, STENCIL, VERTEX, INDEX, BITMAP, OTHER};

  public: Role buffer_role = Role::COLOR;

  public: void *data = nullptr;
};

class Line2
{
  public: float a;
  public: float b;
  public: float c;

  public: Line2(const vec2f p1, const vec2f p2)
  {
    this->a = p2.y - p1.y;
    this->b = p1.x - p2.x;
    this->c = p2.x*p1.y - p1.x*p2.y;
  }

  public: float atY(const float _y) const
  {
    assert(a != 0);
    return -(b*_y + c)/a;
  }

  public: float atX(const float _x) const
  {
    assert(b != 0);
    return -(a*_x + c)/b;
  }

  // .<|>.
/*  public: int sideOf(const vec2f point)
  {
    float d = a*point.x+b*point.y+c;
    return 1*(d > 0) - 1*(d < 0);
  }*/
};

inline constexpr float areClose(const float _a, const float _b)
{
  const float eps = 1e-4;
  auto diff = _a - _b;

  return _a == _b;
  // return (-eps < diff) && (diff < eps);
}

class Triangle;
using Soup = std::vector<Triangle>;

class Triangle
{
  private: vec3fi data[3] = {{0, 0, 0, -1},
                             {0, 0, 0, -1},
                             {0, 0, 0, -1}};

  public: Triangle()
  {}

  public: Triangle(const vec3fi _a, const vec3fi _b, const vec3fi _c)
  {
    this->data[0] = _a;
    this->data[1] = _b;
    this->data[2] = _c;

    this->sort();
  }

  public: Triangle(const vec3fi _data[3])
  {
    memcpy(&this->data[0], &_data[0], sizeof(vec3fi)*3);

    this->sortY();
  }

  public: void sortY()
  {
    std::sort(&this->data[0], &this->data[3], [](auto a, auto b) {
      return a.y > b.y;
    });
  }

  public: void sortX()
  {
    std::sort(&this->data[0], &this->data[3], [](auto a, auto b) {
      return a.x < b.x;
    });
  }

  public: void sort()
  {
    this->sortY();

    if (this->isAligned())
    {
      if (this->isDelta())
      {
        if (data[2].x < data[1].x)
          std::swap(data[1], data[2]);
      }
      else // nabla
      {
        if (data[1].x < data[0].x)
          std::swap(data[1], data[0]);
      }
    }
  }

  public: float top() const noexcept {
    return data[0].y;
  }

  public: float bottom() const noexcept {
    return data[2].y;
  }

  public: float left(const float _y) const noexcept
  {
    assert(this->isAligned());
    assert(_y >= this->top());
    assert(_y <= this->bottom());

    if (this->isDelta())
      return Line2(data[0].xy(), data[1].xy()).atY(_y);
    else
      return Line2(data[0].xy(), data[2].xy()).atY(_y);
  }

  public: float right(const float _y) const noexcept
  {
    assert(this->isAligned());
    assert(_y >= this->top());
    assert(_y <= this->bottom());

    if (this->isDelta())
      return Line2(data[0].xy(), data[2].xy()).atY(_y);
    else
      return Line2(data[1].xy(), data[2].xy()).atY(_y);
  }

  public: bool isAligned() const noexcept
  {
    return  (data[0].y == data[1].y) ||
            (data[1].y == data[2].y) ||
            (data[0].y == data[2].y);
  }

  public: bool isDelta() const noexcept
  {
    assert(this->isAligned());

    return areClose(data[1].y, data[2].y);
  }

  public: bool isNabla() const noexcept
  {
    assert(this->isAligned());

    return areClose(data[0].y, data[1].y);
  }

  public: vec3fi left() const noexcept {
      return data[this->isDelta() ? 1 : 0];
  }

  public: vec3fi right() const noexcept {
      return data[this->isDelta() ? 2 : 1];
  }

  public: vec3fi peak() const noexcept {
      return data[this->isDelta() ? 0 : 2];
  }

  public: const vec3fi &get(int _num) const {
    return data[_num];
  }

  public: std::pair<Triangle, Triangle> split()
  {
    assert(!this->isAligned());

    vec2f split_point2(Line2(data[0].xy(), data[2].xy()).atY(data[1].y), data[1].y);
    auto t = (split_point2 - data[0].xy()).length()/(data[2].xy() - data[0].xy()).length();

    vec3fi split_point(vec3f::lerp(data[0].xyz(), data[2].xyz(), t), -1);

    // to avoid calculation error
    split_point.y = data[1].y;

    Triangle delta{data[0], data[1], split_point};
    Triangle nabla{data[1], split_point, data[2]};

    if (delta.data[2].x < delta.data[1].x)
      std::swap(delta.data[1], delta.data[2]);
    if (nabla.data[1].x < nabla.data[0].x)
      std::swap(nabla.data[1], nabla.data[0]);

    return {delta, nabla};
  }

  public: vec3f getBaricentric(const vec3f _xyz)
  {
    Mat3f3 mat{{data[0].x, data[1].x, data[2].x,
                data[0].y, data[1].y, data[2].y,
                data[0].z, data[1].z, data[2].z}};

    return Mat3f3::linSolve(mat, _xyz);
  }

  friend Triangle operator+(const Triangle _a, const float _b);
  friend Triangle operator*(const Triangle _a, const float _b);
  friend Triangle operator*(const Triangle _a, const vec3fi _b);
  friend void rotateSoupX(const float _phi, Soup &_soup);
  friend void rotateSoupY(const float _phi, Soup &_soup);
};

Triangle operator+(const Triangle _a, const float _b)
{
  auto summand = vec3f{_b, _b, _b};

  return {_a.data[0].xyz() + summand,
          _a.data[1].xyz() + summand,
          _a.data[2].xyz() + summand};
}

Triangle operator*(const Triangle _a, const float _b)
{
  return {_a.data[0].xyz() * _b,
          _a.data[1].xyz() * _b,
          _a.data[2].xyz() * _b};
}

Triangle operator*(const Triangle _a, const vec3fi _b)
{
  return {_a.data[0] * _b,
          _a.data[1] * _b,
          _a.data[2] * _b};
}

// using Soup = std::vector<Triangle>;

void rotateSoupY(const float _phi, Soup &_soup)
{
  float s = std::sin(_phi);
  float c = std::cos(_phi);

  for (auto &triag : _soup)
  {
    for (auto &vtx : triag.data)
    {
      vec3fi xz = vtx;
      vtx.x = xz.x*c - xz.z*s;
      vtx.z = xz.x*s + xz.z*c;
    }
    triag.sort();
  }
}

void rotateSoupX(const float _phi, Soup &_soup)
{
  float s = std::sin(_phi);
  float c = std::cos(_phi);

  for (auto &triag : _soup)
  {
    for (auto &vtx : triag.data)
    {
      vec3fi yz = vtx;
      vtx.y = yz.y*c - yz.z*s;
      vtx.z = yz.y*s + yz.z*c;
    }
    triag.sort();
  }
}

class SoupLoader
{
  public: static int loadStl(const std::string &_filename, Soup &_dst)
  {
    int ret = 0;
    std::ifstream ifs(_filename, std::ios_base::in);

    char line[256] = {0};
    std::vector<vec3fi> tris;
    tris.reserve(3);

    while (ifs.getline(&line[0], 256))
    {
      std::stringstream sstr(line);
      std::string word;
      sstr >> word;

      if (word == "endloop")
      {
        _dst.emplace_back(tris.data());
        tris.clear();
      }

      if (word != "vertex")
        continue;

      float x, y, z;
      sstr >> x >> y >> z;
      tris.emplace_back(x, y, z, ret++);
    }

    ifs.close();
  }
};

class Scanline
{
  public: struct Record {
    float start;
    float end;
    float depth_start;
    float depth_end;
    int triangle_id;
    vec3f baric_start;
    vec3f baric_end;
  };

  int id;
  public: std::vector<Record> items;
};

template<typename T>
class Rect
{
  public: T left;
  public: T top;
  public: T right;
  public: T bottom;

  public: Rect(const T _left, const T _top, const T _right, const T _bottom):
    left(_left), top(_top), right(_right), bottom(_bottom)
  {
  }

  public: T width() const noexcept { return right - left + 1; };
  public: T height()  const noexcept { return bottom - top + 1; };
};

using Recti = Rect<int>;

template<typename T>
inline bool inRange(const T _val, const T _start, const T _end) { return (_val >= _start) && (_val <= _end); }

class Rasterizer
{
  public: void rasterize(Soup & _triangles, const Recti _dst_rect, Buffer & _dst)
  {
    std::vector<Scanline> line_batch(_dst_rect.height());

    // fill scanlines
    Soup nablas;
    int index = 0;
    for (auto iter = _triangles.begin(); ; ++iter, ++index)
    {
      // continue thru the rest of triangles - nablas
      if (iter == _triangles.end())
      {
        if (nablas.empty())
          break;

        iter = nablas.begin();
      }
      else if (iter == nablas.end())
        break;

      // split triangles
      if (!iter->isAligned())
      {
        Triangle delta;
        Triangle nabla;
        std::tie(delta, nabla) = iter->split();

        assert(delta.isAligned());
        assert(nabla.isAligned());

        *iter = delta;
        nablas.push_back(nabla);
      }

      auto triag = (((*iter)+1)*0.5f)*vec3fi(_dst_rect.width(), _dst_rect.height(), 1, 1);

      // I'll regret that I've done it
      *iter = triag;

      // create records in corresponding scanlines
      int top =     std::min((int)std::lround(triag.top()), _dst_rect.height()-1);
      int bottom =  std::max((int)std::lround(triag.bottom()), 0);

      float inv_height = 1.f / (top - bottom);

      // avoid invisible triangles
      if (top == bottom)
        continue;

      for (int i = top; i >= bottom; --i)
      {
        float t = (top - i)*inv_height;
        assert(inRange(t, 0.f, 1.f));

        if (triag.isNabla())
          t = 1-t;

        vec3f start = vec3f::lerp(triag.peak().xyz(), triag.left().xyz(), t);
        vec3f end = vec3f::lerp(triag.peak().xyz(), triag.right().xyz(), t);

        // avoid too narrow edges
        if (std::round(end.x) == std::round(start.x))
          continue;

        Scanline::Record sl_record = {
          .start = start.x,
          .end = end.x,
          .depth_start = start.z,
          .depth_end = end.z,
          .triangle_id = index,
          .baric_start = triag.getBaricentric(start),
          .baric_end = triag.getBaricentric(end),
        };

        // FIXME: init id once
        line_batch[i].id = i;
        line_batch[i].items.push_back(sl_record);
      }
    }
    _triangles.insert(_triangles.end(), nablas.begin(), nablas.end());

    // draw scanlines
    assert(_dst_rect.left == 0 && _dst_rect.top == 0);
    std::vector<float> z_buf(_dst_rect.width(), 1.0f);

    for (auto & line : line_batch)
    {
      // clear z-buffer
      for (auto &&z : z_buf)
        z = 1.0f;

      for (auto record : line.items)
      {
        int start = std::lround(record.start);
        int end = std::lround(record.end);
        float dt = (record.depth_end - record.depth_start) / (end - start);

        auto a_start = std::max(start, 0);
        auto a_end = std::min(end, _dst_rect.width()-1);

        auto triag = _triangles[record.triangle_id];

        for (int i = a_start; i <= a_end; ++i)
        // for (auto i : a_start, a_end})
        {
          int id = _dst_rect.width()*(_dst_rect.bottom - line.id)+i;

          auto depth = 1 - (record.depth_start + (i - start)*dt);

          auto frag_b_coord = vec3f::lerp(record.baric_start, record.baric_end,
                                          (float)(i-start)/(end-start));
          auto frag_coord = triag.get(0)*frag_b_coord.x +
                            triag.get(1)*frag_b_coord.y +
                            triag.get(2)*frag_b_coord.z;
          // auto depth = 1-frag_coord.z;

          if (z_buf[i] < depth)
            continue;

          z_buf[i] = depth;

          auto color = static_cast<uint8_t>(depth*255);
          static_cast<uint8_t*>(_dst.data)[id] = color;
        }
      }
    }
  }
};

class Display
{
  public: void present(const Buffer & _src, const Recti _roi)
  {
    // FIXME: use _roi location
    assert(_roi.top == 0 && _roi.left == 0);
    assert(_src.data != nullptr && _src.length >= _roi.width()*_roi.height());
    // directly uses buffer data without copying
    cv::Mat mat(_roi.height(), _roi.width(), CV_8UC1, _src.data);

    cv::imshow("Software render", mat);
  }

  public: void commit()
  {
    cv::waitKey(1);
  }

  public: void pause()
  {
    cv::waitKey(0);
  }
};


int main()
{

  Soup soup;

  std::random_device rd;
  std::mt19937 rng(rd());
  std::uniform_real_distribution<float> dist(-1, 1);

  // for (int i = 0; i < 10; ++i)
  // {
  //   Triangle tri{ { dist(rng), dist(rng), dist(rng), 1},
  //                 { dist(rng), dist(rng), dist(rng), 2},
  //                 { dist(rng), dist(rng), dist(rng), 3}};

  //   if (!tri.isAligned())
  //     soup.push_back(tri);
  // }

  SoupLoader::loadStl("../susan.stl", soup);
  // SoupLoader::loadStl("../sphere.stl", soup);

  // soup.push_back({{ 0.0f, 0.5f, 0.0f, 1},
  //                 {-0.5f,-0.5f, 0.0f, 2},
  //                 { 0.2f,-0.6f, 0.0f, 3}});


  auto roi = Rect<int>(0, 0, 240 - 1, 240 - 1);
  Buffer rbuf;

  rbuf.length = roi.width() * roi.height() * 1;
  rbuf.data = new uint8_t[rbuf.length];
  rbuf.buffer_role = Buffer::Role::COLOR;
  rbuf.name = "Color renderbuffer 0";

  Rasterizer raster;
  Display display;


  std::chrono::time_point<std::chrono::system_clock> start, end;
  start = std::chrono::system_clock::now();
  int frames = 100;
  for (int i = 0; i < frames; ++i)
  {
    Soup soup_copy = soup;
    rotateSoupX(-0.333f*i, soup_copy);
    rotateSoupY(0.1f*i, soup_copy);

    memset(rbuf.data, 0, rbuf.length);
    raster.rasterize(soup_copy, roi, rbuf);

    display.present(rbuf, roi);
    display.commit();
  }
  end = std::chrono::system_clock::now();

  std::chrono::duration<double> elapsed = end-start;

  std::cout << "Avg FPS: " << frames/elapsed.count() << std::endl;

  display.present(rbuf, roi);
  display.commit();
  display.pause();

  return 0;
}
