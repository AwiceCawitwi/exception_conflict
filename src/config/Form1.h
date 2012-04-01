#pragma once

#pragma warning(disable : 4996)

#include <stdio.h>
#include <vector>
#include <string>
#include <algorithm>
#include <boost/lexical_cast.hpp>

#include <windows.h>
#include <mmsystem.h>
#include <regstr.h>

#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"advapi32.lib")
#pragma comment(lib,"zlib.lib")

#include "../version.h"
#include "../system.h"
#include "../lib/ist/ist_sys.h"

namespace config {

  using namespace System;
  using namespace System::ComponentModel;
  using namespace System::Collections;
  using namespace System::Windows::Forms;
  using namespace System::Data;
  using namespace System::Drawing;

  using namespace System::Runtime::InteropServices;
  using namespace System::Diagnostics;
  using namespace System::IO;
  using namespace System::Text::RegularExpressions;


  std::string to_stlstr(String^ s)
  {
    char *p = (char*)(void*)(Marshal::StringToHGlobalAnsi(s));
    std::string tmp = p;
    Marshal::FreeHGlobal((System::IntPtr)p);
    return tmp;
  }

  namespace {
    const int g_keytable_length = 63;
    int g_keytable[g_keytable_length] = {
      273,274,275,276,
      97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,64,91,92,93,94,8,9,13,32,127,301,303,304,305,306,307,308
    };
  }

  int GetKeyValue(int i)
  {
    return g_keytable[i];
  }

  int GetKeyIndex(int v)
  {
    int i = 0;
    for(; i<g_keytable_length; ++i) {
      if(g_keytable[i]==v) {
        break;
      }
      if(i==g_keytable_length-1) {
        i = 0;
        break;
      }
    }
    return i;
  }


