#ifndef enemy_hfighter_h
#define enemy_hfighter_h

namespace exception {


  class HeavyFighter : public Inherit2(HaveDirection, Enemy)
  {
  typedef Inherit2(HaveDirection, Enemy) Super;
  public:
    class Parts : public ChildEnemy
    {
    typedef ChildEnemy Super;
    public:
      Parts(Deserializer& s) : Super(s) {}
      Parts() {}

      void onCollide(CollideMessage& m)
      {
        solid_ptr s = ToSolid(m.getFrom());
        if(s && s->getVolume()>=1000000.0f) {
          SendDestroyMessage(s, this);
        }
        else {
          Scratch(this, m, 0.1f);
        }
      }
    };

    class Arm : public Parts
    {
    typedef Parts Super;
    private:
      vector4 m_vel;

    public:
      Arm(Deserializer& s) : Super(s)
      {
        s >> m_vel;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        s << m_vel;
      }

    public:
      Arm() {}
      void setVel(const vector4& v) { m_vel=v; }
      const vector4& getVel() { return m_vel; }

      void onUpdate(UpdateMessage& m)
      {
        Super::onUpdate(m);

        m_vel*=0.87f;
        vector4 pos = getRelativePosition();
        pos+=m_vel;
        pos.x*=0.98f;
        setPosition(pos);
      }
    };

  private:
    static const int s_num_parts = 3;
    static const int s_num_arm = 2;
    enum {
      MOVE,
      MOVE_READY,
      ATTACK,
      ATTACK_READY,
    };

    Parts *m_parts[s_num_parts];
    Arm *m_arms[s_num_arm];
    float m_freq;
    float m_rot;
    float m_army;
    int m_mode;

  public:
    HeavyFighter(Deserializer& s) : Super(s)
    {
      DeserializeLinkage(s, m_parts);
      DeserializeLinkage(s, m_arms);
      s >> m_freq >> m_rot >> m_army >> m_mode;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      SerializeLinkage(s, m_parts);
      SerializeLinkage(s, m_arms);
      s << m_freq << m_rot << m_army << m_mode;
    }

    void reconstructLinkage()
    {
      Super::reconstructLinkage();
      ReconstructLinkage(m_parts);
      ReconstructLinkage(m_arms);
    }

  public:
    HeavyFighter(controler_ptr c=0) : m_freq(0.0f), m_rot(0.0f), m_army(0.0f), m_mode(MOVE)
    {
      setControler(c);
      setBox(box(vector4(10), vector4(-10)));
      setLife(20.0f);
      setEnergy(80.0f);
      setBound(box(vector4(1400)));

      {
        box b[s_num_parts] = {
          box(vector4(120, 15, 20), vector4(20, -15, -20)),
          box(vector4(50,  40, 15), vector4(-35,  15, -15)),
          box(vector4(50, -40, 15), vector4(-35, -15, -15)),
        };
        float l[] = {60,35,35};
        for(int i=0; i<s_num_parts; ++i) {
          m_parts[i] = new Parts();
          m_parts[i]->setParent(this);
          m_parts[i]->setBox(b[i]);
          m_parts[i]->setLife(l[i]);
        }
      }
      {
        box b[s_num_arm] = {
          box(vector4(0,  70, 20), vector4(-100,  40, -20)),
          box(vector4(0, -70, 20), vector4(-100, -40, -20)),
        };
        for(int i=0; i<s_num_arm; ++i) {
          m_arms[i] = new Arm();
          m_arms[i]->setParent(this);
          m_arms[i]->setBox(b[i]);
          m_arms[i]->setLife(50);
        }
      }
      setGroup(createGroupID());
    }

    void hyper()
    {
      Super::hyper();
      setLife(20.0f);
      for(int i=0; i<s_num_parts; ++i) {
        m_parts[i]->hyper();
      }
      for(int i=0; i<s_num_arm; ++i) {
        m_arms[i]->hyper();
      }
    }

    float getDrawPriority() { return 1.1f; }

