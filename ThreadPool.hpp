#include<queue>

#include"Task.hpp"
#define NUM 15
class ThreadPool
{
  private:
    std::queue<Task> task_queue;//线程池队列
    int num;//创建线程数量
    bool stop;//线程是否运行
    pthread_mutex_t lock;//锁
    pthread_cond_t cond;//条件变量
    static ThreadPool*single_instance;//单例模式

    ThreadPool(int _num=NUM)
    {
      num=_num;
      pthread_mutex_init(&lock,nullptr);
      pthread_cond_init(&cond,nullptr);
    }

    ThreadPool(const ThreadPool&){};
  public:
  static ThreadPool*getinstance()
  {
    static pthread_mutex_t  mymutex=PTHREAD_MUTEX_INITIALIZER;
    if(single_instance==nullptr)
    {
      pthread_mutex_lock(&mymutex);
      if(single_instance==nullptr)
      {
        single_instance=new ThreadPool();
        single_instance->InitThreadPool();
      }
      pthread_mutex_unlock(&mymutex);
    }
    return single_instance;
  }
  

    bool IsStop()
    {
      return stop;
    }

    
    bool TaskQueueIsEmpty()
    {
      return task_queue.size()==0;
    }


    void ThreadWait()
    {
      pthread_cond_wait(&cond,&lock);
    }
    

    void ThreadWakeUp()
    {
      pthread_cond_signal(&cond);
    }

    void Lock()
    {
      pthread_mutex_lock(&lock);
    }

    void Unlock()
    {
      pthread_mutex_unlock(&lock);
    }

    static void*ThreadToutine(void*args)
    {
      ThreadPool*tp=(ThreadPool*)args;
      while(true)
      {
        Task t;
        tp->Lock();
        while(tp->TaskQueueIsEmpty())
        {
          tp->ThreadWait();
        }
        tp->PopTask(t);
        tp->Unlock();
        t.Process();
      }
    }


    bool InitThreadPool()
    {
      for(int i=0;i<num;i++)
      {
        pthread_t tid;
        if((pthread_create(&tid,nullptr,ThreadToutine,this))!=0)
        {
          LOG(FATAL,"Create ThreadPool Error!");
          return false;
        }
      }
      LOG(INFO,"Create ThreadPool Success!");
      return true;
    }

    void Push(const Task&task)
    {
      Lock();
      task_queue.push(task);
      Unlock();
      ThreadWakeUp();
    }

    void PopTask(Task&t)
    {
      t=task_queue.front();
      task_queue.pop();
  
    }

    ~ThreadPool()
    {
      pthread_mutex_destroy(&lock);
      pthread_cond_destroy(&cond);
    }
};

ThreadPool*ThreadPool::single_instance=nullptr;
