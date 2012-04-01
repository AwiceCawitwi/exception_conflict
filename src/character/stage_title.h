#ifndef stage_title_h
#define stage_title_h

namespace exception {
namespace title {



  class Background : public Inherit2(HavePosition, Object)
  {
  typedef Inherit2(HavePosition, Object) Super;
  private:

    class LineTexture : public RefCounter
    {
    private:
      fbo_ptr m_fbo;

    public:
      LineTexture()
      {
        m_fbo = new ist::FrameBufferObject(128, 64);
        m_fbo->enable();
        glClearColor(0,0,0,0);
        glClear(GL_COLOR_BUFFER_BIT);
        draw();
        m_fbo->disable();
      }

      void draw()
      {
        ScreenMatrix sm(m_fbo->getWidth(), m_fbo->getHeight());

        glDisable(GL_LIGHTING);
        glColor4f(0,0,0,1);
        glLineWidth(3.0f);

        for(int j=0; j<5; ++j) {
          const float speed = 1.0f;
          vector2 pos;
          vector2 dir(speed, 0);

          glBegin(GL_LINE_STRIP);
          glVertex2fv(pos.v);
          for(int i=0; i<250; ++i) {
            pos+=dir;
            glVertex2fv(pos.v);
            if(GenRand() < 0.05f) {
              float f = float(GenRand());
              if(f<0.35f) {
                dir = vector2(0.0f, speed);
              }
              else {
                dir = vector2(speed, 0.0f);
              }
            }
          }
          glEnd();
        }

        glLineWidth(1.0f);
        glColor4f(1,1,1,1);
        glEnable(GL_LIGHTING);
      }

      void assign() { m_fbo->assign(); }
      void disassign() { m_fbo->disassign(); }
    };
    typedef intrusive_ptr<LineTexture> ltex_ptr;

    class Block : public Inherit5(HaveDirection, Box, HavePosition, Dummy, RefCounter)
    {
    typedef Inherit5(HaveDirection, Box, HavePosition, Dummy, RefCounter) Super;
    private:
      ltex_ptr m_lt;
      vector4 m_vel;

    public:
      Block(Deserializer& s) : Super(s)
      {
        s >> m_vel;
      }

      void serialize(Serializer& s) const
      {
        Super::serialize(s);
        s << m_vel;
      }

    public:
      Block()
      {
        setBox(box(vector4(30)));
      }

      void setLineTexture(ltex_ptr v) { m_lt=v; }
      void assign() { m_lt->assign(); }
      void disassign() { m_lt->disassign(); }

      struct invert_x_coord
      { vector2 operator()(const vector2& v) { return vector2(1.0f-v.x, v.y); } };

      struct invert_y_coord
      { vector2 operator()(const vector2& v) { return vector2(v.x, 1.0f-v.y); } };

      void initTexcoord(vector2 *tex, const vector4& ur, const vector4& bl)
      {
        tex[0] = vector2(1.0f, 1.0f);
        tex[1] = vector2(0.0f, 1.0f);
        tex[2] = vector2(0.0f, 0.0f);
        tex[3] = vector2(1.0f, 0.0f);
        if(GenRand()<0.5f) { std::transform(tex+0, tex+4, tex+0, invert_x_coord()); }
        if(GenRand()<0.5f) { std::transform(tex+0, tex+4, tex+0, invert_y_coord()); }

        tex[4] = vector2(0.0f, 0.0f);
        tex[5] = vector2(0.0f, 0.0f);
        tex[6] = vector2(0.0f, 0.0f);
        tex[7] = vector2(0.0f, 0.0f);
        if(GenRand()<0.5f) { std::transform(tex+4, tex+8, tex+4, invert_x_coord()); }
        if(GenRand()<0.5f) { std::transform(tex+4, tex+8, tex+4, invert_y_coord()); }

        tex[8] = vector2(1.0f, 1.0f);
        tex[9] = vector2(0.0f, 1.0f);
        tex[10] = vector2(0.0f, 0.0f);
        tex[11] = vector2(1.0f, 0.0f);
        if(GenRand()<0.5f) { std::transform(tex+8, tex+12, tex+8, invert_x_coord()); }
        if(GenRand()<0.5f) { std::transform(tex+8, tex+12, tex+8, invert_y_coord()); }

        tex[12] = vector2(1.0f, 1.0f);
        tex[13] = vector2(1.0f, 0.0f);
        tex[14] = vector2(0.0f, 0.0f);
        tex[15] = vector2(0.0f, 1.0f);
        if(GenRand()<0.5f) { std::transform(tex+12, tex+16, tex+12, invert_x_coord()); }
        if(GenRand()<0.5f) { std::transform(tex+12, tex+16, tex+12, invert_y_coord()); }

        tex[16] = vector2(1.0f, 1.0f);
        tex[17] = vector2(0.0f, 1.0f);
        tex[18] = vector2(0.0f, 0.0f);
        tex[19] = vector2(1.0f, 0.0f);
        if(GenRand()<0.5f) { std::transform(tex+16, tex+20, tex+16, invert_x_coord()); }
        if(GenRand()<0.5f) { std::transform(tex+16, tex+20, tex+16, invert_y_coord()); }

        tex[20] = vector2(1.0f, 1.0f);
        tex[21] = vector2(0.0f, 1.0f);
        tex[22] = vector2(0.0f, 0.0f);
        tex[23] = vector2(1.0f, 0.0f);
        if(GenRand()<0.5f) { std::transform(tex+20, tex+24, tex+20, invert_x_coord()); }
        if(GenRand()<0.5f) { std::transform(tex+20, tex+24, tex+20, invert_y_coord()); }
      }