    void setGroup(gid v)
    {
      Super::setGroup(v);
      SetGroup(m_parts, v);
      SetGroup(m_arms, v);
    }

    float getRotate() { return m_rot; }
    void setRotate(float v) { modMatrix(); m_rot=v; }

    Parts* getParts(int i) { return i>=s_num_parts ? 0 : m_parts[i]; }
    Arm* getArm(int i) { return i>=s_num_arm ? 0 : m_arms[i]; }

    void updateMatrix(matrix44& mat)
    {
      Super::updateMatrix(mat);
      mat.rotateX(getRotate());
    }

    void draw()
    {
      glMaterialfv(GL_FRONT, GL_EMISSION, vector4(0.2f, 0.2f, 1.0f).v);
      Super::draw();
      glMaterialfv(GL_FRONT, GL_EMISSION, vector4().v);

      ChildEnemy *burner[4] = {
        m_parts[1], m_parts[2],
        m_arms[0], m_arms[1]
      };
      vector4 pos[4] = {
        vector4(-40,   30, 0),
        vector4(-40,  -30, 0),
        vector4(-105,  55, 0),
        vector4(-105, -55, 0),
      };
      for(int i=0; i<4; ++i) {
        ChildEnemy *p = burner[i];
        if(p) {
        DrawSprite("burner.png",
            getMatrix()*(pos[i]+p->getRelativePosition()),
            vector4(35.0f+::sinf(m_freq*ist::radian)*3.0f));
        }
      }
    }

    void shootBlaster()
    {
      if(m_mode!=ATTACK_READY) {
        return;
      }

      vector4 pos[2] = {
        vector4(0, 55, 0),
        vector4(0,-55, 0),
      };
      for(int i=0; i<2; ++i) {
        Arm *p = m_arms[i];
        if(p) {
          Blaster *b = new Blaster(p, getMatrix()*vector4(1,0,0,0));
          b->setPosition(getMatrix()*(pos[i]+p->getRelativePosition()));
          p->setVel(getDirection()*3.0f);
        }
      }
    }

    float getArmY() { return m_army; }
    void setArmY(float y)
    {
      m_army = y;
      for(int i=0; i<s_num_arm; ++i) {
        if(Arm *a = m_arms[i]) {
          vector4 pos = a->getRelativePosition();
          pos.y = i==0 ? y : -y;
          a->setPosition(pos);
        }
      }
    }

    void open()  { m_mode=ATTACK; }
    void close() { m_mode=MOVE; }


    void onUpdate(UpdateMessage& m)
    {
      Super::onUpdate(m);
      SweepDeadObject(m_parts);
      SweepDeadObject(m_arms);

      if(!AliveAny(m_parts) && !AliveAny(m_arms)) {
        SendDestroyMessage(0, this);
        return;
      }
      m_freq+=53.0f;

      if(m_mode==ATTACK) {
        float y = getArmY();
        y = std::min<float>(y+0.5f, 30.0f);
        if(y==30.0f) {
          m_mode = ATTACK_READY;
        }
        setArmY(y);
      }
      else if(m_mode==MOVE) {
        float y = getArmY();
        y = std::max<float>(y-0.5f, 0.0f);
        if(y==30.0f) {
          m_mode = MOVE_READY;
        }
        setArmY(y);
      }
    }

    void onDestroy(DestroyMessage& m)
    {
      Super::onDestroy(m);

      if(m.getStat()==0) {
        PutMediumImpact(getPosition());
      }
    }
  };


  class HeavyFighter_Controler : public TControler<HeavyFighter>
  {
  typedef TControler<HeavyFighter> Super;
  public:
    typedef HeavyFighter::Parts Parts;
    typedef HeavyFighter::Arm Arm;

    HeavyFighter_Controler(Deserializer& s) : Super(s) {}
    HeavyFighter_Controler() {}