  /// <summary>
  /// Form1 �̊T�v
  ///
  /// �x��: ���̃N���X�̖��O��ύX����ꍇ�A���̃N���X���ˑ����邷�ׂĂ� .resx �t�@�C���Ɋ֘A�t����ꂽ
  ///          �}�l�[�W ���\�[�X �R���p�C�� �c�[���ɑ΂��� 'Resource File Name' �v���p�e�B��
  ///          �ύX����K�v������܂��B���̕ύX���s��Ȃ��ƁA
  ///          �f�U�C�i�ƁA���̃t�H�[���Ɋ֘A�t����ꂽ���[�J���C�Y�ς݃��\�[�X�Ƃ��A
  ///          ���������݂ɗ��p�ł��Ȃ��Ȃ�܂��B
  /// </summary>
  public ref class Form1 : public System::Windows::Forms::Form
  {
  public:
    Form1(void)
    {
      InitializeComponent();

      try {
        loadSystemInfo();
        loadDefaultKeyAssign();
        loadDefaultConfig_Low();
        loadConfig();
      }
      catch(...) {
      }
    }


    std::wstring getJoystickName(int index, const wchar_t *szRegKey)
    {
      HKEY hTopKey;
      HKEY hKey;
      DWORD regsize;
      LONG regresult;
      wchar_t regkey[256];
      wchar_t regvalue[256];
      wchar_t regname[256];

      swprintf(regkey, L"%s\\%s\\%s", REGSTR_PATH_JOYCONFIG, szRegKey, REGSTR_KEY_JOYCURR);
      hTopKey = HKEY_LOCAL_MACHINE;
      regresult = RegOpenKeyExW(hTopKey, regkey, 0, KEY_READ, &hKey);
      if(regresult!=ERROR_SUCCESS) {
        hTopKey = HKEY_CURRENT_USER;
        regresult = RegOpenKeyEx(hTopKey, regkey, 0, KEY_READ, &hKey);
      }
      if (regresult != ERROR_SUCCESS) {
        return NULL;
      }

      regsize = sizeof(regname);
      swprintf(regvalue, L"Joystick%d%s", index+1, REGSTR_VAL_JOYOEMNAME);
      regresult = RegQueryValueExW(hKey, regvalue, 0, 0, (LPBYTE)regname, &regsize);
      RegCloseKey(hKey);

      if(regresult!=ERROR_SUCCESS) {
        return NULL;
      }

      swprintf(regkey, L"%s\\%s", REGSTR_PATH_JOYOEM, regname);
      regresult = RegOpenKeyExW(hTopKey, regkey, 0, KEY_READ, &hKey);
      if(regresult!=ERROR_SUCCESS) {
        return NULL;
      }

      regsize = sizeof(regvalue);
      regresult = RegQueryValueExW(hKey, REGSTR_VAL_JOYOEMNAME, 0, 0, NULL, &regsize);

      std::wstring ret;
      if(regresult==ERROR_SUCCESS) {
        wchar_t *name = new wchar_t[regsize];
        regresult = RegQueryValueExW(hKey, REGSTR_VAL_JOYOEMNAME, 0, 0, (LPBYTE)name, &regsize);
        ret = name;
        delete[] name;
      }
      RegCloseKey(hKey);

      return ret;
    }

    void updateJoystickInfo()
    {
      int ix = cm_dir_x->SelectedIndex;
      int iy = cm_dir_y->SelectedIndex;
      cm_dir_x->Items->Clear();
      cm_dir_y->Items->Clear();
      cm_dir_x->Items->Add(L"����");
      cm_dir_y->Items->Add(L"����");

      UINT dev = cm_controller->SelectedIndex;
      JOYINFOEX joyinfo;
      JOYCAPS joycaps;
      joyinfo.dwSize = sizeof(joyinfo);
      joyinfo.dwFlags = JOY_RETURNALL;
      if(dev>=0 && dev<joyGetNumDevs() &&
        joyGetPosEx(dev, &joyinfo)==JOYERR_NOERROR &&
        joyGetDevCaps(dev, &joycaps, sizeof(joycaps))==JOYERR_NOERROR)
      {
        int flags[] = {JOYCAPS_HASZ, JOYCAPS_HASU, JOYCAPS_HASV, JOYCAPS_HASR};
        wchar_t names[][8] = {L"Z��", L"X��]", L"Y��]", L"Z��]"};
        for(int i=0; i<4; ++i) {
          if(joycaps.wCaps&flags[i]) {
            cm_dir_x->Items->Add(gcnew String(names[i]));
            cm_dir_y->Items->Add(gcnew String(names[i]));
          }
        }
      }

      if(ix < cm_dir_x->Items->Count) { cm_dir_x->SelectedIndex = ix; }
      if(iy < cm_dir_y->Items->Count) { cm_dir_y->SelectedIndex = iy; }
    }

    void loadSystemInfo()
    {
      cm_controller->Items->Clear();
      for(UINT i=0; i<joyGetNumDevs(); ++i) {
        JOYINFOEX joyinfo;
        JOYCAPS joycaps;
        joyinfo.dwSize = sizeof(joyinfo);
        joyinfo.dwFlags = JOY_RETURNALL;
        if(joyGetPosEx(i, &joyinfo)==JOYERR_NOERROR &&
           joyGetDevCaps(i, &joycaps, sizeof(joycaps))==JOYERR_NOERROR) {
          cm_controller->Items->Add(gcnew String(getJoystickName(i, joycaps.szRegKey).c_str()));
        }
      }
      if(cm_controller->Items->Count > 0) {
        cm_controller->SelectedIndex = 0;
      }

      ls_resolution->Items->Clear();
      const exception::DisplaySizeList dsl = exception::EnumDisplaySize();
      wchar_t buf[64];
      for(size_t i=0; i<dsl.size(); ++i) {
        swprintf(buf, 64, L"%dx%d", dsl[i].width, dsl[i].height);
        ls_resolution->Items->Add(gcnew System::String(buf));
      }
    }

    void loadDefaultKeyAssign()
    {
      cm_key_laser->SelectedItem = L"Z";
      cm_key_ray->SelectedItem = L"X";
      cm_key_direction->SelectedItem = L"V";
      cm_key_catapult->SelectedItem = L"C";
      cm_key_lrot->SelectedItem = L"A";
      cm_key_rrot->SelectedItem = L"S";
      cm_key_up->SelectedItem = L"��";
      cm_key_down->SelectedItem = L"��";
      cm_key_right->SelectedItem = L"��";
      cm_key_left->SelectedItem = L"��";

      cm_pad_laser->SelectedIndex = 0;
      cm_pad_ray->SelectedIndex = 1;
      cm_pad_direction->SelectedIndex = 2;
      cm_pad_catapult->SelectedIndex = 3;
      cm_pad_lrot->SelectedIndex = 4;
      cm_pad_rrot->SelectedIndex = 6;
      cm_pad_menu->SelectedIndex = 9;
      ck_hat->Checked = false;

      ed_threshold1->Text = L"15000";
      ed_threshold2->Text = L"5000";
      cm_dir_x->SelectedIndex = 0;
      cm_dir_y->SelectedIndex = 0;
    }

    void loadDefaultConfig_Common()
    {
      pn_color_show->BackColor = System::Drawing::Color::FromArgb(0xff, 0xcc, 0xcc, 0xcc);
      ck_fullscreen->Checked = false;
      ck_vsync->Checked = false;
      ck_frameskip->Checked = true;
      ck_30fps->Checked = false;
      ck_exlight->Checked = true;
      sl_bloom->Value = 10;
      ck_update->Checked = true;

      {
        SYSTEM_INFO info;
        GetSystemInfo(&info);
        num_thread->Value = std::max<int>(1, std::min<int>(8, int(info.dwNumberOfProcessors)));
      }
      ed_port->Text = L"10040";

      ck_bgm->Checked = true;
      ck_se->Checked = true;
      sl_bgm_volume->Value = 6;
      sl_se_volume->Value = 6;
    }

    void loadDefaultConfig_Low()
    {
      ls_resolution->SelectedItem = "640x480";
      ck_vertex_buffer->Checked = false;
      ck_shader->Checked = false;
      ck_simplebg->Checked = false;
      ck_noblur->Checked = true;
      ck_show_fps->Checked = true;
      ck_show_obj->Checked = true;

      loadDefaultConfig_Common();
    }

    void loadDefaultConfig_High()
    {
      ls_resolution->SelectedItem = "1024x768";
      ck_vertex_buffer->Checked = true;
      ck_shader->Checked = true;
      ck_simplebg->Checked = false;
      ck_noblur->Checked = false;
      ck_show_fps->Checked = true;
      ck_show_obj->Checked = true;

      loadDefaultConfig_Common();
    }


    void save()
    {
      FILE *f = fopen("config", "wb");
      if(!f) {
        return;
      }
      fprintf(f, "color=%.2f,%.2f,%.2f\n",
        float(pn_color_show->BackColor.R)/255.0f,
        float(pn_color_show->BackColor.G)/255.0f,
        float(pn_color_show->BackColor.B)/255.0f);
      fprintf(f, "scorename=%s\n",    to_stlstr(ed_name->Text).c_str());
      fprintf(f, "resolution=%s\n",   to_stlstr(ls_resolution->SelectedItem->ToString()).c_str());
      fprintf(f, "fullscreen=%d\n",   ck_fullscreen->Checked);
      fprintf(f, "vsync=%d\n",        ck_vsync->Checked);
      fprintf(f, "frameskip=%d\n",    ck_frameskip->Checked);
      fprintf(f, "vertex_buffer=%d\n",ck_vertex_buffer->Checked);
      fprintf(f, "shader=%d\n",       ck_shader->Checked);
      fprintf(f, "simplebg=%d\n",     ck_simplebg->Checked);
      fprintf(f, "noblur=%d\n",       ck_noblur->Checked);
      fprintf(f, "30fps=%d\n",        ck_30fps->Checked);
      fprintf(f, "exlight=%d\n",      ck_exlight->Checked);
      fprintf(f, "bloom=%f\n",        float(sl_bloom->Value)*0.1f);

      fprintf(f, "update=%d\n",       ck_update->Checked);
      fprintf(f, "show_fps=%d\n",     ck_show_fps->Checked);
      fprintf(f, "show_obj=%d\n",     ck_show_obj->Checked);
      fprintf(f, "thread=%d\n",       int(num_thread->Value));
      fprintf(f, "server_port=%s\n",  ed_port->Text);

      fprintf(f, "bgm=%d\n",        ck_bgm->Checked);
      fprintf(f, "se=%d\n",         ck_se->Checked);
      fprintf(f, "bgm_volume=%d\n", sl_bgm_volume->Value*16);
      fprintf(f, "se_volume=%d\n",  sl_se_volume->Value*16);

      fprintf(f, "key0=%d\n", GetKeyValue(cm_key_laser->SelectedIndex));
      fprintf(f, "key1=%d\n", GetKeyValue(cm_key_ray->SelectedIndex));
      fprintf(f, "key2=%d\n", GetKeyValue(cm_key_direction->SelectedIndex));
      fprintf(f, "key3=%d\n", GetKeyValue(cm_key_catapult->SelectedIndex));
      fprintf(f, "key4=%d\n", GetKeyValue(cm_key_lrot->SelectedIndex));
      fprintf(f, "key5=%d\n", GetKeyValue(cm_key_rrot->SelectedIndex));
      fprintf(f, "key6=%d\n", GetKeyValue(cm_key_up->SelectedIndex));
      fprintf(f, "key7=%d\n", GetKeyValue(cm_key_down->SelectedIndex));
      fprintf(f, "key8=%d\n", GetKeyValue(cm_key_right->SelectedIndex));
      fprintf(f, "key9=%d\n", GetKeyValue(cm_key_left->SelectedIndex));

      fprintf(f, "pad0=%d\n", cm_pad_laser->SelectedIndex);
      fprintf(f, "pad1=%d\n", cm_pad_ray->SelectedIndex);
      fprintf(f, "pad2=%d\n", cm_pad_direction->SelectedIndex);
      fprintf(f, "pad3=%d\n", cm_pad_catapult->SelectedIndex);
      fprintf(f, "pad4=%d\n", cm_pad_lrot->SelectedIndex);
      fprintf(f, "pad5=%d\n", cm_pad_rrot->SelectedIndex);
      fprintf(f, "pad6=%d\n", cm_pad_menu->SelectedIndex);

      fprintf(f, "controller=%d\n", cm_controller->SelectedIndex);
      fprintf(f, "daxis1=%d\n",     cm_dir_x->SelectedIndex+1);
      fprintf(f, "daxis2=%d\n",     cm_dir_y->SelectedIndex+1);
      fprintf(f, "threshold1=%s\n", ed_threshold1->Text);
      fprintf(f, "threshold2=%s\n", ed_threshold2->Text);
      fprintf(f, "hat=%d\n",        ck_hat->Checked);
      fclose(f);
    }

    bool loadConfig()
    {
      FILE *in = fopen("config", "rb");
      if(!in) {
        return false;
      }

      char l[256];
      char buf[128];
      int i;
      float f, r,g,b;
      while(fgets(l, 256, in)) {
        if (sscanf(l, "color=%f,%f,%f", &r,&g,&b)==3) {
          pn_color_show->BackColor = System::Drawing::Color::FromArgb(255, int(r*255.0f), int(g*255.0f), int(b*255.0f));
        }
        else if(sscanf(l, "scorename=%[^\n]", buf)){ ed_name->Text = gcnew String(buf); }
        else if(sscanf(l, "resolution=%s", buf))   { ls_resolution->SelectedItem = gcnew String(buf); }
        else if(sscanf(l, "fullscreen=%d", &i))    { ck_fullscreen->Checked = i!=0; }
        else if(sscanf(l, "vsync=%d", &i))         { ck_vsync->Checked = i!=0; }
        else if(sscanf(l, "frameskip=%d", &i))     { ck_frameskip->Checked = i!=0; }
        else if(sscanf(l, "vertex_buffer=%d", &i)) { ck_vertex_buffer->Checked = i!=0; }
        else if(sscanf(l, "shader=%d", &i))        { ck_shader->Checked = i!=0; }
        else if(sscanf(l, "simplebg=%d", &i))      { ck_simplebg->Checked = i!=0; }
        else if(sscanf(l, "noblur=%d", &i))        { ck_noblur->Checked = i!=0; }
        else if(sscanf(l, "30fps=%d", &i))         { ck_30fps->Checked = i!=0; }
        else if(sscanf(l, "exlight=%d", &i))       { ck_exlight->Checked = i!=0; }
        else if(sscanf(l, "bloom=%f", &f))         { sl_bloom->Value = int(f*10.0f); }

        else if(sscanf(l, "update=%d", &i))        { ck_update->Checked = i!=0; }
        else if(sscanf(l, "show_fps=%d", &i))      { ck_show_fps->Checked = i!=0; }
        else if(sscanf(l, "show_obj=%d", &i))      { ck_show_obj->Checked = i!=0; }
        else if(sscanf(l, "thread=%d", &i))        { num_thread->Value = std::max<int>(1, std::min<int>(8, i)); }
        else if(sscanf(l, "server_port=%d", &i))   { ed_port->Text = gcnew String(boost::lexical_cast<std::string>(i).c_str()); }

        else if(sscanf(l, "bgm=%d", &i))           { ck_bgm->Checked = i!=0; }
        else if(sscanf(l, "se=%d", &i))            { ck_se->Checked = i!=0; }
        else if(sscanf(l, "bgm_volume=%d", &i))    { sl_bgm_volume->Value = std::max<int>(0, std::min<int>(8, i/16)); }
        else if(sscanf(l, "se_volume=%d", &i))     { sl_se_volume->Value = std::max<int>(0, std::min<int>(8, i/16)); }

        else if(sscanf(l, "key0=%d", &i)) { cm_key_laser->SelectedIndex = GetKeyIndex(i); }
        else if(sscanf(l, "key1=%d", &i)) { cm_key_ray->SelectedIndex = GetKeyIndex(i); }
        else if(sscanf(l, "key2=%d", &i)) { cm_key_direction->SelectedIndex = GetKeyIndex(i); }
        else if(sscanf(l, "key3=%d", &i)) { cm_key_catapult->SelectedIndex = GetKeyIndex(i); }
        else if(sscanf(l, "key4=%d", &i)) { cm_key_lrot->SelectedIndex = GetKeyIndex(i); }
        else if(sscanf(l, "key5=%d", &i)) { cm_key_rrot->SelectedIndex = GetKeyIndex(i); }
        else if(sscanf(l, "key6=%d", &i)) { cm_key_up->SelectedIndex = GetKeyIndex(i); }
        else if(sscanf(l, "key7=%d", &i)) { cm_key_down->SelectedIndex = GetKeyIndex(i); }
        else if(sscanf(l, "key8=%d", &i)) { cm_key_right->SelectedIndex = GetKeyIndex(i); }
        else if(sscanf(l, "key9=%d", &i)) { cm_key_left->SelectedIndex = GetKeyIndex(i); }

        else if(sscanf(l, "pad0=%d", &i)) { cm_pad_laser->SelectedIndex = i; }
        else if(sscanf(l, "pad1=%d", &i)) { cm_pad_ray->SelectedIndex = i; }
        else if(sscanf(l, "pad2=%d", &i)) { cm_pad_direction->SelectedIndex = i; }
        else if(sscanf(l, "pad3=%d", &i)) { cm_pad_catapult->SelectedIndex = i; }
        else if(sscanf(l, "pad4=%d", &i)) { cm_pad_lrot->SelectedIndex = i; }
        else if(sscanf(l, "pad5=%d", &i)) { cm_pad_rrot->SelectedIndex = i; }
        else if(sscanf(l, "pad6=%d", &i)) { cm_pad_menu->SelectedIndex = i; }

        else if(sscanf(l, "controller=%d", &i)) { if(i<cm_controller->Items->Count) { cm_controller->SelectedIndex = i; } }
        else if(sscanf(l, "daxis1=%d", &i))     { if(i>0) { cm_dir_x->SelectedIndex = i-1; } }
        else if(sscanf(l, "daxis2=%d", &i))     { if(i>0) { cm_dir_y->SelectedIndex = i-1; } }
        else if(sscanf(l, "threshold1=%d", &i)) { ed_threshold1->Text = gcnew String(boost::lexical_cast<std::string>(i).c_str()); }
        else if(sscanf(l, "threshold2=%d", &i)) { ed_threshold2->Text = gcnew String(boost::lexical_cast<std::string>(i).c_str()); }
        else if(sscanf(l, "hat=%d", &i))        { ck_hat->Checked = i!=0; }
      }
      fclose(in);
      return true;
    }

    void setDescription(String^ v) { ed_desc->Text=v; }
    String^ getDescription() { return ed_desc->Text; }



  protected:
    /// <summary>
    /// �g�p���̃��\�[�X�����ׂăN���[���A�b�v���܂��B
    /// </summary>
    ~Form1()
    {
      if (components)
      {
        delete components;
      }
    }
  private: System::Windows::Forms::CheckBox^  ck_vsync;
  private: System::Windows::Forms::CheckBox^  ck_fullscreen;
  private: System::Windows::Forms::ListBox^  ls_resolution;
  private: System::Windows::Forms::CheckBox^  ck_shader;
  private: System::Windows::Forms::CheckBox^  ck_simplebg;
  private: System::Windows::Forms::GroupBox^  gp_resolution;
  private: System::Windows::Forms::NumericUpDown^  num_thread;
  private: System::Windows::Forms::Label^  lb_thread;
  private: System::Windows::Forms::TextBox^  ed_name;
  private: System::Windows::Forms::TextBox^  ed_desc;
  private: System::Windows::Forms::GroupBox^  gp_draw;
  private: System::Windows::Forms::CheckBox^  ck_noblur;
private: System::Windows::Forms::CheckBox^  ck_vertex_buffer;
  private: System::Windows::Forms::GroupBox^  gp_other;
  private: System::Windows::Forms::Label^  ls_name;
  private: System::Windows::Forms::Button^  bu_invoke;
  private: System::Windows::Forms::CheckBox^  ck_show_fps;
  private: System::Windows::Forms::CheckBox^  ck_show_obj;
private: System::Windows::Forms::Button^  bu_default_high;
private: System::Windows::Forms::GroupBox^  gp_input;
private: System::Windows::Forms::Label^  lb_cat;
private: System::Windows::Forms::Label^  lb_dir;
private: System::Windows::Forms::Label^  lb_ray;
private: System::Windows::Forms::Label^  lb_laser;
private: System::Windows::Forms::ComboBox^  cm_pad_laser;
private: System::Windows::Forms::ComboBox^  cm_pad_catapult;
private: System::Windows::Forms::ComboBox^  cm_pad_direction;



private: System::Windows::Forms::ComboBox^  cm_pad_ray;

private: System::Windows::Forms::GroupBox^  gp_sound;
private: System::Windows::Forms::CheckBox^  ck_se;
private: System::Windows::Forms::CheckBox^  ck_bgm;
private: System::Windows::Forms::TrackBar^  sl_bgm_volume;
private: System::Windows::Forms::TrackBar^  sl_se_volume;
private: System::Windows::Forms::ComboBox^  cm_pad_menu;

private: System::Windows::Forms::Label^  lb_menu;
private: System::Windows::Forms::Label^  lb_menu_esc;
private: System::Windows::Forms::ComboBox^  cm_pad_lrot;

private: System::Windows::Forms::Label^  label2;
private: System::Windows::Forms::Label^  label1;
private: System::Windows::Forms::ComboBox^  cm_pad_rrot;

private: System::Windows::Forms::ComboBox^  cm_key_laser;
private: System::Windows::Forms::ComboBox^  cm_key_rrot;


private: System::Windows::Forms::ComboBox^  cm_key_lrot;

private: System::Windows::Forms::ComboBox^  cm_key_catapult;
private: System::Windows::Forms::ComboBox^  cm_key_direction;


private: System::Windows::Forms::ComboBox^  cm_key_ray;

private: System::Windows::Forms::Button^  bu_default_low;


private: System::Windows::Forms::Label^  lb_threshold;
private: System::Windows::Forms::ComboBox^  cm_key_left;

private: System::Windows::Forms::Label^  label6;
private: System::Windows::Forms::ComboBox^  cm_key_right;

private: System::Windows::Forms::Label^  label5;
private: System::Windows::Forms::ComboBox^  cm_key_up;

private: System::Windows::Forms::Label^  label4;
private: System::Windows::Forms::ComboBox^  cm_key_down;

private: System::Windows::Forms::Label^  label3;
private: System::Windows::Forms::Label^  lb_port;
private: System::Windows::Forms::TextBox^  ed_port;


private: System::Windows::Forms::CheckBox^  ck_30fps;

private: System::Windows::Forms::ComboBox^  cm_controller;



private: System::Windows::Forms::GroupBox^  groupBox1;

private: System::Windows::Forms::GroupBox^  groupBox3;





private: System::Windows::Forms::Label^  label9;
private: System::Windows::Forms::TextBox^  ed_threshold2;

private: System::Windows::Forms::TextBox^  ed_threshold1;


private: System::Windows::Forms::Label^  label8;
private: System::Windows::Forms::Label^  label7;
private: System::Windows::Forms::ComboBox^  cm_dir_y;

private: System::Windows::Forms::ComboBox^  cm_dir_x;
private: System::Windows::Forms::CheckBox^  ck_exlight;

private: System::Windows::Forms::TrackBar^  sl_bloom;
private: System::Windows::Forms::Label^  lb_bloom;
private: System::Windows::Forms::CheckBox^  ck_hat;
private: System::Windows::Forms::CheckBox^  ck_update;
private: System::Windows::Forms::CheckBox^  ck_frameskip;
private: System::Windows::Forms::ColorDialog^  colorDialog1;
private: System::Windows::Forms::Button^  bu_color;
private: System::Windows::Forms::Panel^  pn_color_show;
private: System::Windows::Forms::Button^  bu_reset_keyassign;











  protected: 

  private:
    /// <summary>
    /// �K�v�ȃf�U�C�i�ϐ��ł��B
    /// </summary>
    System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
    /// <summary>
    /// �f�U�C�i �T�|�[�g�ɕK�v�ȃ��\�b�h�ł��B���̃��\�b�h�̓��e��
    /// �R�[�h �G�f�B�^�ŕύX���Ȃ��ł��������B
    /// </summary>
    void InitializeComponent(void)
    {
      System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(Form1::typeid));
      this->ck_vsync = (gcnew System::Windows::Forms::CheckBox());
      this->ck_fullscreen = (gcnew System::Windows::Forms::CheckBox());
      this->ls_resolution = (gcnew System::Windows::Forms::ListBox());
      this->ck_shader = (gcnew System::Windows::Forms::CheckBox());
      this->ck_simplebg = (gcnew System::Windows::Forms::CheckBox());
      this->gp_resolution = (gcnew System::Windows::Forms::GroupBox());
      this->num_thread = (gcnew System::Windows::Forms::NumericUpDown());
      this->lb_thread = (gcnew System::Windows::Forms::Label());
      this->ed_name = (gcnew System::Windows::Forms::TextBox());
      this->ed_desc = (gcnew System::Windows::Forms::TextBox());
      this->gp_draw = (gcnew System::Windows::Forms::GroupBox());
      this->ck_frameskip = (gcnew System::Windows::Forms::CheckBox());
      this->lb_bloom = (gcnew System::Windows::Forms::Label());
      this->ck_exlight = (gcnew System::Windows::Forms::CheckBox());
      this->sl_bloom = (gcnew System::Windows::Forms::TrackBar());
      this->ck_30fps = (gcnew System::Windows::Forms::CheckBox());
      this->ck_noblur = (gcnew System::Windows::Forms::CheckBox());
      this->ck_vertex_buffer = (gcnew System::Windows::Forms::CheckBox());
      this->gp_other = (gcnew System::Windows::Forms::GroupBox());
      this->pn_color_show = (gcnew System::Windows::Forms::Panel());
      this->bu_color = (gcnew System::Windows::Forms::Button());
      this->ck_update = (gcnew System::Windows::Forms::CheckBox());
      this->lb_port = (gcnew System::Windows::Forms::Label());
      this->ed_port = (gcnew System::Windows::Forms::TextBox());
      this->ck_show_obj = (gcnew System::Windows::Forms::CheckBox());
      this->ck_show_fps = (gcnew System::Windows::Forms::CheckBox());
      this->ls_name = (gcnew System::Windows::Forms::Label());
      this->bu_invoke = (gcnew System::Windows::Forms::Button());
      this->bu_default_high = (gcnew System::Windows::Forms::Button());
      this->gp_input = (gcnew System::Windows::Forms::GroupBox());
      this->cm_key_left = (gcnew System::Windows::Forms::ComboBox());
      this->label6 = (gcnew System::Windows::Forms::Label());
      this->cm_key_right = (gcnew System::Windows::Forms::ComboBox());
      this->label5 = (gcnew System::Windows::Forms::Label());
      this->cm_key_up = (gcnew System::Windows::Forms::ComboBox());
      this->label4 = (gcnew System::Windows::Forms::Label());
      this->cm_key_down = (gcnew System::Windows::Forms::ComboBox());
      this->label3 = (gcnew System::Windows::Forms::Label());
      this->cm_key_rrot = (gcnew System::Windows::Forms::ComboBox());
      this->cm_key_lrot = (gcnew System::Windows::Forms::ComboBox());
      this->cm_key_catapult = (gcnew System::Windows::Forms::ComboBox());
      this->cm_key_direction = (gcnew System::Windows::Forms::ComboBox());
      this->cm_key_ray = (gcnew System::Windows::Forms::ComboBox());
      this->cm_key_laser = (gcnew System::Windows::Forms::ComboBox());
      this->label2 = (gcnew System::Windows::Forms::Label());
      this->label1 = (gcnew System::Windows::Forms::Label());
      this->cm_pad_rrot = (gcnew System::Windows::Forms::ComboBox());
      this->cm_pad_lrot = (gcnew System::Windows::Forms::ComboBox());
      this->lb_menu_esc = (gcnew System::Windows::Forms::Label());
      this->cm_pad_menu = (gcnew System::Windows::Forms::ComboBox());
      this->lb_menu = (gcnew System::Windows::Forms::Label());
      this->cm_pad_catapult = (gcnew System::Windows::Forms::ComboBox());
      this->cm_pad_direction = (gcnew System::Windows::Forms::ComboBox());
      this->cm_pad_ray = (gcnew System::Windows::Forms::ComboBox());
      this->cm_pad_laser = (gcnew System::Windows::Forms::ComboBox());
      this->lb_dir = (gcnew System::Windows::Forms::Label());
      this->lb_cat = (gcnew System::Windows::Forms::Label());
      this->lb_ray = (gcnew System::Windows::Forms::Label());
      this->lb_laser = (gcnew System::Windows::Forms::Label());
      this->lb_threshold = (gcnew System::Windows::Forms::Label());
      this->gp_sound = (gcnew System::Windows::Forms::GroupBox());
      this->sl_se_volume = (gcnew System::Windows::Forms::TrackBar());
      this->sl_bgm_volume = (gcnew System::Windows::Forms::TrackBar());
      this->ck_se = (gcnew System::Windows::Forms::CheckBox());
      this->ck_bgm = (gcnew System::Windows::Forms::CheckBox());
      this->bu_default_low = (gcnew System::Windows::Forms::Button());
      this->cm_controller = (gcnew System::Windows::Forms::ComboBox());
      this->groupBox1 = (gcnew System::Windows::Forms::GroupBox());
      this->ck_hat = (gcnew System::Windows::Forms::CheckBox());
      this->ed_threshold2 = (gcnew System::Windows::Forms::TextBox());
      this->ed_threshold1 = (gcnew System::Windows::Forms::TextBox());
      this->label9 = (gcnew System::Windows::Forms::Label());
      this->groupBox3 = (gcnew System::Windows::Forms::GroupBox());
      this->label8 = (gcnew System::Windows::Forms::Label());
      this->label7 = (gcnew System::Windows::Forms::Label());
      this->cm_dir_y = (gcnew System::Windows::Forms::ComboBox());
      this->cm_dir_x = (gcnew System::Windows::Forms::ComboBox());
      this->colorDialog1 = (gcnew System::Windows::Forms::ColorDialog());
      this->bu_reset_keyassign = (gcnew System::Windows::Forms::Button());
      this->gp_resolution->SuspendLayout();
      (cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->num_thread))->BeginInit();
      this->gp_draw->SuspendLayout();
      (cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->sl_bloom))->BeginInit();
      this->gp_other->SuspendLayout();
      this->gp_input->SuspendLayout();
      this->gp_sound->SuspendLayout();
      (cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->sl_se_volume))->BeginInit();
      (cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->sl_bgm_volume))->BeginInit();
      this->groupBox1->SuspendLayout();
      this->groupBox3->SuspendLayout();
      this->SuspendLayout();
      // 
      // ck_vsync
      // 
      this->ck_vsync->AutoSize = true;
      this->ck_vsync->Location = System::Drawing::Point(6, 18);
      this->ck_vsync->Name = L"ck_vsync";
      this->ck_vsync->Size = System::Drawing::Size(57, 16);
      this->ck_vsync->TabIndex = 1;
      this->ck_vsync->Text = L"VSync";
      this->ck_vsync->UseVisualStyleBackColor = true;
      this->ck_vsync->MouseEnter += gcnew System::EventHandler(this, &Form1::ck_vsync_MouseEnter);
      // 
      // ck_fullscreen
      // 
      this->ck_fullscreen->AutoSize = true;
      this->ck_fullscreen->Location = System::Drawing::Point(4, 100);
      this->ck_fullscreen->Name = L"ck_fullscreen";
      this->ck_fullscreen->Size = System::Drawing::Size(85, 16);
      this->ck_fullscreen->TabIndex = 2;
      this->ck_fullscreen->Text = L"�t���X�N���[��";
      this->ck_fullscreen->UseVisualStyleBackColor = true;
      this->ck_fullscreen->MouseEnter += gcnew System::EventHandler(this, &Form1::ck_fullscreen_MouseEnter);
      // 
      // ls_resolution
      // 
      this->ls_resolution->FormattingEnabled = true;
      this->ls_resolution->ItemHeight = 12;
      this->ls_resolution->Location = System::Drawing::Point(6, 18);
      this->ls_resolution->Name = L"ls_resolution";
      this->ls_resolution->Size = System::Drawing::Size(83, 76);
      this->ls_resolution->TabIndex = 1;
      this->ls_resolution->MouseEnter += gcnew System::EventHandler(this, &Form1::ls_resolution_MouseEnter);
      // 
      // ck_shader
      // 
      this->ck_shader->AutoSize = true;
      this->ck_shader->Location = System::Drawing::Point(116, 18);
      this->ck_shader->Name = L"ck_shader";
      this->ck_shader->Size = System::Drawing::Size(60, 16);
      this->ck_shader->TabIndex = 7;
      this->ck_shader->Text = L"�V�F�[�_";
      this->ck_shader->UseVisualStyleBackColor = true;
      this->ck_shader->MouseEnter += gcnew System::EventHandler(this, &Form1::ck_shader_MouseEnter);
      // 
      // ck_simplebg
      // 
      this->ck_simplebg->AutoSize = true;
      this->ck_simplebg->Location = System::Drawing::Point(116, 40);
      this->ck_simplebg->Name = L"ck_simplebg";
      this->ck_simplebg->Size = System::Drawing::Size(84, 16);
      this->ck_simplebg->TabIndex = 8;
      this->ck_simplebg->Text = L"�w�i�ȗ���";
      this->ck_simplebg->UseVisualStyleBackColor = true;
      this->ck_simplebg->MouseEnter += gcnew System::EventHandler(this, &Form1::ck_simplebg_MouseEnter);
      // 
      // gp_resolution
      // 
      this->gp_resolution->Controls->Add(this->ck_fullscreen);
      this->gp_resolution->Controls->Add(this->ls_resolution);
      this->gp_resolution->Location = System::Drawing::Point(12, 5);
      this->gp_resolution->Name = L"gp_resolution";
      this->gp_resolution->Size = System::Drawing::Size(99, 184);
      this->gp_resolution->TabIndex = 0;
      this->gp_resolution->TabStop = false;
      this->gp_resolution->Text = L"��ʉ𑜓x";
      // 
      // num_thread
      // 
      this->num_thread->Location = System::Drawing::Point(65, 135);
      this->num_thread->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) {8, 0, 0, 0});
      this->num_thread->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) {1, 0, 0, 0});
      this->num_thread->Name = L"num_thread";
      this->num_thread->Size = System::Drawing::Size(37, 19);
      this->num_thread->TabIndex = 5;
      this->num_thread->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) {2, 0, 0, 0});
      // 
      // lb_thread
      // 
      this->lb_thread->AutoSize = true;
      this->lb_thread->Location = System::Drawing::Point(6, 138);
      this->lb_thread->Name = L"lb_thread";
      this->lb_thread->Size = System::Drawing::Size(53, 12);
      this->lb_thread->TabIndex = 0;
      this->lb_thread->Text = L"���񏈗�";
      this->lb_thread->MouseEnter += gcnew System::EventHandler(this, &Form1::lb_thread_MouseEnter);
      // 
      // ed_name
      // 
      this->ed_name->Location = System::Drawing::Point(43, 15);
      this->ed_name->MaxLength = 10;
      this->ed_name->Name = L"ed_name";
      this->ed_name->Size = System::Drawing::Size(69, 19);
      this->ed_name->TabIndex = 2;
      this->ed_name->Text = L"nullpo";
      this->ed_name->MouseEnter += gcnew System::EventHandler(this, &Form1::ed_name_MouseEnter);
      // 
      // ed_desc
      // 
      this->ed_desc->Location = System::Drawing::Point(13, 361);
      this->ed_desc->Multiline = true;
      this->ed_desc->Name = L"ed_desc";
      this->ed_desc->ReadOnly = true;
      this->ed_desc->Size = System::Drawing::Size(610, 54);
      this->ed_desc->TabIndex = 60;
      this->ed_desc->TabStop = false;
      // 
      // gp_draw
      // 
      this->gp_draw->Controls->Add(this->ck_frameskip);
      this->gp_draw->Controls->Add(this->lb_bloom);
      this->gp_draw->Controls->Add(this->ck_exlight);
      this->gp_draw->Controls->Add(this->sl_bloom);
      this->gp_draw->Controls->Add(this->ck_30fps);
      this->gp_draw->Controls->Add(this->ck_noblur);
      this->gp_draw->Controls->Add(this->ck_vertex_buffer);
      this->gp_draw->Controls->Add(this->ck_vsync);
      this->gp_draw->Controls->Add(this->ck_shader);
      this->gp_draw->Controls->Add(this->ck_simplebg);
      this->gp_draw->Location = System::Drawing::Point(117, 4);
      this->gp_draw->Name = L"gp_draw";
      this->gp_draw->Size = System::Drawing::Size(212, 185);
      this->gp_draw->TabIndex = 10;
      this->gp_draw->TabStop = false;
      this->gp_draw->Text = L"�`��";
      // 
      // ck_frameskip
      // 
      this->ck_frameskip->AutoSize = true;
      this->ck_frameskip->Location = System::Drawing::Point(6, 40);
      this->ck_frameskip->Name = L"ck_frameskip";
      this->ck_frameskip->Size = System::Drawing::Size(96, 16);
      this->ck_frameskip->TabIndex = 2;
      this->ck_frameskip->Text = L"�t���[���X�L�b�v";
      this->ck_frameskip->UseVisualStyleBackColor = true;
      this->ck_frameskip->MouseEnter += gcnew System::EventHandler(this, &Form1::ck_frameskip_MouseEnter);
      // 
      // lb_bloom
      // 
      this->lb_bloom->AutoSize = true;
      this->lb_bloom->Location = System::Drawing::Point(114, 62);
      this->lb_bloom->Name = L"lb_bloom";
      this->lb_bloom->Size = System::Drawing::Size(44, 12);
      this->lb_bloom->TabIndex = 8;
      this->lb_bloom->Text = L"�u���[��";
      this->lb_bloom->MouseEnter += gcnew System::EventHandler(this, &Form1::lb_bloom_MouseEnter);
      // 
      // ck_exlight
      // 
      this->ck_exlight->AutoSize = true;
      this->ck_exlight->Location = System::Drawing::Point(6, 84);
      this->ck_exlight->Name = L"ck_exlight";
      this->ck_exlight->Size = System::Drawing::Size(72, 16);
      this->ck_exlight->TabIndex = 4;
      this->ck_exlight->Text = L"�ǉ�����";
      this->ck_exlight->UseVisualStyleBackColor = true;
      this->ck_exlight->MouseEnter += gcnew System::EventHandler(this, &Form1::ck_exlight_MouseEnter);
      // 
      // sl_bloom
      // 
      this->sl_bloom->LargeChange = 2;
      this->sl_bloom->Location = System::Drawing::Point(116, 77);
      this->sl_bloom->Name = L"sl_bloom";
      this->sl_bloom->Size = System::Drawing::Size(90, 45);
      this->sl_bloom->TabIndex = 9;
      this->sl_bloom->MouseEnter += gcnew System::EventHandler(this, &Form1::lb_bloom_MouseEnter);
      // 
      // ck_30fps
      // 
      this->ck_30fps->AutoSize = true;
      this->ck_30fps->Location = System::Drawing::Point(6, 128);
      this->ck_30fps->Name = L"ck_30fps";
      this->ck_30fps->Size = System::Drawing::Size(57, 16);
      this->ck_30fps->TabIndex = 6;
      this->ck_30fps->Text = L"30FPS";
      this->ck_30fps->UseVisualStyleBackColor = true;
      this->ck_30fps->MouseEnter += gcnew System::EventHandler(this, &Form1::ck_30fps_MouseEnter);
      // 
      // ck_noblur
      // 
      this->ck_noblur->AutoSize = true;
      this->ck_noblur->Location = System::Drawing::Point(6, 106);
      this->ck_noblur->Name = L"ck_noblur";
      this->ck_noblur->Size = System::Drawing::Size(87, 16);
      this->ck_noblur->TabIndex = 5;
      this->ck_noblur->Text = L"�u���[������";
      this->ck_noblur->UseVisualStyleBackColor = true;
      this->ck_noblur->MouseEnter += gcnew System::EventHandler(this, &Form1::ck_noblur_MouseEnter);
      // 
      // ck_vertex_buffer
      // 
      this->ck_vertex_buffer->AutoSize = true;
      this->ck_vertex_buffer->Location = System::Drawing::Point(6, 62);
      this->ck_vertex_buffer->Name = L"ck_vertex_buffer";
      this->ck_vertex_buffer->Size = System::Drawing::Size(80, 16);
      this->ck_vertex_buffer->TabIndex = 3;
      this->ck_vertex_buffer->Text = L"���_�o�b�t�@";
      this->ck_vertex_buffer->UseVisualStyleBackColor = true;
      this->ck_vertex_buffer->MouseEnter += gcnew System::EventHandler(this, &Form1::ck_vertex_buffer_MouseEnter);
      // 
      // gp_other
      // 
      this->gp_other->Controls->Add(this->pn_color_show);
      this->gp_other->Controls->Add(this->bu_color);
      this->gp_other->Controls->Add(this->ck_update);
      this->gp_other->Controls->Add(this->lb_port);
      this->gp_other->Controls->Add(this->ed_port);
      this->gp_other->Controls->Add(this->ck_show_obj);
      this->gp_other->Controls->Add(this->ck_show_fps);
      this->gp_other->Controls->Add(this->ls_name);
      this->gp_other->Controls->Add(this->ed_name);
      this->gp_other->Controls->Add(this->num_thread);
      this->gp_other->Controls->Add(this->lb_thread);
      this->gp_other->Location = System::Drawing::Point(335, 5);
      this->gp_other->Name = L"gp_other";
      this->gp_other->Size = System::Drawing::Size(124, 184);
      this->gp_other->TabIndex = 20;
      this->gp_other->TabStop = false;
      this->gp_other->Text = L"���̑�";
      // 
      // pn_color_show
      // 
      this->pn_color_show->Location = System::Drawing::Point(93, 46);
      this->pn_color_show->Name = L"pn_color_show";
      this->pn_color_show->Size = System::Drawing::Size(19, 17);
      this->pn_color_show->TabIndex = 11;
      // 
      // bu_color
      // 
      this->bu_color->Location = System::Drawing::Point(9, 40);
      this->bu_color->Name = L"bu_color";
      this->bu_color->Size = System::Drawing::Size(75, 23);
      this->bu_color->TabIndex = 10;
      this->bu_color->Text = L"color";
      this->bu_color->UseVisualStyleBackColor = true;
      this->bu_color->Click += gcnew System::EventHandler(this, &Form1::bu_color_Click);
      this->bu_color->MouseEnter += gcnew System::EventHandler(this, &Form1::bu_color_MouseEnter);
      // 
      // ck_update
      // 
      this->ck_update->AutoSize = true;
      this->ck_update->Location = System::Drawing::Point(6, 70);
      this->ck_update->Name = L"ck_update";
      this->ck_update->Size = System::Drawing::Size(101, 16);
      this->ck_update->TabIndex = 8;
      this->ck_update->Text = L"�����A�b�v�f�[�g";
      this->ck_update->UseVisualStyleBackColor = true;
      this->ck_update->MouseEnter += gcnew System::EventHandler(this, &Form1::ck_update_MouseEnter);
      // 
      // lb_port
      // 
      this->lb_port->AutoSize = true;
      this->lb_port->Location = System::Drawing::Point(7, 159);
      this->lb_port->Name = L"lb_port";
      this->lb_port->Size = System::Drawing::Size(25, 12);
      this->lb_port->TabIndex = 7;
      this->lb_port->Text = L"port";
      // 
      // ed_port
      // 
      this->ed_port->Location = System::Drawing::Point(38, 156);
      this->ed_port->MaxLength = 5;
      this->ed_port->Name = L"ed_port";
      this->ed_port->Size = System::Drawing::Size(37, 19);
      this->ed_port->TabIndex = 6;
      this->ed_port->Text = L"10040";
      this->ed_port->MouseEnter += gcnew System::EventHandler(this, &Form1::ed_port_MouseEnter);
      // 
      // ck_show_obj
      // 
      this->ck_show_obj->AutoSize = true;
      this->ck_show_obj->Location = System::Drawing::Point(6, 114);
      this->ck_show_obj->Name = L"ck_show_obj";
      this->ck_show_obj->Size = System::Drawing::Size(111, 16);
      this->ck_show_obj->TabIndex = 4;
      this->ck_show_obj->Text = L"�I�u�W�F�N�g���\��";
      this->ck_show_obj->UseVisualStyleBackColor = true;
      this->ck_show_obj->MouseEnter += gcnew System::EventHandler(this, &Form1::ck_show_obj_MouseEnter);
      // 
      // ck_show_fps
      // 
      this->ck_show_fps->AutoSize = true;
      this->ck_show_fps->Location = System::Drawing::Point(6, 92);
      this->ck_show_fps->Name = L"ck_show_fps";
      this->ck_show_fps->Size = System::Drawing::Size(69, 16);
      this->ck_show_fps->TabIndex = 3;
      this->ck_show_fps->Text = L"FPS�\��";
      this->ck_show_fps->UseVisualStyleBackColor = true;
      this->ck_show_fps->MouseEnter += gcnew System::EventHandler(this, &Form1::ck_show_fps_MouseEnter);
      // 
      // ls_name
      // 
      this->ls_name->AutoSize = true;
      this->ls_name->Location = System::Drawing::Point(6, 18);
      this->ls_name->Name = L"ls_name";
      this->ls_name->Size = System::Drawing::Size(32, 12);
      this->ls_name->TabIndex = 0;
      this->ls_name->Text = L"name";
      // 
      // bu_invoke
      // 
      this->bu_invoke->Location = System::Drawing::Point(523, 421);
      this->bu_invoke->Name = L"bu_invoke";
      this->bu_invoke->Size = System::Drawing::Size(100, 23);
      this->bu_invoke->TabIndex = 103;
      this->bu_invoke->Text = L"�N��";
      this->bu_invoke->UseVisualStyleBackColor = true;
      this->bu_invoke->Click += gcnew System::EventHandler(this, &Form1::bu_invoke_Click);
      this->bu_invoke->MouseEnter += gcnew System::EventHandler(this, &Form1::bu_invoke_Enter);
      // 
      // bu_default_high
      // 
      this->bu_default_high->Location = System::Drawing::Point(118, 421);
      this->bu_default_high->Name = L"bu_default_high";
      this->bu_default_high->Size = System::Drawing::Size(100, 23);
      this->bu_default_high->TabIndex = 101;
      this->bu_default_high->Text = L"�f�t�H���g (high)";
      this->bu_default_high->UseVisualStyleBackColor = true;
      this->bu_default_high->Click += gcnew System::EventHandler(this, &Form1::bu_default_high_Click);
      this->bu_default_high->MouseEnter += gcnew System::EventHandler(this, &Form1::bu_default_high_MouseEnter);
      // 
      // gp_input
      // 
      this->gp_input->Controls->Add(this->cm_key_left);
      this->gp_input->Controls->Add(this->label6);
      this->gp_input->Controls->Add(this->cm_key_right);
      this->gp_input->Controls->Add(this->label5);
      this->gp_input->Controls->Add(this->cm_key_up);
      this->gp_input->Controls->Add(this->label4);
      this->gp_input->Controls->Add(this->cm_key_down);
      this->gp_input->Controls->Add(this->label3);
      this->gp_input->Controls->Add(this->cm_key_rrot);
      this->gp_input->Controls->Add(this->cm_key_lrot);
      this->gp_input->Controls->Add(this->cm_key_catapult);
      this->gp_input->Controls->Add(this->cm_key_direction);
      this->gp_input->Controls->Add(this->cm_key_ray);
      this->gp_input->Controls->Add(this->cm_key_laser);
      this->gp_input->Controls->Add(this->label2);
      this->gp_input->Controls->Add(this->label1);
      this->gp_input->Controls->Add(this->cm_pad_rrot);
      this->gp_input->Controls->Add(this->cm_pad_lrot);
      this->gp_input->Controls->Add(this->lb_menu_esc);
      this->gp_input->Controls->Add(this->cm_pad_menu);
      this->gp_input->Controls->Add(this->lb_menu);
      this->gp_input->Controls->Add(this->cm_pad_catapult);
      this->gp_input->Controls->Add(this->cm_pad_direction);
      this->gp_input->Controls->Add(this->cm_pad_ray);
      this->gp_input->Controls->Add(this->cm_pad_laser);
      this->gp_input->Controls->Add(this->lb_dir);
      this->gp_input->Controls->Add(this->lb_cat);
      this->gp_input->Controls->Add(this->lb_ray);
      this->gp_input->Controls->Add(this->lb_laser);
      this->gp_input->Location = System::Drawing::Point(12, 195);
      this->gp_input->Name = L"gp_input";
      this->gp_input->Size = System::Drawing::Size(428, 160);
      this->gp_input->TabIndex = 40;
      this->gp_input->TabStop = false;
      this->gp_input->Text = L"�L�[�R���t�B�O";
      // 
      // cm_key_left
      // 
      this->cm_key_left->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
      this->cm_key_left->FormattingEnabled = true;
      this->cm_key_left->Items->AddRange(gcnew cli::array< System::Object^  >(63) {L"��", L"��", L"��", L"��", L"A", L"B", L"C", L"D", 
        L"E", L"F", L"G", L"H", L"I", L"J", L"K", L"L", L"M", L"N", L"O", L"P", L"Q", L"R", L"S", L"T", L"U", L"V", L"W", L"X", L"Y", 
        L"Z", L",", L"-", L".", L"/", L"0", L"1", L"2", L"3", L"4", L"5", L"6", L"7", L"8", L"9", L":", L";", L"@", L"[", L"\\", L"]", 
        L"^", L"Backspace", L"Tab", L"Enter", L"Space", L"Del", L"CapsLock", L"�EShift", L"��Shift", L"�ECtrl", L"��Ctrl", L"�EAlt", L"��Alt"});
      this->cm_key_left->Location = System::Drawing::Point(272, 40);
      this->cm_key_left->Name = L"cm_key_left";
      this->cm_key_left->Size = System::Drawing::Size(78, 20);
      this->cm_key_left->TabIndex = 32;
      this->cm_key_left->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::checkKeyBatting);
      this->cm_key_left->MouseEnter += gcnew System::EventHandler(this, &Form1::cm_key1_MouseEnter);
      // 
      // label6
      // 
      this->label6->AutoSize = true;
      this->label6->Location = System::Drawing::Point(229, 43);
      this->label6->Name = L"label6";
      this->label6->Size = System::Drawing::Size(41, 12);
      this->label6->TabIndex = 26;
      this->label6->Text = L"���ړ�";
      // 
      // cm_key_right
      // 
      this->cm_key_right->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
      this->cm_key_right->FormattingEnabled = true;
      this->cm_key_right->Items->AddRange(gcnew cli::array< System::Object^  >(63) {L"��", L"��", L"��", L"��", L"A", L"B", L"C", L"D", 
        L"E", L"F", L"G", L"H", L"I", L"J", L"K", L"L", L"M", L"N", L"O", L"P", L"Q", L"R", L"S", L"T", L"U", L"V", L"W", L"X", L"Y", 
        L"Z", L",", L"-", L".", L"/", L"0", L"1", L"2", L"3", L"4", L"5", L"6", L"7", L"8", L"9", L":", L";", L"@", L"[", L"\\", L"]", 
        L"^", L"Backspace", L"Tab", L"Enter", L"Space", L"Del", L"CapsLock", L"�EShift", L"��Shift", L"�ECtrl", L"��Ctrl", L"�EAlt", L"��Alt"});
      this->cm_key_right->Location = System::Drawing::Point(272, 84);
      this->cm_key_right->Name = L"cm_key_right";
      this->cm_key_right->Size = System::Drawing::Size(78, 20);
      this->cm_key_right->TabIndex = 34;
      this->cm_key_right->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::checkKeyBatting);
      this->cm_key_right->MouseEnter += gcnew System::EventHandler(this, &Form1::cm_key1_MouseEnter);
      // 
      // label5
      // 
      this->label5->AutoSize = true;
      this->label5->Location = System::Drawing::Point(229, 87);
      this->label5->Name = L"label5";
      this->label5->Size = System::Drawing::Size(41, 12);
      this->label5->TabIndex = 24;
      this->label5->Text = L"�E�ړ�";
      // 
      // cm_key_up
      // 
      this->cm_key_up->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
      this->cm_key_up->FormattingEnabled = true;
      this->cm_key_up->Items->AddRange(gcnew cli::array< System::Object^  >(63) {L"��", L"��", L"��", L"��", L"A", L"B", L"C", L"D", 
        L"E", L"F", L"G", L"H", L"I", L"J", L"K", L"L", L"M", L"N", L"O", L"P", L"Q", L"R", L"S", L"T", L"U", L"V", L"W", L"X", L"Y", 
        L"Z", L",", L"-", L".", L"/", L"0", L"1", L"2", L"3", L"4", L"5", L"6", L"7", L"8", L"9", L":", L";", L"@", L"[", L"\\", L"]", 
        L"^", L"Backspace", L"Tab", L"Enter", L"Space", L"Del", L"CapsLock", L"�EShift", L"��Shift", L"�ECtrl", L"��Ctrl", L"�EAlt", L"��Alt"});
      this->cm_key_up->Location = System::Drawing::Point(272, 18);
      this->cm_key_up->Name = L"cm_key_up";
      this->cm_key_up->Size = System::Drawing::Size(78, 20);
      this->cm_key_up->TabIndex = 31;
      this->cm_key_up->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::checkKeyBatting);
      this->cm_key_up->MouseEnter += gcnew System::EventHandler(this, &Form1::cm_key1_MouseEnter);
      // 
      // label4
      // 
      this->label4->AutoSize = true;
      this->label4->Location = System::Drawing::Point(229, 21);
      this->label4->Name = L"label4";
      this->label4->Size = System::Drawing::Size(41, 12);
      this->label4->TabIndex = 22;
      this->label4->Text = L"��ړ�";
      // 
      // cm_key_down
      // 
      this->cm_key_down->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
      this->cm_key_down->FormattingEnabled = true;
      this->cm_key_down->Items->AddRange(gcnew cli::array< System::Object^  >(63) {L"��", L"��", L"��", L"��", L"A", L"B", L"C", L"D", 
        L"E", L"F", L"G", L"H", L"I", L"J", L"K", L"L", L"M", L"N", L"O", L"P", L"Q", L"R", L"S", L"T", L"U", L"V", L"W", L"X", L"Y", 
        L"Z", L",", L"-", L".", L"/", L"0", L"1", L"2", L"3", L"4", L"5", L"6", L"7", L"8", L"9", L":", L";", L"@", L"[", L"\\", L"]", 
        L"^", L"Backspace", L"Tab", L"Enter", L"Space", L"Del", L"CapsLock", L"�EShift", L"��Shift", L"�ECtrl", L"��Ctrl", L"�EAlt", L"��Alt"});
      this->cm_key_down->Location = System::Drawing::Point(272, 62);
      this->cm_key_down->Name = L"cm_key_down";
      this->cm_key_down->Size = System::Drawing::Size(78, 20);
      this->cm_key_down->TabIndex = 33;
      this->cm_key_down->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::checkKeyBatting);
      this->cm_key_down->MouseEnter += gcnew System::EventHandler(this, &Form1::cm_key1_MouseEnter);
      // 
      // label3
      // 
      this->label3->AutoSize = true;
      this->label3->Location = System::Drawing::Point(229, 65);
      this->label3->Name = L"label3";
      this->label3->Size = System::Drawing::Size(41, 12);
      this->label3->TabIndex = 20;
      this->label3->Text = L"���ړ�";
      // 
      // cm_key_rrot
      // 
      this->cm_key_rrot->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
      this->cm_key_rrot->FormattingEnabled = true;
      this->cm_key_rrot->Items->AddRange(gcnew cli::array< System::Object^  >(63) {L"��", L"��", L"��", L"��", L"A", L"B", L"C", L"D", 
        L"E", L"F", L"G", L"H", L"I", L"J", L"K", L"L", L"M", L"N", L"O", L"P", L"Q", L"R", L"S", L"T", L"U", L"V", L"W", L"X", L"Y", 
        L"Z", L",", L"-", L".", L"/", L"0", L"1", L"2", L"3", L"4", L"5", L"6", L"7", L"8", L"9", L":", L";", L"@", L"[", L"\\", L"]", 
        L"^", L"Backspace", L"Tab", L"Enter", L"Space", L"Del", L"CapsLock", L"�EShift", L"��Shift", L"�ECtrl", L"��Ctrl", L"�EAlt", L"��Alt"});
      this->cm_key_rrot->Location = System::Drawing::Point(272, 132);
      this->cm_key_rrot->Name = L"cm_key_rrot";
      this->cm_key_rrot->Size = System::Drawing::Size(78, 20);
      this->cm_key_rrot->TabIndex = 36;
      this->cm_key_rrot->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::checkKeyBatting);
      this->cm_key_rrot->MouseEnter += gcnew System::EventHandler(this, &Form1::cm_key1_MouseEnter);
      // 
      // cm_key_lrot
      // 
      this->cm_key_lrot->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
      this->cm_key_lrot->FormattingEnabled = true;
      this->cm_key_lrot->Items->AddRange(gcnew cli::array< System::Object^  >(63) {L"��", L"��", L"��", L"��", L"A", L"B", L"C", L"D", 
        L"E", L"F", L"G", L"H", L"I", L"J", L"K", L"L", L"M", L"N", L"O", L"P", L"Q", L"R", L"S", L"T", L"U", L"V", L"W", L"X", L"Y", 
        L"Z", L",", L"-", L".", L"/", L"0", L"1", L"2", L"3", L"4", L"5", L"6", L"7", L"8", L"9", L":", L";", L"@", L"[", L"\\", L"]", 
        L"^", L"Backspace", L"Tab", L"Enter", L"Space", L"Del", L"CapsLock", L"�EShift", L"��Shift", L"�ECtrl", L"��Ctrl", L"�EAlt", L"��Alt"});
      this->cm_key_lrot->Location = System::Drawing::Point(272, 110);
      this->cm_key_lrot->Name = L"cm_key_lrot";
      this->cm_key_lrot->Size = System::Drawing::Size(78, 20);
      this->cm_key_lrot->TabIndex = 35;
      this->cm_key_lrot->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::checkKeyBatting);
      this->cm_key_lrot->MouseEnter += gcnew System::EventHandler(this, &Form1::cm_key1_MouseEnter);
      // 
      // cm_key_catapult
      // 
      this->cm_key_catapult->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
      this->cm_key_catapult->FormattingEnabled = true;
      this->cm_key_catapult->Items->AddRange(gcnew cli::array< System::Object^  >(63) {L"��", L"��", L"��", L"��", L"A", L"B", L"C", 
        L"D", L"E", L"F", L"G", L"H", L"I", L"J", L"K", L"L", L"M", L"N", L"O", L"P", L"Q", L"R", L"S", L"T", L"U", L"V", L"W", L"X", 
        L"Y", L"Z", L",", L"-", L".", L"/", L"0", L"1", L"2", L"3", L"4", L"5", L"6", L"7", L"8", L"9", L":", L";", L"@", L"[", L"\\", 
        L"]", L"^", L"Backspace", L"Tab", L"Enter", L"Space", L"Del", L"CapsLock", L"�EShift", L"��Shift", L"�ECtrl", L"��Ctrl", L"�EAlt", 
        L"��Alt"});
      this->cm_key_catapult->Location = System::Drawing::Point(71, 62);
      this->cm_key_catapult->Name = L"cm_key_catapult";
      this->cm_key_catapult->Size = System::Drawing::Size(78, 20);
      this->cm_key_catapult->TabIndex = 13;
      this->cm_key_catapult->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::checkKeyBatting);
      this->cm_key_catapult->MouseEnter += gcnew System::EventHandler(this, &Form1::cm_key1_MouseEnter);
      // 
      // cm_key_direction
      // 
      this->cm_key_direction->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
      this->cm_key_direction->FormattingEnabled = true;
      this->cm_key_direction->Items->AddRange(gcnew cli::array< System::Object^  >(63) {L"��", L"��", L"��", L"��", L"A", L"B", L"C", 
        L"D", L"E", L"F", L"G", L"H", L"I", L"J", L"K", L"L", L"M", L"N", L"O", L"P", L"Q", L"R", L"S", L"T", L"U", L"V", L"W", L"X", 
        L"Y", L"Z", L",", L"-", L".", L"/", L"0", L"1", L"2", L"3", L"4", L"5", L"6", L"7", L"8", L"9", L":", L";", L"@", L"[", L"\\", 
        L"]", L"^", L"Backspace", L"Tab", L"Enter", L"Space", L"Del", L"CapsLock", L"�EShift", L"��Shift", L"�ECtrl", L"��Ctrl", L"�EAlt", 
        L"��Alt"});
      this->cm_key_direction->Location = System::Drawing::Point(71, 84);
      this->cm_key_direction->Name = L"cm_key_direction";
      this->cm_key_direction->Size = System::Drawing::Size(78, 20);
      this->cm_key_direction->TabIndex = 14;
      this->cm_key_direction->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::checkKeyBatting);
      this->cm_key_direction->MouseEnter += gcnew System::EventHandler(this, &Form1::cm_key1_MouseEnter);
      // 
      // cm_key_ray
      // 
      this->cm_key_ray->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
      this->cm_key_ray->FormattingEnabled = true;
      this->cm_key_ray->Items->AddRange(gcnew cli::array< System::Object^  >(63) {L"��", L"��", L"��", L"��", L"A", L"B", L"C", L"D", 
        L"E", L"F", L"G", L"H", L"I", L"J", L"K", L"L", L"M", L"N", L"O", L"P", L"Q", L"R", L"S", L"T", L"U", L"V", L"W", L"X", L"Y", 
        L"Z", L",", L"-", L".", L"/", L"0", L"1", L"2", L"3", L"4", L"5", L"6", L"7", L"8", L"9", L":", L";", L"@", L"[", L"\\", L"]", 
        L"^", L"Backspace", L"Tab", L"Enter", L"Space", L"Del", L"CapsLock", L"�EShift", L"��Shift", L"�ECtrl", L"��Ctrl", L"�EAlt", L"��Alt"});
      this->cm_key_ray->Location = System::Drawing::Point(71, 40);
      this->cm_key_ray->Name = L"cm_key_ray";
      this->cm_key_ray->Size = System::Drawing::Size(78, 20);
      this->cm_key_ray->TabIndex = 12;
      this->cm_key_ray->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::checkKeyBatting);
      this->cm_key_ray->MouseEnter += gcnew System::EventHandler(this, &Form1::cm_key1_MouseEnter);
      // 
      // cm_key_laser
      // 
      this->cm_key_laser->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
      this->cm_key_laser->FormattingEnabled = true;
      this->cm_key_laser->Items->AddRange(gcnew cli::array< System::Object^  >(63) {L"��", L"��", L"��", L"��", L"A", L"B", L"C", L"D", 
        L"E", L"F", L"G", L"H", L"I", L"J", L"K", L"L", L"M", L"N", L"O", L"P", L"Q", L"R", L"S", L"T", L"U", L"V", L"W", L"X", L"Y", 
        L"Z", L",", L"-", L".", L"/", L"0", L"1", L"2", L"3", L"4", L"5", L"6", L"7", L"8", L"9", L":", L";", L"@", L"[", L"\\", L"]", 
        L"^", L"Backspace", L"Tab", L"Enter", L"Space", L"Del", L"CapsLock", L"�EShift", L"��Shift", L"�ECtrl", L"��Ctrl", L"�EAlt", L"��Alt"});
      this->cm_key_laser->Location = System::Drawing::Point(71, 18);
      this->cm_key_laser->Name = L"cm_key_laser";
      this->cm_key_laser->Size = System::Drawing::Size(78, 20);
      this->cm_key_laser->TabIndex = 11;
      this->cm_key_laser->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::checkKeyBatting);
      this->cm_key_laser->MouseEnter += gcnew System::EventHandler(this, &Form1::cm_key1_MouseEnter);
      // 
      // label2
      // 
      this->label2->AutoSize = true;
      this->label2->Location = System::Drawing::Point(229, 135);
      this->label2->Name = L"label2";
      this->label2->Size = System::Drawing::Size(41, 12);
      this->label2->TabIndex = 14;
      this->label2->Text = L"�E��]";
      // 
      // label1
      // 
      this->label1->AutoSize = true;
      this->label1->Location = System::Drawing::Point(229, 113);
      this->label1->Name = L"label1";
      this->label1->Size = System::Drawing::Size(41, 12);
      this->label1->TabIndex = 13;
      this->label1->Text = L"����]";
      // 
      // cm_pad_rrot
      // 
      this->cm_pad_rrot->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
      this->cm_pad_rrot->FormattingEnabled = true;
      this->cm_pad_rrot->Items->AddRange(gcnew cli::array< System::Object^  >(23) {L"�{�^��1", L"�{�^��2", L"�{�^��3", L"�{�^��4", L"�{�^��5", 
        L"�{�^��6", L"�{�^��7", L"�{�^��8", L"�{�^��9", L"�{�^��10", L"�{�^��11", L"�{�^��12", L"�{�^��13", L"�{�^��14", L"�{�^��15", L"�{�^��16", L"��3up", L"��3down", 
        L"��4up", L"��4down", L"��5up", L"��5down", L"����"});
      this->cm_pad_rrot->Location = System::Drawing::Point(356, 132);
      this->cm_pad_rrot->Name = L"cm_pad_rrot";
      this->cm_pad_rrot->Size = System::Drawing::Size(66, 20);
      this->cm_pad_rrot->TabIndex = 38;
      this->cm_pad_rrot->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::checkButtonBatting);
      this->cm_pad_rrot->MouseEnter += gcnew System::EventHandler(this, &Form1::cm_pad1_MouseEnter);
      // 
      // cm_pad_lrot
      // 
      this->cm_pad_lrot->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
      this->cm_pad_lrot->FormattingEnabled = true;
      this->cm_pad_lrot->Items->AddRange(gcnew cli::array< System::Object^  >(23) {L"�{�^��1", L"�{�^��2", L"�{�^��3", L"�{�^��4", L"�{�^��5", 
        L"�{�^��6", L"�{�^��7", L"�{�^��8", L"�{�^��9", L"�{�^��10", L"�{�^��11", L"�{�^��12", L"�{�^��13", L"�{�^��14", L"�{�^��15", L"�{�^��16", L"��3up", L"��3down", 
        L"��4up", L"��4down", L"��5up", L"��5down", L"����"});
      this->cm_pad_lrot->Location = System::Drawing::Point(356, 110);
      this->cm_pad_lrot->Name = L"cm_pad_lrot";
      this->cm_pad_lrot->Size = System::Drawing::Size(66, 20);
      this->cm_pad_lrot->TabIndex = 37;
      this->cm_pad_lrot->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::checkButtonBatting);
      this->cm_pad_lrot->MouseEnter += gcnew System::EventHandler(this, &Form1::cm_pad1_MouseEnter);
      // 
      // lb_menu_esc
      // 
      this->lb_menu_esc->AutoSize = true;
      this->lb_menu_esc->Location = System::Drawing::Point(72, 113);
      this->lb_menu_esc->Name = L"lb_menu_esc";
      this->lb_menu_esc->Size = System::Drawing::Size(24, 12);
      this->lb_menu_esc->TabIndex = 0;
      this->lb_menu_esc->Text = L"Esc";
      // 
      // cm_pad_menu
      // 
      this->cm_pad_menu->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
      this->cm_pad_menu->FormattingEnabled = true;
      this->cm_pad_menu->Items->AddRange(gcnew cli::array< System::Object^  >(23) {L"�{�^��1", L"�{�^��2", L"�{�^��3", L"�{�^��4", L"�{�^��5", 
        L"�{�^��6", L"�{�^��7", L"�{�^��8", L"�{�^��9", L"�{�^��10", L"�{�^��11", L"�{�^��12", L"�{�^��13", L"�{�^��14", L"�{�^��15", L"�{�^��16", L"��3up", L"��3down", 
        L"��4up", L"��4down", L"��5up", L"��5down", L"����"});
      this->cm_pad_menu->Location = System::Drawing::Point(155, 110);
      this->cm_pad_menu->Name = L"cm_pad_menu";
      this->cm_pad_menu->Size = System::Drawing::Size(66, 20);
      this->cm_pad_menu->TabIndex = 19;
      this->cm_pad_menu->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::checkButtonBatting);
      this->cm_pad_menu->MouseEnter += gcnew System::EventHandler(this, &Form1::cm_pad1_MouseEnter);
      // 
      // lb_menu
      // 
      this->lb_menu->AutoSize = true;
      this->lb_menu->Location = System::Drawing::Point(6, 113);
      this->lb_menu->Name = L"lb_menu";
      this->lb_menu->Size = System::Drawing::Size(40, 12);
      this->lb_menu->TabIndex = 0;
      this->lb_menu->Text = L"���j���[";
      // 
      // cm_pad_catapult
      // 
      this->cm_pad_catapult->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
      this->cm_pad_catapult->FormattingEnabled = true;
      this->cm_pad_catapult->Items->AddRange(gcnew cli::array< System::Object^  >(23) {L"�{�^��1", L"�{�^��2", L"�{�^��3", L"�{�^��4", L"�{�^��5", 
        L"�{�^��6", L"�{�^��7", L"�{�^��8", L"�{�^��9", L"�{�^��10", L"�{�^��11", L"�{�^��12", L"�{�^��13", L"�{�^��14", L"�{�^��15", L"�{�^��16", L"��3up", L"��3down", 
        L"��4up", L"��4down", L"��5up", L"��5down", L"����"});
      this->cm_pad_catapult->Location = System::Drawing::Point(155, 62);
      this->cm_pad_catapult->Name = L"cm_pad_catapult";
      this->cm_pad_catapult->Size = System::Drawing::Size(66, 20);
      this->cm_pad_catapult->TabIndex = 17;
      this->cm_pad_catapult->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::checkButtonBatting);
      this->cm_pad_catapult->MouseEnter += gcnew System::EventHandler(this, &Form1::cm_pad1_MouseEnter);
      // 
      // cm_pad_direction
      // 
      this->cm_pad_direction->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
      this->cm_pad_direction->FormattingEnabled = true;
      this->cm_pad_direction->Items->AddRange(gcnew cli::array< System::Object^  >(23) {L"�{�^��1", L"�{�^��2", L"�{�^��3", L"�{�^��4", L"�{�^��5", 
        L"�{�^��6", L"�{�^��7", L"�{�^��8", L"�{�^��9", L"�{�^��10", L"�{�^��11", L"�{�^��12", L"�{�^��13", L"�{�^��14", L"�{�^��15", L"�{�^��16", L"��3up", L"��3down", 
        L"��4up", L"��4down", L"��5up", L"��5down", L"����"});
      this->cm_pad_direction->Location = System::Drawing::Point(155, 84);
      this->cm_pad_direction->Name = L"cm_pad_direction";
      this->cm_pad_direction->Size = System::Drawing::Size(66, 20);
      this->cm_pad_direction->TabIndex = 18;
      this->cm_pad_direction->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::checkButtonBatting);
      this->cm_pad_direction->MouseEnter += gcnew System::EventHandler(this, &Form1::cm_pad1_MouseEnter);
      // 
      // cm_pad_ray
      // 
      this->cm_pad_ray->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
      this->cm_pad_ray->FormattingEnabled = true;
      this->cm_pad_ray->Items->AddRange(gcnew cli::array< System::Object^  >(23) {L"�{�^��1", L"�{�^��2", L"�{�^��3", L"�{�^��4", L"�{�^��5", L"�{�^��6", 
        L"�{�^��7", L"�{�^��8", L"�{�^��9", L"�{�^��10", L"�{�^��11", L"�{�^��12", L"�{�^��13", L"�{�^��14", L"�{�^��15", L"�{�^��16", L"��3up", L"��3down", L"��4up", 
        L"��4down", L"��5up", L"��5down", L"����"});
      this->cm_pad_ray->Location = System::Drawing::Point(155, 40);
      this->cm_pad_ray->Name = L"cm_pad_ray";
      this->cm_pad_ray->Size = System::Drawing::Size(66, 20);
      this->cm_pad_ray->TabIndex = 16;
      this->cm_pad_ray->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::checkButtonBatting);
      this->cm_pad_ray->MouseEnter += gcnew System::EventHandler(this, &Form1::cm_pad1_MouseEnter);
      // 
      // cm_pad_laser
      // 
      this->cm_pad_laser->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
      this->cm_pad_laser->FormattingEnabled = true;
      this->cm_pad_laser->Items->AddRange(gcnew cli::array< System::Object^  >(23) {L"�{�^��1", L"�{�^��2", L"�{�^��3", L"�{�^��4", L"�{�^��5", 
        L"�{�^��6", L"�{�^��7", L"�{�^��8", L"�{�^��9", L"�{�^��10", L"�{�^��11", L"�{�^��12", L"�{�^��13", L"�{�^��14", L"�{�^��15", L"�{�^��16", L"��3up", L"��3down", 
        L"��4up", L"��4down", L"��5up", L"��5down", L"����"});
      this->cm_pad_laser->Location = System::Drawing::Point(155, 18);
      this->cm_pad_laser->Name = L"cm_pad_laser";
      this->cm_pad_laser->Size = System::Drawing::Size(66, 20);
      this->cm_pad_laser->TabIndex = 15;
      this->cm_pad_laser->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::checkButtonBatting);
      this->cm_pad_laser->MouseEnter += gcnew System::EventHandler(this, &Form1::cm_pad1_MouseEnter);
      // 
      // lb_dir
      // 
      this->lb_dir->AutoSize = true;
      this->lb_dir->Location = System::Drawing::Point(6, 87);
      this->lb_dir->Name = L"lb_dir";
      this->lb_dir->Size = System::Drawing::Size(64, 12);
      this->lb_dir->TabIndex = 0;
      this->lb_dir->Text = L"�f�B���N�V����";
      // 
      // lb_cat
      // 
      this->lb_cat->AutoSize = true;
      this->lb_cat->Location = System::Drawing::Point(6, 65);
      this->lb_cat->Name = L"lb_cat";
      this->lb_cat->Size = System::Drawing::Size(50, 12);
      this->lb_cat->TabIndex = 0;
      this->lb_cat->Text = L"�J�^�p���g";
      // 
      // lb_ray
      // 
      this->lb_ray->AutoSize = true;
      this->lb_ray->Location = System::Drawing::Point(6, 43);
      this->lb_ray->Name = L"lb_ray";
      this->lb_ray->Size = System::Drawing::Size(23, 12);
      this->lb_ray->TabIndex = 0;
      this->lb_ray->Text = L"���C";
      // 
      // lb_laser
      // 
      this->lb_laser->AutoSize = true;
      this->lb_laser->Location = System::Drawing::Point(6, 23);
      this->lb_laser->Name = L"lb_laser";
      this->lb_laser->Size = System::Drawing::Size(44, 12);
      this->lb_laser->TabIndex = 0;
      this->lb_laser->Text = L"���[�U�[";
      // 
      // lb_threshold
      // 
      this->lb_threshold->AutoSize = true;
      this->lb_threshold->Location = System::Drawing::Point(6, 87);
      this->lb_threshold->Name = L"lb_threshold";
      this->lb_threshold->Size = System::Drawing::Size(65, 12);
      this->lb_threshold->TabIndex = 17;
      this->lb_threshold->Text = L"�ړ���臒l";
      this->lb_threshold->MouseEnter += gcnew System::EventHandler(this, &Form1::ed_threshold1_MouseEnter);
      // 
      // gp_sound
      // 
      this->gp_sound->Controls->Add(this->sl_se_volume);
      this->gp_sound->Controls->Add(this->sl_bgm_volume);
      this->gp_sound->Controls->Add(this->ck_se);
      this->gp_sound->Controls->Add(this->ck_bgm);
      this->gp_sound->Location = System::Drawing::Point(465, 6);
      this->gp_sound->Name = L"gp_sound";
      this->gp_sound->Size = System::Drawing::Size(157, 183);
      this->gp_sound->TabIndex = 30;
      this->gp_sound->TabStop = false;
      this->gp_sound->Text = L"�T�E���h";
      // 
      // sl_se_volume
      // 
      this->sl_se_volume->Location = System::Drawing::Point(57, 69);
      this->sl_se_volume->Maximum = 8;
      this->sl_se_volume->Name = L"sl_se_volume";
      this->sl_se_volume->Size = System::Drawing::Size(94, 45);
      this->sl_se_volume->TabIndex = 5;
      this->sl_se_volume->MouseEnter += gcnew System::EventHandler(this, &Form1::sl_se_volume_MouseEnter);
      // 
      // sl_bgm_volume
      // 
      this->sl_bgm_volume->Location = System::Drawing::Point(57, 18);
      this->sl_bgm_volume->Maximum = 8;
      this->sl_bgm_volume->Name = L"sl_bgm_volume";
      this->sl_bgm_volume->Size = System::Drawing::Size(94, 45);
      this->sl_bgm_volume->TabIndex = 3;
      this->sl_bgm_volume->MouseEnter += gcnew System::EventHandler(this, &Form1::sl_bgm_volume_MouseEnter);
      // 
      // ck_se
      // 
      this->ck_se->AutoSize = true;
      this->ck_se->Location = System::Drawing::Point(6, 77);
      this->ck_se->Name = L"ck_se";
      this->ck_se->Size = System::Drawing::Size(38, 16);
      this->ck_se->TabIndex = 4;
      this->ck_se->Text = L"SE";
      this->ck_se->UseVisualStyleBackColor = true;
      this->ck_se->MouseEnter += gcnew System::EventHandler(this, &Form1::ck_se_MouseEnter);
      // 
      // ck_bgm
      // 
      this->ck_bgm->AutoSize = true;
      this->ck_bgm->Location = System::Drawing::Point(6, 28);
      this->ck_bgm->Name = L"ck_bgm";
      this->ck_bgm->Size = System::Drawing::Size(49, 16);
      this->ck_bgm->TabIndex = 2;
      this->ck_bgm->Text = L"BGM";
      this->ck_bgm->UseVisualStyleBackColor = true;
      this->ck_bgm->MouseEnter += gcnew System::EventHandler(this, &Form1::ck_bgm_MouseEnter);
      // 
      // bu_default_low
      // 
      this->bu_default_low->Location = System::Drawing::Point(12, 421);
      this->bu_default_low->Name = L"bu_default_low";
      this->bu_default_low->Size = System::Drawing::Size(100, 23);
      this->bu_default_low->TabIndex = 100;
      this->bu_default_low->Text = L"�f�t�H���g (low)";
      this->bu_default_low->UseVisualStyleBackColor = true;
      this->bu_default_low->Click += gcnew System::EventHandler(this, &Form1::bu_default_low_Click);
      this->bu_default_low->MouseEnter += gcnew System::EventHandler(this, &Form1::bu_default_low_MouseEnter);
      // 
      // cm_controller
      // 
      this->cm_controller->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
      this->cm_controller->FormattingEnabled = true;
      this->cm_controller->Location = System::Drawing::Point(6, 18);
      this->cm_controller->Name = L"cm_controller";
      this->cm_controller->Size = System::Drawing::Size(165, 20);
      this->cm_controller->TabIndex = 1;
      this->cm_controller->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::cm_controller_SelectedIndexChanged);
      this->cm_controller->MouseEnter += gcnew System::EventHandler(this, &Form1::cm_controller_MouseEnter);
      // 
      // groupBox1
      // 
      this->groupBox1->Controls->Add(this->ck_hat);
      this->groupBox1->Controls->Add(this->ed_threshold2);
      this->groupBox1->Controls->Add(this->ed_threshold1);
      this->groupBox1->Controls->Add(this->label9);
      this->groupBox1->Controls->Add(this->groupBox3);
      this->groupBox1->Controls->Add(this->cm_controller);
      this->groupBox1->Controls->Add(this->lb_threshold);
      this->groupBox1->Location = System::Drawing::Point(446, 196);
      this->groupBox1->Name = L"groupBox1";
      this->groupBox1->Size = System::Drawing::Size(177, 159);
      this->groupBox1->TabIndex = 50;
      this->groupBox1->TabStop = false;
      this->groupBox1->Text = L"�R���g���[��";
      // 
      // ck_hat
      // 
      this->ck_hat->AutoSize = true;
      this->ck_hat->Location = System::Drawing::Point(8, 132);
      this->ck_hat->Name = L"ck_hat";
      this->ck_hat->Size = System::Drawing::Size(117, 16);
      this->ck_hat->TabIndex = 5;
      this->ck_hat->Text = L"�n�b�g�X�C�b�`�̎g�p";
      this->ck_hat->UseVisualStyleBackColor = true;
      this->ck_hat->MouseEnter += gcnew System::EventHandler(this, &Form1::ck_hat_MouseEnter);
      // 
      // ed_threshold2
      // 
      this->ed_threshold2->Location = System::Drawing::Point(77, 107);
      this->ed_threshold2->MaxLength = 5;
      this->ed_threshold2->Name = L"ed_threshold2";
      this->ed_threshold2->Size = System::Drawing::Size(37, 19);
      this->ed_threshold2->TabIndex = 4;
      this->ed_threshold2->Text = L"15000";
      this->ed_threshold2->MouseEnter += gcnew System::EventHandler(this, &Form1::ed_threshold2_MouseEnter);
      // 
      // ed_threshold1
      // 
      this->ed_threshold1->Location = System::Drawing::Point(77, 84);
      this->ed_threshold1->MaxLength = 5;
      this->ed_threshold1->Name = L"ed_threshold1";
      this->ed_threshold1->Size = System::Drawing::Size(37, 19);
      this->ed_threshold1->TabIndex = 3;
      this->ed_threshold1->Text = L"15000";
      this->ed_threshold1->MouseEnter += gcnew System::EventHandler(this, &Form1::ed_threshold1_MouseEnter);
      // 
      // label9
      // 
      this->label9->AutoSize = true;
      this->label9->Location = System::Drawing::Point(6, 110);
      this->label9->Name = L"label9";
      this->label9->Size = System::Drawing::Size(65, 12);
      this->label9->TabIndex = 18;
      this->label9->Text = L"������臒l";
      this->label9->MouseEnter += gcnew System::EventHandler(this, &Form1::ed_threshold2_MouseEnter);
      // 
      // groupBox3
      // 
      this->groupBox3->Controls->Add(this->label8);
      this->groupBox3->Controls->Add(this->label7);
      this->groupBox3->Controls->Add(this->cm_dir_y);
      this->groupBox3->Controls->Add(this->cm_dir_x);
      this->groupBox3->Location = System::Drawing::Point(6, 42);
      this->groupBox3->Name = L"groupBox3";
      this->groupBox3->Size = System::Drawing::Size(165, 39);
      this->groupBox3->TabIndex = 2;
      this->groupBox3->TabStop = false;
      this->groupBox3->Text = L"������";
      // 
      // label8
      // 
      this->label8->AutoSize = true;
      this->label8->Location = System::Drawing::Point(84, 20);
      this->label8->Name = L"label8";
      this->label8->Size = System::Drawing::Size(12, 12);
      this->label8->TabIndex = 7;
      this->label8->Text = L"Y";
      // 
      // label7
      // 
      this->label7->AutoSize = true;
      this->label7->Location = System::Drawing::Point(5, 20);
      this->label7->Name = L"label7";
      this->label7->Size = System::Drawing::Size(12, 12);
      this->label7->TabIndex = 6;
      this->label7->Text = L"X";
      // 
      // cm_dir_y
      // 
      this->cm_dir_y->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
      this->cm_dir_y->FormattingEnabled = true;
      this->cm_dir_y->Items->AddRange(gcnew cli::array< System::Object^  >(1) {L"����"});
      this->cm_dir_y->Location = System::Drawing::Point(102, 14);
      this->cm_dir_y->Name = L"cm_dir_y";
      this->cm_dir_y->Size = System::Drawing::Size(56, 20);
      this->cm_dir_y->TabIndex = 3;
      this->cm_dir_y->MouseEnter += gcnew System::EventHandler(this, &Form1::cm_dir_x_MouseEnter);
      // 
      // cm_dir_x
      // 
      this->cm_dir_x->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
      this->cm_dir_x->FormattingEnabled = true;
      this->cm_dir_x->Items->AddRange(gcnew cli::array< System::Object^  >(1) {L"����"});
      this->cm_dir_x->Location = System::Drawing::Point(20, 14);
      this->cm_dir_x->Name = L"cm_dir_x";
      this->cm_dir_x->Size = System::Drawing::Size(56, 20);
      this->cm_dir_x->TabIndex = 2;
      this->cm_dir_x->MouseEnter += gcnew System::EventHandler(this, &Form1::cm_dir_x_MouseEnter);
      // 
      // bu_reset_keyassign
      // 
      this->bu_reset_keyassign->Location = System::Drawing::Point(264, 421);
      this->bu_reset_keyassign->Name = L"bu_reset_keyassign";
      this->bu_reset_keyassign->Size = System::Drawing::Size(119, 23);
      this->bu_reset_keyassign->TabIndex = 104;
      this->bu_reset_keyassign->Text = L"�L�[�R���t�B�O������";
      this->bu_reset_keyassign->UseVisualStyleBackColor = true;
      this->bu_reset_keyassign->Click += gcnew System::EventHandler(this, &Form1::bu_reset_keyassign_Click);
      this->bu_reset_keyassign->MouseEnter += gcnew System::EventHandler(this, &Form1::bu_reset_keyassign_MouseEnter);
      // 
      // Form1
      // 
      this->AutoScaleDimensions = System::Drawing::SizeF(6, 12);
      this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
      this->ClientSize = System::Drawing::Size(634, 453);
      this->Controls->Add(this->bu_reset_keyassign);
      this->Controls->Add(this->groupBox1);
      this->Controls->Add(this->bu_default_low);
      this->Controls->Add(this->gp_sound);
      this->Controls->Add(this->gp_input);
      this->Controls->Add(this->bu_default_high);
      this->Controls->Add(this->bu_invoke);
      this->Controls->Add(this->gp_other);
      this->Controls->Add(this->gp_draw);
      this->Controls->Add(this->gp_resolution);
      this->Controls->Add(this->ed_desc);
      this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
      this->Icon = (cli::safe_cast<System::Drawing::Icon^  >(resources->GetObject(L"$this.Icon")));
      this->MaximizeBox = false;
      this->MinimizeBox = false;
      this->Name = L"Form1";
      this->Text = L"exception config";
      this->Load += gcnew System::EventHandler(this, &Form1::Form1_Load);
      this->FormClosed += gcnew System::Windows::Forms::FormClosedEventHandler(this, &Form1::Form1_FormClosed);
      this->gp_resolution->ResumeLayout(false);
      this->gp_resolution->PerformLayout();
      (cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->num_thread))->EndInit();
      this->gp_draw->ResumeLayout(false);
      this->gp_draw->PerformLayout();
      (cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->sl_bloom))->EndInit();
      this->gp_other->ResumeLayout(false);
      this->gp_other->PerformLayout();
      this->gp_input->ResumeLayout(false);
      this->gp_input->PerformLayout();
      this->gp_sound->ResumeLayout(false);
      this->gp_sound->PerformLayout();
      (cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->sl_se_volume))->EndInit();
      (cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->sl_bgm_volume))->EndInit();
      this->groupBox1->ResumeLayout(false);
      this->groupBox1->PerformLayout();
      this->groupBox3->ResumeLayout(false);
      this->groupBox3->PerformLayout();
      this->ResumeLayout(false);
      this->PerformLayout();

    }