      void setVel(const vector4& v) { m_vel=v; }
      const vector4& getVel() const { return m_vel; }

      void update()
      {
        vector4 pos = getPosition();
        pos+=m_vel;
        if(pos.z > 400.0f) {
          pos.z-=1800.0f;
        }
        setPosition(pos);
      }
    };

    typedef intrusive_ptr<Block> block_ptr;
    typedef std::vector<block_ptr> block_cont;

    ltex_ptr m_ltex[4];
    po_ptr m_hline;
    po_ptr m_glow;
    fbo_ptr m_fbo_tmp;
    fbo_ptr m_fbo_lines;
    fbo_ptr m_fbo_glow;
    fbo_ptr m_fbo_glow_b;

    ist::PerspectiveCamera m_cam;
    ist::Fog m_fog;
    ist::Material m_bgmat;
    ist::Light m_light;
    block_cont m_blocks;
    int m_frame;

  public:
    Background(Deserializer& s) : Super(s)
    {
      s >> m_cam >> m_fog >> m_bgmat;
      size_t size;
      s >> size;
      for(size_t i=0; i<size; ++i) {
        m_blocks.push_back(new Block(s));
      }
      s >> m_frame;
    }

    void serialize(Serializer& s) const
    {
      Super::serialize(s);
      s << m_cam << m_fog << m_bgmat;
      s << m_blocks.size();
      for(size_t i=0; i<m_blocks.size(); ++i) {
        m_blocks[i]->serialize(s);
      }
      s << m_frame;
    }

  public:
    Background() : m_frame(0)
    {
      m_cam.setPosition(vector4(100, -50, 400));
      m_cam.setTarget(vector4(0, 0, 0));
      m_cam.setFovy(60.0f);
      m_cam.setZFar(10000.0f);

      m_light.setPosition(vector4(1000.0f, 2000.0f, 1000.0f));

      m_fog.setColor(vector4(0.0f, 0.0f, 0.0f));
      m_fog.setNear(0.0f);
      m_fog.setFar(1500.0f);

      m_bgmat.setDiffuse(vector4(0.2f, 0.2f, 0.6f));
      m_bgmat.setSpecular(vector4(0.9f, 0.9f, 1.0f));
      m_bgmat.setShininess(30.0f);

      for(size_t i=0; i<400; ++i) {
        float d = 200.0f+100.0f*GenRand();
        vector4 dir = vector4(GenRand2(), GenRand2(), 0).normal();

        Block *b = new Block();
        b->setPosition(dir*d + vector4(0, 0, -1800.0f*GenRand()+400.0f));
        b->setDirection(-dir);
        b->setVel(vector4(0, 0, 4.0f));
        m_blocks.push_back(b);
      }
    }

    void drawRect(const vector2& ur, const vector2& bl)
    {
      glBegin(GL_QUADS);
      glTexCoord2f(0.0f, 1.0f);
      glVertex2f(bl.x, bl.y);
      glTexCoord2f(0.0f, 0.0f);
      glVertex2f(bl.x, ur.y);
      glTexCoord2f(1.0f, 0.0f);
      glVertex2f(ur.x, ur.y);
      glTexCoord2f(1.0f, 1.0f);
      glVertex2f(ur.x, bl.y);
      glEnd();
    }

    void draw()
    {
      m_light.enable();
      {
        ist::ProjectionMatrixSaver pm;
        ist::ModelviewMatrixSaver mm;
        m_cam.look();

        m_fog.enable();
        m_bgmat.assign();
        for(size_t i=0; i<m_blocks.size(); ++i) {
          m_blocks[i]->draw();
        }
        ist::Material().assign();
        m_fog.disable();
      }

      if(GetConfig()->shader && !GetConfig()->simplebg) {
        draw_gl20();
      }
      m_light.disable();
    }