    Getter(getParent, gobj_ptr);
    Getter(getMatrix, const matrix44&);
    Getter(getParentMatrix, const matrix44&);
    Getter(getParentIMatrix, const matrix44&);
    Getter(getRelativePosition, const vector4&);
    Getter(getGroup, gid);
    Getter(getPosition, const vector4&);
    Getter(getDirection, const vector4&);
    Getter(getRotate, float);
    Getter2(getParts, Parts*, int);
    Getter2(getArm, Arm*, int);
    Getter(isHyper, bool);

    Setter(setGroup, gid);
    Setter(setPosition, const vector4&);
    Setter(setDirection, const vector4&);
    Setter(setRotate, float);

    Caller(open);
    Caller(close);
  };


  // �@�������˂�����ł��遨U�^�[�����ă~�T�C�������ޏ� 
  class HeavyFighter_PutMines : public HeavyFighter_Controler
  {
  typedef HeavyFighter_Controler Super;
  private:
    int m_frame;
    float m_vel;
    vector4 m_initial_pos;
    vector4 m_move;
    vector4 m_dir;
    int m_action;

  public:
    HeavyFighter_PutMines(Deserializer& s) : Super(s)
    {
      s >> m_frame >> m_vel >> m_initial_pos >> m_move >> m_dir >> m_action;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_frame << m_vel << m_initial_pos << m_move << m_dir << m_action;
    }

  public:
    HeavyFighter_PutMines() : m_frame(0), m_vel(0.0f), m_action(0)
    {}

    void onConstruct(ConstructMessage& m)
    {
      m_initial_pos = getRelativePosition();
      m_dir = getDirection();
      m_move = m_dir*900.0f;
      open();
    }

    void move()
    {
      if(m_frame==150) {
        vector4 mpos[2] = {vector4(-40, 30, 0), vector4(-40, -30, 0)};
        int parts[2] = {1, 2};
        for(int i=0; i<2; ++i) {
          if(getParts(parts[i])) {
            BurstMine *e = new BurstMine();
            e->setParent(getParent());
            e->setPosition(getParentIMatrix()*getMatrix()*mpos[i]);
            e->setVel(-getDirection()*4.0f);
            e->setGroup(getGroup());
          }
        }
      }

      if(m_frame==350) {
        close();
      }
      if(m_frame<=600) {
        setPosition(m_initial_pos+m_move*Sin90(1.0f/500*m_frame));
      }
      if(m_frame>450 && m_frame<=600) {
        setDirection(matrix44().rotateZ(Cos180I(1.0f/150.0f*(m_frame-450))*180.0f)*m_dir);
      }

      if(m_frame==600) {
        m_frame = 0;
        ++m_action;
      }
    }

    void away()
    {
      if(m_frame==1) {
        for(int i=0; i<2; ++i) {
          if(Arm *arm = getArm(i)) {
            vector4 dir = matrix44().rotateZ(i==0 ? 135 : -135)*getDirection();
            for(int j=0; j<2; ++j) {
              Missile *e = new BurstMissile(false);
              e->setParent(getParent());
              e->setDirection(getDirection());
              e->setPosition(getParentIMatrix()*arm->getCenter());
              e->setGroup(getGroup());
              e->setMoveTarget(dir*(75.0f*(j+1)));
            }
          }
        }
      }
      m_vel+=0.02f;
      setPosition(getRelativePosition()+getDirection()*m_vel);

      if(m_frame==270) {
        SendKillMessage(0, get());
      }
    }

    void onUpdate(UpdateMessage& m)
    {
      ++m_frame;

      switch(m_action) {
      case 0: move(); break;
      case 1: away(); break;
      }
    }
  };

  class HeavyFighter_Straight : public HeavyFighter_Controler
  {
  typedef HeavyFighter_Controler Super;
  private:
    vector4 m_initial_pos;
    vector4 m_target_pos;
    float m_speed;
    int m_frame;

  public:
    HeavyFighter_Straight(Deserializer& s) : Super(s)
    {
      s >> m_initial_pos >> m_target_pos >> m_speed >> m_frame;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_initial_pos << m_target_pos << m_speed << m_frame;
    }