#pragma endregion


private: System::Void Form1_Load(System::Object^  sender, System::EventArgs^  e) {
         }
private: System::Void Form1_FormClosed(System::Object^  sender, System::Windows::Forms::FormClosedEventArgs^  e) {
           save();
         }
private: System::Void ls_resolution_MouseEnter(System::Object^  sender, System::EventArgs^  e) {
           setDescription(L"��ʉ𑜓x���w�肵�܂��B");
         }
private: System::Void ck_fullscreen_MouseEnter(System::Object^  sender, System::EventArgs^  e) {
           setDescription(L"�w��𑜓x�A32bit�J���[�A���t���b�V�����[�g60Hz�̃t���X�N���[�����[�h�ł̋N�������݂܂��B");
         }
private: System::Void ck_vsync_MouseEnter(System::Object^  sender, System::EventArgs^  e) {
           setDescription(L"��ʂ̍X�V�Ԋu�����j�^�̃��t���b�V�����[�g�ɍ��킹�A���炩�ɉ�ʂ��X�V���܂��B\r\n���t���b�V�����[�g��60Hz�̏ꍇ��t���X�N���[�����[�h�̏ꍇ�Ɍ��ʓI�ł��B");
         }
private: System::Void ck_vertex_buffer_MouseEnter(System::Object^  sender, System::EventArgs^  e) {
           setDescription(L"�Ή����Ă���Β��_�o�b�t�@���g�p���܂��B\r\n�`��̎኱�̍����������҂ł��܂��B");
         }
