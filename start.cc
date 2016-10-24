#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "vec.hh"

class Buffer
{
public: size_t length;
public: std::string name;

public: enum class role {COLOR, DEPTH, STENCIL, VERTEX, INDEX, BITMAP, OTHER}: int;

public: void *data = nullptr;
};

class Line2
{
  public: float a;
  public: float b;
  public: float c;

  public: Line(const vec2f p1, const vec2f p2);

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
  private: vec3fi data[3];

  public: Triangle(const vec3fi[3] & _data)
  {
    memcpy(&this->data[0], &_data[0], sizeof(vec3fi)*3);

    this->sortY();
  }

  public: void sortY()
  {
    std::sort(&this->data[0], &this->data[3], [](auto a, auto b) {
      return a.y < b.y;
    });
  }

  public: void sortX()
  {
    std::sort(&this->data[0], &this->data[3], [](auto a, auto b) {
      return a.x < b.x;
    });
  }

  public: float top() const noexcept
  {
    return data[0].y;
  }

  public: float bottom() const noexcept
  {
    return data[2].y;
  }

  public: float left(const float _y) const noexcept
  {
    assert(this->isAligned())
    assert(y >= this->top());
    assert(y <= this->bottom());

    if (this->isDelta())
      return Line2(data[0].xy, data[1]).atY(_y);
    else
      return Line2(data[0].xy, data[2]).atY(_y);
  }

  public: float right(const float _y) const noexcept
  {
    assert(this->isAligned())
    assert(y >= this->top());
    assert(y <= this->bottom());

    if (this->isDelta())
      return Line2(data[0].xy, data[2]).atY(_y);
    else
      return Line2(data[1].xy, data[2]).atY(_y);
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

  public: std::pair<Triangle, Triangle> split()
  {
    assert(!this->aligned());


    vec2f split_point2(Line2(data[0].xy, data[1].xy).atY(data[1].y), data[1].y);
    auto t = (split_point2 - data[1].xy).length()/(data[2].xy - data[0].xy).length();

    vec3fi split_point(vec3f::lerp(data[0].xyz, data[2].xyz, t), -1);

    Triangle delta(data[0], data[1], split_point);
    Triangle nabla(data[1], split_point, data[2]);

    if (delta.data[2].x > delta.data[1].x)
      std::swap(delta.data[1], delta.data[2]);
    if (nabla.data[1].x > nabla.data[0].x)
      std::swap(nabla.data[1], nabla.data[0]);

    return {delta, nabla};
  }
};

using Soup = std::vector<Triangle>;

class Scanline
{
public: struct Record {
    int num;
    float start;
    float end;
    float depth_start;
    float depth_end;
  };

public: std::vector<Record> items;
};

template<typename T>
class Rect<T>
{
  public: T left;
  public: T top;
  public: T right;
  public: T bottom;

  public: Rect(const T _left, const T _top, const T _right, cont T _bottom):
    left(_left), top(_top), right(_right), bottom(_bottom)
  {
  }

  public: T width() const noexcept { return _right - _left + 1; };
  public: T height()  const noexcept { return _bottom - _top + 1; };
}

using Recti = Rect<int>;

template<typename T>
inline bool inRange(const T _val, const T _start, const T _end) { return (_val >= _start) && (_val <= _end); }

class Rasterizer
{
  public: resterize(Soup & _triangles, const Buffer & _dst, const Recti _dst_rect)
  {
    std::vector<Scanline> line_batch(_dst_rect.height());

    // fill scanlines
    for (auto iter = _triangles.begin(); iter != _triangles.end(); ++iter)
    {
      // split triangles
      if (!iter->isAligned())
      {
        Triangle delta;
        Triangle nabla;
        std::tie(delta, nabla) = iter->split();

        *iter = delta;
        _triangles.push_back(nabla);
      }

      auto triag = (((*iter)+1)/2)*vec3fi(_dst_rect.width(), _dst_rect.height(), 1, 1);

      // create records in corresponding scanlines
      for (int i = triag.top(); i <= triag.bottom; ++i)
      {
        float t = (i-triag.top()) / (triag.bottom()-triag.top());
        assert(inRange(t, 0, 1));

        if (triag.isNabla())
          t = 1-t;

        vec3f start = vec3f::lerp(triag.peak(), triag.left(), t);
        vec3f end = vec3f::lerp(triag.peak(), triag.right(), t);

        Scanline::Record sl_record = {
          .num = i,
          .start = start.x,
          .end = end.x,
          .depth_start = start.z,
          .depth_end = end.z
        };

        line_batch[i].push_back(sl_record);
      }
    }

    // draw scanlines
    assert(_dst_rect.left == 0 && _dst_rect.top == 0);
    for (auto line : line_batch)
    {
      for (int i = line.start; i <= line.end; ++i)
      {
        int id = _dst_rect.width*line.num+i;
        _dst.data[id] = 128;
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
    assert(_src.data != nullptr && _src.length >= _roi.width*_roi.height);
    // directly uses buffer data without copying
    cv::Mat mat(_roi.height, _roi.width, CV_UC1, _src.data);

    cv::imshow("Software render", mat);
    cv::waitKey(1000);
  }

  public: int width = 320;
  public: int height = 240;
}