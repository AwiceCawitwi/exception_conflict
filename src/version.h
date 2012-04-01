#ifndef version_h
#define version_h

#ifndef _WIN32_WINNT
  #define _WIN32_WINNT 0x0500
#endif
#ifndef WINVER
  #define WINVER 0x0500
#endif

#define EXCEPTION_REPLAY_VERSION 106
#define EXCEPTION_VERSION 110

#ifdef EXCEPTION_DEBUG
  #define EXCEPTION_ENABLE_PROFILE          // �v���t�@�C���_�C�A���O�̗L�� 
  #define EXCEPTION_ENABLE_STATE_SAVE       // �X�e�[�g�Z�[�u�@�\�̗L�� 
  #define EXCEPTION_ENABLE_DATA_RESCUE      // Win32Exception����񂾎������v���C�Ƃ���ۑ�����B/EHa�K�{�B�x���Ȃ� 
  #define EXCEPTION_ENABLE_RUNTIME_CHECK    // �e�탉���^�C���`�F�b�N�̗L���B���Ȃ�x���Ȃ� 
  #define EXCEPTION_HOST "i-saint.skr.jp"
  #define EXCEPTION_HOST_PATH "/exception_conflict/test/"
#else
  #define EXCEPTION_HOST "i-saint.skr.jp"
  #define EXCEPTION_HOST_PATH "/exception_conflict/"
#endif

#ifdef WIN32
  #define EXCEPTION_ENABLE_NETRANKING // �l�b�g�����L���O�̗L�� 
  #define EXCEPTION_ENABLE_NETPLAY    // �l�b�g�z�������l�v���C�̗L�� 
  #define EXCEPTION_ENABLE_NETUPDATE // �����A�b�v�f�[�g�̗L�� 
#endif

#define EXCEPTION_ENABLE_REPLAY_DUMP
#define EXCEPTION_ENABLE_DEDICATED_SERVER

#endif