private: System::Void ck_noblur_MouseEnter(System::Object^  sender, System::EventArgs^  e) {
           setDescription(L"�ł����G��j�󂵂��Ƃ��Ȃǂɔ�������A��ʂ��c�ފ����̃G�t�F�N�g�ނ𖳌��ɂ��܂��B\r\n�኱�������ׂ��y������܂����A�����ڂ��₵���Ȃ�܂��B");
         }
private: System::Void ck_exlight_MouseEnter(System::Object^  sender, System::EventArgs^  e) {
           setDescription(L"�ł����G��j�󂵂��Ƃ��ȂǂɌ�����ݒu���A�ꎞ�I�Ɏ��͂𖾂邭���܂��B\r\n����������Ǝ኱�������ׂ��y�����܂����A�����ڂ��₵���Ȃ�܂��B");
         }
private: System::Void ck_30fps_MouseEnter(System::Object^  sender, System::EventArgs^  e) {
           setDescription(L"�`��̕p�x�𔼕��ɂ��܂��B\r\n�ǂ����Ă����삪�d���ꍇ�L���ɂ��Ă݂Ă��������B\r\n�܂��A�����L���ɂ��Ă���ꍇ�AVSync�̐ݒ�𖳎����ă^�C�}�[�ő҂悤�ɂȂ�܂��B");
         }
