
namespace exception {


  class Lines : public Inherit3(HavePosition, Dummy, RefCounter)
  {
  private:
    const float m_speed;
    const int m_cycle;
    std::deque<vector2> m_line;
    vector2 m_pos;
    vector2 m_direction;
    size_t m_frame;

    static float getRand()
    {
      static ist::Random s_rand(::time(0));
      return float(s_rand.genReal());
    }

  public:
    Lines(size_t length=50, float speed=5.0f, int cycle=320) :
        m_speed(speed), m_cycle(cycle), m_frame(0)
    {
      m_frame = size_t(getRand()*20.0f);
      m_direction = vector2(0.0f, 5.0f);
      m_pos = vector2(0, 0);
      m_line.resize(length, m_pos);
    };

    void update()
    {
      m_pos+=m_direction;
      m_line.push_front(m_pos);
      m_line.pop_back();

      if(++m_frame==m_cycle) {
        m_frame = 0;
        m_pos = vector2(0, 0);
        std::fill(m_line.begin(), m_line.end(), m_pos);
      }

      if(getRand() < 0.05f) {
        float f = float(getRand());
        if(f < 0.5f) {
          m_direction = vector2(m_speed, 0.0f);
        }
        else {
          m_direction = vector2(0.0f, m_speed);
        }
      }
    }

    void draw()
    {
      static std::vector<vector4> color;
      int length = int(m_line.size());
      color.clear();
      float u = 1.0f/length;
      for(int i=length; i>=0; --i) {
        color.push_back(vector4(u*i, u*i, 1.0f, u*i));
      }

      glBegin(GL_LINE_STRIP);
      for(int i=0; i<length; ++i) {
        glColor4fv(color[i].v);
        glVertex2fv(m_line[i].v);
      }
      glEnd();
      glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    }
  };
  typedef intrusive_ptr<Lines> line_ptr;
  typedef std::vector<line_ptr> line_cont;



  class BackgroundBase : public Inherit2(HavePosition, IEffect)
  {
  typedef Inherit2(HavePosition, IEffect) Super;
  protected:
    int m_fade;
    float m_opa;

    enum {
      NONE,
      FADEIN,
      FADEOUT,
    };

  public:
    BackgroundBase() : m_fade(NONE), m_opa(1.0f)
    {
      Register(this);
      fadein();
    }

    BackgroundBase(Deserializer& s) : Super(s)
    {
      s >> m_fade >> m_opa;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_fade << m_opa;
    }

    void fadeout() { m_fade=FADEOUT; }
    void fadein() { m_fade=FADEIN; }
    void setOpa(float v) { m_opa=v; }

    bool isFadeout() { return m_fade==FADEOUT; }
    bool isFadein() { return m_fade==FADEIN; }
    float getOpa() { return m_opa; }

    float getDrawPriority() { return 0.0f; }

    void drawFadeEffect()
    {
      if(m_opa>=0.0f) {
        glColor4f(0,0,0,m_opa);
        DrawRect(vector2(640, 480), vector2(0,0));
        glColor4f(1,1,1,1);
      }
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);

      if(m_fade==FADEIN) {
        m_opa-=(1.0f/60.0f);
        if(m_opa<=0.0f) {
          m_opa = 0;
          m_fade = NONE;
        }
      }
      else if(m_fade==FADEOUT) {
        m_opa+=(1.0f/480.0f);
        if(m_opa>=1.0f) {
          SendDestroyMessage(0, this);
        }
      }
    }
  };


  class Actor : public Optional
  {
  typedef Optional Super;
  public:
    Actor() {}
    Actor(Deserializer& s) : Super(s) {}
  };

  class ScrolledActor : public Inherit2(HaveParent, Actor)
  {
  typedef Inherit2(HaveParent, Actor) Super;
  public:
    ScrolledActor() {}
    ScrolledActor(Deserializer& s) : Super(s) {}

    virtual void scroll()
    {
      vector4 pos = getRelativePosition();
      pos+=getParentIMatrix()*GetGlobalScroll();
      setPosition(pos);
    }

    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);
      scroll();
    }
  };

} // exception 
