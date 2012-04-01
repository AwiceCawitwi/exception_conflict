#ifndef ist_scheduler
#define ist_scheduler

#ifndef _WIN32_WINNT
  #define _WIN32_WINNT 0x0500
  #define WINVER 0x0500
#endif

#include <vector>
#include <list>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

namespace ist {

class Task;
typedef boost::shared_ptr<Task> task_ptr;


namespace impl {
  class TaskThread;
  class TaskQueue;
}

class Task
{
friend class Scheduler;
friend class impl::TaskQueue;
friend class impl::TaskThread;
private:
  bool m_finished;
  boost::thread::id m_affinity;

  void initialize();
  void finalize();

public:
  Task();
  virtual ~Task();

  void setAffinity(boost::thread::id affinity);
  boost::thread::id getAffinity() const;
  bool isFinished() const;

  virtual void operator()()=0;
};


namespace impl {

  class TaskQueue
  {
  private:
    typedef std::list<task_ptr> task_cont;
    task_cont m_tasks;
    size_t m_num_waiting;
    boost::mutex m_suspender;
    boost::condition_variable m_cond;

  public:
    TaskQueue();
    size_t getNumWaiting() const;
    bool empty();
    void push(task_ptr t);

    template<class Iter>
    void push(Iter begin, Iter end)
    {
      boost::lock_guard<boost::mutex> lock(m_suspender);
      for(; begin!=end; ++begin) {
        task_ptr t = boost::static_pointer_cast<Task>(*begin);
        t->initialize();
        m_tasks.push_back(t);
      }

      notify();
    }

    task_ptr pop();
    task_ptr waitForNewTask();
    void notify();
  };



  class TaskThread
  {
  private:
    TaskQueue& m_task_queue;
    bool m_stop_flag;
    boost::shared_ptr<boost::thread> m_thread;
    boost::condition_variable m_cond;
    task_ptr m_current_task;

  public:
    TaskThread(TaskQueue& tq, int processor);
    ~TaskThread();
    void stop();
    boost::thread::id getID() const;
    void operator()();
  };
}


class Scheduler
{
private:
  typedef boost::shared_ptr<impl::TaskThread> thread_ptr;
  typedef std::vector<thread_ptr> thread_cont;
  impl::TaskQueue m_task_queue;
  thread_cont m_threads;
  static Scheduler *s_instance;

public:
  static Scheduler* instance();

  // Singleton�B�����C���X�^���X��낤�Ƃ���Ɨ�O������B 
  // ������0�ȉ��̏ꍇCPU�̐��Ɏ��������B 
  Scheduler(int num_thread=0);

  // ���ݏ������̃^�X�N�̊�����҂��Ă����~����B 
  // (�^�X�N�L���[����ɂȂ�̂�҂��Ȃ�) 
  ~Scheduler();

  // �S�^�X�N�̊�����҂B�^�X�N�L���[����ł͂Ȃ��ꍇ�A�Ăяo�����X���b�h���^�X�N�����ɉ����B 
  // �^�X�N������ĂԂƉi�v��~����̂ł������_���B 
  void waitForAll();

  // �w��^�X�N�̊�����҂B�^�X�N�L���[����ł͂Ȃ��ꍇ�A�Ăяo�����X���b�h���^�X�N�����ɉ����B 
  void waitFor(task_ptr task);

  // �͈͎w��o�[�W���� 
  template<class Iter>
  void waitFor(Iter begin, Iter end)
  {
    while(true) {
      bool finished = true;
      for(Iter i=begin; i!=end; ++i) {
        if(!(*i)->isFinished()) {
          finished = false;
          break;
        }
      }

      if(finished) {
        break;
      }
      else if(task_ptr t = m_task_queue.pop()) {
        (*t)();
        t->finalize();
      }
      else {
        boost::this_thread::yield();
      }
    }
  }


  // �^�X�N�̃X�P�W���[�����O���s���B 
  void schedule(task_ptr task);

  // �͈͎w��o�[�W���� 
  template<class Iter>
  void schedule(Iter begin, Iter end)
  {
    m_task_queue.push(begin, end);
  }


  size_t getThreadCount() const;
  boost::thread::id getThreadID(size_t i) const;


  // �ȉ��f�o�b�O�p�B�ʏ�͎g������_���B 
  impl::TaskQueue& getTaskQueue();
};

} // namespace ist
#endif // ist_scheduler 