private: System::Void ck_shader_MouseEnter(System::Object^  sender, System::EventArgs^  e) {
           setDescription(L"�Ή����Ă���Ίe��V�F�[�_���g�p���܂��B\r\n�����G�t�F�N�g�⓮�I�e�N�X�`�����g�����G�t�F�N�g�����L���ɂȂ�A�����ڂ��h��ɂȂ�܂����A���̕��������ׂ��傫���Ȃ�܂��B");
         }
private: System::Void ck_simplebg_MouseEnter(System::Object^  sender, System::EventArgs^  e) {
           setDescription(L"�w�i�ւ̃V�F�[�_�̎g�p�𖳌������܂��B\r\n�V�F�[�_�L�����̔w�i�͏������ׂ������̂ŁA���삪�x���ꍇ�܂��͂����ς��Ă݂�Ƃ����ł��傤�B\r\n�V�F�[�_���g��Ȃ��ݒ�̏ꍇ�ω��͂���܂���B");
         }
private: System::Void lb_bloom_MouseEnter(System::Object^  sender, System::EventArgs^  e) {
           setDescription(L"�V�F�[�_�L�����́A���邢�������ڂ�����ῂ��������ɂ���G�t�F�N�g�̋����ݒ肵�܂��B\r\n�V�F�[�_���g��Ȃ��ݒ�̏ꍇ�ω��͂���܂���B");
         }