  public:
    HeavyFighter_Straight() : m_speed(0.0f), m_frame(0)
    {}

    void onConstruct(ConstructMessage& m)
    {
      m_initial_pos = getRelativePosition();
      m_target_pos = m_initial_pos+getDirection()*500.0f;
    }

    void onUpdate(UpdateMessage& m)
    {
      int f = ++m_frame;
      if(f < 200) {
        setPosition(m_initial_pos+(m_target_pos-m_initial_pos)*Sin90(1.0f/200*m_frame));
      }
      else if(f < 800) {
        if(m_speed<1.25f) {
          m_speed+=0.01f;
        }
        setPosition(getRelativePosition()+getDirection()*m_speed);
      }
      else {
        if(m_speed<2.5f) {
          m_speed+=0.006f;
        }
        setPosition(getRelativePosition()+getDirection()*m_speed);
      }

      if(m_frame==300 || m_frame==500 || m_frame==700) {
        vector4 mpos[2] = {vector4(-40, 30, 0), vector4(-40, -30, 0)};
        int parts[2] = {1, 2};
        for(int i=0; i<2; ++i) {
          if(getParts(parts[i])) {
            BurstMine *e = new BurstMine();
            e->setParent(getParent());
            e->setPosition(getParentIMatrix()*getMatrix()*mpos[i]);
            e->setVel(-getDirection()*4.0f);
            e->setGroup(getGroup());
          }
        }
      }
    }
  };

  // ���ꁨ�~�T�C���A�ˁ�U�^�[�����đޏ� 
  class HeavyFighter_Missiles : public HeavyFighter_Controler
  {
  typedef HeavyFighter_Controler Super;
  protected:
    int m_frame;
    float m_vel;
    vector4 m_move;
    vector4 m_dir;
    bool m_scroll;
    int m_action;

  public:
    HeavyFighter_Missiles(Deserializer& s) : Super(s)
    {
      s >> m_frame >> m_vel >> m_move >> m_dir >> m_scroll >> m_action;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_frame << m_vel << m_move << m_dir << m_scroll << m_action;
    }

  public:
    HeavyFighter_Missiles(bool scroll) :
      m_frame(0), m_vel(0.0f), m_scroll(scroll), m_action(0)
    {}

    void onConstruct(ConstructMessage& m)
    {
      m_dir = getDirection();
      m_move = m_dir*400.0f;
    }

    void move()
    {
      if(m_frame<=180) {
        const vector4& pos = getRelativePosition();
        setPosition(pos+m_move*(Sin90(1.0f/180*m_frame)-Sin90(1.0f/180*(m_frame-1))));
      }

      if(m_frame==180) {
        m_frame = 0;
        ++m_action;
      }
    }

    void fire()
    {
      int f = m_frame;
      if(f==30 || f==90 || f==150 || f==210) {
        vector4 mpos[2] = {vector4(45, 30, 0), vector4(45, -30, 0)};
        int parts[2] = {1, 2};
        for(int i=0; i<2; ++i) {
          if(getParts(parts[i])) {
            Missile *e = new BurstMissile(m_scroll);
            e->setParent(getParent());
            e->setDirection(getDirection());
            e->setPosition(getParentIMatrix()*getMatrix()*mpos[i]);
            e->setGroup(getGroup());
          }

          if(Arm *arm = getArm(i)) {
            vector4 dir = matrix44().rotateZ(i==0 ? 135 : -135)*getDirection();
            Missile *e = new BurstMissile(m_scroll);
            e->setParent(getParent());
            e->setDirection(getDirection());
            e->setPosition(getParentIMatrix()*arm->getCenter());
            e->setGroup(getGroup());
            e->setMoveTarget(dir*50.0f);
          }
        }
      }
      if(f>300 && f<=450) {
        setDirection(matrix44().rotateZ(Cos180I(1.0f/150.0f*(m_frame-300))*180.0f)*m_dir);
      }

      if(f==450) {
        m_frame = 0;
        ++m_action;
      }
    }