    void draw_gl20()
    {
      if(!m_hline) {
        for(int i=0; i<4; ++i) {
          m_ltex[i] = new LineTexture();
        }
        for(size_t i=0; i<m_blocks.size(); ++i) {
          m_blocks[i]->setLineTexture(m_ltex[i%4]);
        }

        m_hline = new ist::ProgramObject();
        m_hline->attach(GetVertexShader("horde.vsh"));
        m_hline->attach(GetFragmentShader("horde.fsh"));
        m_hline->link();

        m_glow = new ist::ProgramObject();
        m_glow->attach(GetFragmentShader("title_glow.fsh"));
        m_glow->link();

        m_fbo_lines = new ist::FrameBufferObject(640, 480, GL_COLOR_BUFFER_BIT);
        m_fbo_tmp = new ist::FrameBufferObject(640, 480, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_fbo_glow = new ist::FrameBufferObject(256, 256);
        m_fbo_glow_b = new ist::FrameBufferObject(256, 256);
      }

      {
        ist::ProjectionMatrixSaver pm;
        ist::ModelviewMatrixSaver mm;

        m_cam.look();
        matrix44 icam;
        glGetFloatv(GL_MODELVIEW_MATRIX, icam.v);

        glDisable(GL_LIGHTING);

        // �a��`�� 
        m_fbo_tmp->enable();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_TEXTURE_2D);
        m_hline->enable();
        m_hline->setMatrix4fv("icam", 1, false, icam.invert().v);
        m_hline->setUniform1f("sz", float(m_frame));
        m_hline->setUniform4f("basecolor", 1.0f, 1.0f, 2.0f, 1.0f);
        for(size_t i=0; i<m_blocks.size(); ++i) {
          m_blocks[i]->assign();
          m_blocks[i]->draw();
          m_blocks[i]->disassign();
        }
        m_hline->disable();
        glDisable(GL_TEXTURE_2D);
        m_fbo_tmp->disable();

        // �t�B�[�h�o�b�N�u���[ 
        m_fbo_lines->enable();
        if(m_frame==1) {
          glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
          glClear(GL_COLOR_BUFFER_BIT);
        }
        {
          ScreenMatrix sm;

          glDisable(GL_DEPTH_TEST);
          glDepthMask(GL_FALSE);

          m_fbo_lines->assign();
          glEnable(GL_TEXTURE_2D);
          vector2 str(4.0f, 3.0f);
          glColor4f(1.0f, 1.0f, 1.0f, 0.4f);
          drawRect(vector2(640.0f, 480.0f)+str*3.0f, vector2(0.0f, 0.0f)-str*3.0f);
          drawRect(vector2(640.0f, 480.0f)+str*2.0f, vector2(0.0f, 0.0f)-str*2.0f);
          drawRect(vector2(640.0f, 480.0f)+str*1.0f, vector2(0.0f, 0.0f)-str*1.0f);
          glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
          glDisable(GL_TEXTURE_2D);
          m_fbo_lines->disassign();

          glColor4f(0.0f, 0.0f, 0.0f, 0.4f);
          drawRect(vector2(640,480), vector2(0,0));
          glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

          m_fbo_tmp->assign();
          glEnable(GL_TEXTURE_2D);
          glBlendFunc(GL_SRC_ALPHA, GL_ONE);
          glColor4f(1.0f, 1.0f, 1.0f, 0.6f);
          drawRect(vector2(640,480), vector2(0,0));
          glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
          glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
          glDisable(GL_TEXTURE_2D);
          m_fbo_tmp->disassign();

          glDepthMask(GL_TRUE);
          glEnable(GL_DEPTH_TEST);
        }
        m_fbo_lines->disable();

        glEnable(GL_LIGHTING);
      }

      {
        ScreenMatrix sm;
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_2D);

        // �e�N�X�`����������Ԃ����� 
        m_fbo_lines->assign();
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        drawRect(vector2(640,480), vector2(0,0));
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        m_fbo_lines->disassign();

        // �e�N�X�`����������Ԃ��ڂ���(�O���[�G�t�F�N�g��) 
        m_fbo_lines->assign();
        m_glow->enable();
        m_glow->setUniform1f("width", float(m_fbo_glow->getWidth()));
        m_glow->setUniform1f("height", float(m_fbo_glow->getHeight()));
        for(int i=0; i<2; ++i) {
          if(i!=0) {
            swap(m_fbo_glow, m_fbo_glow_b);
            m_fbo_glow_b->assign();
          }
          m_fbo_glow->enable();
          m_glow->setUniform1i("pass", i+1);
          DrawRect(vector2(640,480), vector2(0,0));
          m_fbo_glow->disable();
          m_fbo_glow->disassign();
        }
        m_glow->disable();


        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);

        // �O���[�G�t�F�N�g��`�� 
        m_fbo_glow->assign();
        glColor4f(0.3f, 0.3f, 1.0f, 1.0f);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        drawRect(vector2(640,480), vector2(0,0));
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1,1,1,1);
        m_fbo_glow->disassign();
        
      // �m�F�p 
      //  m_fbo_lines->assign();
      //  DrawRect(vector2(float(m_fbo_lines->getWidth()), float(m_fbo_lines->getHeight())), vector2(0.0f, 0.0f));
      //  m_fbo_lines->disassign();

        glDisable(GL_TEXTURE_2D);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
      }
    }

    void update()
    {
      m_cam.setPosition(vector4(100, -50, 400));
      m_cam.setTarget(vector4(0, 0, 0));

      for(size_t i=0; i<m_blocks.size(); ++i) {
        m_blocks[i]->update();
      }
      std::sort(m_blocks.begin(), m_blocks.end(), greater_z<block_ptr>());

      ++m_frame;
    }
  };


} // title
} // exception
#endif