private: System::Void ed_name_MouseEnter(System::Object^  sender, System::EventArgs^  e) {
           setDescription(L"���v���C�f�[�^�⃉���L���O�ɋL�^����閼�O�ł��B");
         }
private: System::Void ck_update_MouseEnter(System::Object^  sender, System::EventArgs^  e) {
           setDescription(L"�Q�[���N������Web�o�R�ōŐV�p�b�`�̗L���𒲂ׂ܂��B");
         }
private: System::Void ck_show_fps_MouseEnter(System::Object^  sender, System::EventArgs^  e) {
           setDescription(L"�t���[�����[�g��\�����܂��B");
         }
private: System::Void ck_show_obj_MouseEnter(System::Object^  sender, System::EventArgs^  e) {
           setDescription(L"�Q�[�����̃I�u�W�F�N�g�̐���\�����܂��B");
         }
private: System::Void lb_thread_MouseEnter(System::Object^  sender, System::EventArgs^  e) {
           setDescription(L"�������������X���b�h�ɕ����邩���w�肵�܂��B\r\n�f�t�H���g�œK�ɂȂ�悤�ɂ��Ă��܂����A���삪�ُ�ɏd���Ƃ����͒������Ă݂�ƕω������邩������܂���B\r\n����CPU�̃R�A���Ɠ����̎��ł����ʂ�����܂��B");
         }