    void away()
    {
      m_vel+=0.03f;
      setPosition(getRelativePosition()+getDirection()*m_vel);

      if(m_frame==200) {
        SendKillMessage(0, get());
      }
    }

    void onUpdate(UpdateMessage& m)
    {
      ++m_frame;
      switch(m_action) {
      case 0: move(); break;
      case 1: fire(); break;
      case 2: away(); break;
      }

      if(m_scroll) {
        setPosition(getRelativePosition()+getParentIMatrix()*GetGlobalScroll());
      }
    }
  };

  // 4�{�X�p
  // ���ꁨ���@�������~�T�C���A�ˁ�U�^�[�����đޏ� 
  class HeavyFighter_Missiles2 : public HeavyFighter_Controler
  {
  typedef HeavyFighter_Controler Super;
  protected:
    int m_frame;
    float m_vel;
    vector4 m_move;
    vector4 m_dir;
    bool m_scroll;
    int m_action;

  public:
    HeavyFighter_Missiles2(Deserializer& s) : Super(s)
    {
      s >> m_frame >> m_vel >> m_move >> m_dir >> m_scroll >> m_action;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_frame << m_vel << m_move << m_dir << m_scroll << m_action;
    }

  public:
    HeavyFighter_Missiles2(bool scroll) :
      m_frame(0), m_vel(0.0f), m_scroll(scroll), m_action(0)
    {}

      void onConstruct(ConstructMessage& m)
    {
      m_dir = getDirection();
      m_move = m_dir*400.0f;
    }

    void move()
    {
      int f = m_frame;

      if(f<=180) {
        const vector4& pos = getRelativePosition();
        float pq = Sin90(1.0f/180*(f-1));
        float q = Sin90(1.0f/180*f);
        setPosition(pos+m_move*(q-pq));
      }
      if(f==180) {
        m_frame = 0;
        ++m_action;
      }
    }

    void aim()
    {
      int f = ++m_frame;
      vector4 dir = getParentIMatrix()*(GetNearestPlayerPosition(getPosition())-getPosition()).normal();
      setDirection(getDirection()+dir*0.03f);
      if(f>=120) {
        m_frame = 0;
        ++m_action;
      }
    }

    void fire()
    {
      int f = m_frame;
      if(f==30 || f==90 || f==150 || f==210) {
        vector4 mpos[2] = {vector4(45, 30, 0), vector4(45, -30, 0)};
        int parts[2] = {1, 2};
        for(int i=0; i<2; ++i) {
          if(getParts(parts[i])) {
            Missile *e = new BurstMissile(m_scroll);
            e->setParent(getParent());
            e->setDirection(getDirection());
            e->setPosition(getParentIMatrix()*getMatrix()*mpos[i]);
            e->setGroup(getGroup());
          }

          if(Arm *arm = getArm(i)) {
            vector4 dir = matrix44().rotateZ(i==0 ? 135 : -135)*getDirection();
            Missile *e = new BurstMissile(m_scroll);
            e->setParent(getParent());
            e->setDirection(getDirection());
            e->setPosition(getParentIMatrix()*arm->getCenter());
            e->setGroup(getGroup());
            e->setMoveTarget(dir*50.0f);
          }
        }
      }
      if(f>300 && f<=450) {
        setDirection(getDirection()+(-m_dir*0.03f));
      }

      if(f==450) {
        m_frame = 0;
        ++m_action;
      }
    }

    void away()
    {
      m_vel+=0.03f;
      setPosition(getRelativePosition()+getDirection()*m_vel);

      if(m_frame==200) {
        SendKillMessage(0, get());
      }
    }

    void onUpdate(UpdateMessage& m)
    {
      ++m_frame;
      switch(m_action) {
      case 0: move(); break;
      case 1: aim();  break;
      case 2: fire(); break;
      case 3: away(); break;
      }
      if(m_scroll) {
        setPosition(getRelativePosition()+getParentIMatrix()*GetGlobalScroll());
      }
    }
  };

}
#endif
