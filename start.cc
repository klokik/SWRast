#include <iostream>
#include <random>

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

    this->sortY();
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

    return (data[1].y == data[2].y);
  }

  public: bool isNabla() const noexcept
  {
    assert(this->isAligned());

    return (data[0].y == data[1].y);
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

  public: std::pair<Triangle, Triangle> split()
  {
    assert(!this->isAligned());

    vec2f split_point2(Line2(data[0].xy(), data[2].xy()).atY(data[1].y), data[1].y);
    auto t = (split_point2 - data[0].xy()).length()/(data[2].xy() - data[0].xy()).length();

    vec3fi split_point(vec3f::lerp(data[0].xyz(), data[2].xyz(), t), -1);

    Triangle delta{data[0], data[1], split_point};
    Triangle nabla{data[1], split_point, data[2]};

    if (delta.data[2].x < delta.data[1].x)
      std::swap(delta.data[1], delta.data[2]);
    if (nabla.data[1].x < nabla.data[0].x)
      std::swap(nabla.data[1], nabla.data[0]);

    return {delta, nabla};
  }

  friend Triangle operator+(const Triangle _a, const float _b);
  friend Triangle operator*(const Triangle _a, const float _b);
  friend Triangle operator*(const Triangle _a, const vec3fi _b);
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

using Soup = std::vector<Triangle>;

class Scanline
{
  public: struct Record {
    float start;
    float end;
    float depth_start;
    float depth_end;
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
    auto *triangles_range = &_triangles;
    for (auto iter = triangles_range->begin(); iter != triangles_range->end(); ++iter)
    {
      // continue thru the rest of triangles - nablas
      if (iter == triangles_range->end())
        triangles_range = &nablas;

      // split triangles
      if (!iter->isAligned())
      {
        Triangle delta;
        Triangle nabla;
        std::tie(delta, nabla) = iter->split();

        assert(delta.isAligned());
        assert(nabla.isAligned());

        *iter = delta;
        // _triangles.push_back(nabla);
        nablas.push_back(nabla);
      }

      auto triag = (((*iter)+1)*0.5f)*vec3fi(_dst_rect.width(), _dst_rect.height(), 1, 1);

      // create records in corresponding scanlines
      for (int i = triag.top(); i >= triag.bottom(); --i)
      {
        float t = (triag.top()-i) / (triag.top()-triag.bottom());
        assert(inRange(t, 0.f, 1.f));

        if (triag.isNabla())
          t = 1-t;

        vec3f start = vec3f::lerp(triag.peak().xyz(), triag.left().xyz(), t);
        vec3f end = vec3f::lerp(triag.peak().xyz(), triag.right().xyz(), t);

        Scanline::Record sl_record = {
          .start = start.x,
          .end = end.x,
          .depth_start = start.z,
          .depth_end = end.z
        };

        // FIXME: init id once
        line_batch[i].id = i;
        line_batch[i].items.push_back(sl_record);
      }
    }

    // draw scanlines
    assert(_dst_rect.left == 0 && _dst_rect.top == 0);
    for (auto & line : line_batch)
    {
      for (auto record : line.items)
      {
        for (int i = record.start; i <= record.end; ++i)
        {
          int id = _dst_rect.width()*line.id+i;
          static_cast<uint8_t*>(_dst.data)[id] = 127;
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
    cv::waitKey(30);
  }

  public: void pause()
  {
    cv::waitKey(0);
  }

  public: int width = 320;
  public: int height = 240;
};


int main()
{

  Soup soup;

  std::random_device rd;
  std::mt19937 rng(rd());
  std::uniform_real_distribution<float> dist(-1, 1);

  for (int i = 0; i < 1; ++i)
  {

    soup.push_back({{ dist(rng), dist(rng), dist(rng), 1},
                    { dist(rng), dist(rng), dist(rng), 2},
                    { dist(rng), dist(rng), dist(rng), 3}});
  }

  // soup.push_back({{ 0.0f, 0.5f, 0.0f, 1},
  //                 {-0.5f,-0.5f, 0.0f, 2},
  //                 { 0.2f,-0.6f, 0.0f, 3}});


  Buffer rbuf;
  rbuf.length = 320 * 240 * 1;
  rbuf.data = new uint8_t[rbuf.length];
  rbuf.buffer_role = Buffer::Role::COLOR;
  rbuf.name = "Color renderbuffer 0";

  auto roi = Rect<int>(0, 0, 320 - 1, 240 - 1);

  Rasterizer raster;
  raster.rasterize(soup, roi, rbuf);

  Display display;
  display.present(rbuf, roi);
  display.commit();
  display.pause();

  return 0;
}