private: System::Void ed_port_MouseEnter(System::Object^  sender, System::EventArgs^  e) {
           setDescription(L"�T�[�o�[�ɂȂ鎞�g�p����|�[�g���w�肵�܂��B");
         }
private: System::Void ck_bgm_MouseEnter(System::Object^  sender, System::EventArgs^  e) {
           setDescription(L"BGM�̃I���I�t���w�肵�܂��B");
         }
private: System::Void ck_se_MouseEnter(System::Object^  sender, System::EventArgs^  e) {
           setDescription(L"���ʉ��̃I���I�t���w�肵�܂��B");
         }
private: System::Void sl_bgm_volume_MouseEnter(System::Object^  sender, System::EventArgs^  e) {
           setDescription(L"BGM�̉��ʂ��w�肵�܂��B");
         }
private: System::Void sl_se_volume_MouseEnter(System::Object^  sender, System::EventArgs^  e) {
           setDescription(L"���ʉ��̉��ʂ��w�肵�܂��B");
         }
private: System::Void cm_pad1_MouseEnter(System::Object^  sender, System::EventArgs^  e) {
           Regex^ reg = gcnew Regex(L"^�o�b�e�B���O");
           if(!reg->IsMatch(getDescription())) {
             setDescription(L"���蓖�Ă�p�b�h�̃{�^�����w�肵�܂��B");
           }
         }
private: System::Void cm_key1_MouseEnter(System::Object^  sender, System::EventArgs^  e) {
           Regex^ reg = gcnew Regex(L"^�o�b�e�B���O");
           if(!reg->IsMatch(getDescription())) {
             setDescription(L"���蓖�Ă�L�[�{�[�h�̃L�[���w�肵�܂��B");
           }
         }
private: System::Void cm_controller_MouseEnter(System::Object^  sender, System::EventArgs^  e) {
           setDescription(L"�g�p����R���g���[�����w�肵�܂��B");
         }
private: System::Void ed_threshold1_MouseEnter(System::Object^  sender, System::EventArgs^  e) {
           setDescription(L"�p�b�h�̏\���L�[���͂�臒l���w�肵�܂��B\r\n���������ĂȂ��̂ɏ\���L�[���������ςȂ��ɂ����悤�ȓ��������ꍇ�A������グ��Ɖ��P�����\��������܂��B");
         }
private: System::Void ed_threshold2_MouseEnter(System::Object^  sender, System::EventArgs^  e) {
           setDescription(L"�p�b�h�̕����]���p�̎���臒l���w�肵�܂��B");
         }
private: System::Void cm_dir_x_MouseEnter(System::Object^  sender, System::EventArgs^  e) {
           setDescription(L"�p�b�h�̕����]���p�̃X�e�B�b�N���̎����w�肵�܂��B\r\nX,Y���͈ړ��p�̂��߁A���󂵂Ă���܂��B");
         }
private: System::Void ck_hat_MouseEnter(System::Object^  sender, System::EventArgs^  e) {
           setDescription(L"�n�b�g�X�C�b�`���\���L�[�Ƃ��Ďg�p�ł���悤�ɂ��܂��B");
         }
private: System::Void bu_default_low_MouseEnter(System::Object^  sender, System::EventArgs^  e) {
           setDescription(L"�G�t�F�N�g�͕n���ł����A���̊��łƂ肠���������ݒ�ɂ��܂��B\r\n������Ԃ̐ݒ�ł��B");
         }
private: System::Void bu_default_high_MouseEnter(System::Object^  sender, System::EventArgs^  e) {
           setDescription(L"�S�G�t�F�N�g��L���ɂ��܂��B\r\nGeForce6000�n/RadeonX1000�n�N���X�ȍ~�̃r�f�I�J�[�h��ς񂾊��p�̐ݒ�ł��B");
         }
private: System::Void bu_invoke_Enter(System::Object^  sender, System::EventArgs^  e) {
           setDescription(L"�ݒ��ۑ����A�Q�[�����N�����܂��B");
         }
private: System::Void bu_default_high_Click(System::Object^  sender, System::EventArgs^  e) {
           loadDefaultConfig_High();
         }
private: System::Void bu_default_low_Click(System::Object^  sender, System::EventArgs^  e) {
           loadDefaultConfig_Low();
         }
private: System::Void bu_invoke_Click(System::Object^  sender, System::EventArgs^  e) {
           Close();

           String ^path = "exception_conflict.exe";
           if(File::Exists(path)) {
             Process::Start(path);
           }
         }

private: System::Void checkKeyBatting(System::Object^  sender, System::EventArgs^  e) {
           cli::array<System::Windows::Forms::ComboBox^>^ cb = gcnew cli::array<System::Windows::Forms::ComboBox^>(10) {
             cm_key_laser,
             cm_key_ray,
             cm_key_direction,
             cm_key_catapult,
             cm_key_rrot,
             cm_key_lrot,
             cm_key_up,
             cm_key_down,
             cm_key_right,
             cm_key_left
           };
           for(int i=0; i<9; ++i) {
             for(int j=i+1; j<10; ++j) {
               if(cb[i]->SelectedIndex==cb[j]->SelectedIndex && cb[i]->SelectedItem!=L"����") {
                 setDescription(L"�o�b�e�B���O���Ă�L�[������܂��I: "+cb[i]->SelectedItem);
                 goto loop_end;
               }
             }
           }
          setDescription(L"");
          loop_end:;
         }
private: System::Void checkButtonBatting(System::Object^  sender, System::EventArgs^  e) {
           cli::array<System::Windows::Forms::ComboBox^>^ cb = gcnew cli::array<System::Windows::Forms::ComboBox^>(7) {
             cm_pad_laser,
             cm_pad_ray,
             cm_pad_direction,
             cm_pad_catapult,
             cm_pad_rrot,
             cm_pad_lrot,
             cm_pad_menu,
           };
           for(int i=0; i<6; ++i) {
             for(int j=i+1; j<7; ++j) {
               if(cb[i]->SelectedIndex==cb[j]->SelectedIndex && cb[i]->SelectedItem!=L"����") {
                 setDescription(L"�o�b�e�B���O���Ă�{�^��������܂��I: "+cb[i]->SelectedItem);
                 goto loop_end;
               }
             }
           }
          setDescription(L"");
          loop_end:;
         }
private: System::Void cm_controller_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e) {
           updateJoystickInfo();
         }
private: System::Void ck_frameskip_MouseEnter(System::Object^  sender, System::EventArgs^  e) {
           setDescription(L"�������������Ƃ��A�`����X�L�b�v���ăQ�[���̐i�s���x��������x�ێ����܂��B");
         }
private: System::Void bu_color_MouseEnter(System::Object^  sender, System::EventArgs^  e) {
           setDescription(L"���@�̐F���w�肵�܂��B");
         }
private: System::Void bu_color_Click(System::Object^  sender, System::EventArgs^  e) {
           ColorDialog^ dialog = gcnew ColorDialog;
           if(dialog->ShowDialog()==System::Windows::Forms::DialogResult::OK) {
             pn_color_show->BackColor = dialog->Color;
           }
         }
private: System::Void bu_reset_keyassign_Click(System::Object^  sender, System::EventArgs^  e) {
           loadDefaultKeyAssign();
         }
private: System::Void bu_reset_keyassign_MouseEnter(System::Object^  sender, System::EventArgs^  e) {
           setDescription(L"�L�[�R���t�B�O�������ݒ�ɖ߂��܂��B");
         }
};


}

