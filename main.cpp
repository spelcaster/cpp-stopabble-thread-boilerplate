#include <future>
#include <thread>
#include <chrono>
#include <iostream>
#include <memory>

class Thread
{
protected:
  int mSleepTimer;
  std::unique_ptr< std::thread > mThread;
  std::promise< void > mStop;
  std::future< void > mStopObj;
  void run();
  bool hasStopped();
  virtual void work() = 0;

public:
  Thread();
  virtual ~Thread();

  void Start();
  void Stop();
  void Join();
  void SetSleepTimer(uint32_t timer);

private:
  std::condition_variable mStopCondition;
  std::mutex mStopMtx;
};

Thread::Thread() : mSleepTimer(1000)
{
  mStopObj = mStop.get_future();
}

Thread::~Thread()
{
  Join();
}

void Thread::SetSleepTimer(uint32_t timer)
{
  mSleepTimer = timer;
}

void Thread::Start()
{
  std::cout << "Starting the thread..." << std::endl;
  mThread = std::make_unique< std::thread >(&Thread::run, this);
}

void Thread::Stop()
{
  if (hasStopped())
    return;

  mStop.set_value();

  std::lock_guard<std::mutex> lck(mStopMtx);
  mStopCondition.notify_one();
}

void Thread::Join()
{
  if (mThread->joinable())
  {
    mThread->join();
  }
}

bool Thread::hasStopped()
{
  auto stopStatus = mStopObj.wait_for(std::chrono::milliseconds(1));
  return stopStatus != std::future_status::timeout;
}

void Thread::run()
{
  std::unique_lock<std::mutex> lck(mStopMtx);

  while (!hasStopped())
  {
    work();
    mStopCondition.wait_for(lck, std::chrono::milliseconds(mSleepTimer));
  }

  std::cout << "The thread has stopped..." << std::endl;
}

class MyThread : public Thread
{
protected:
  void work()
  {
    std::cout << "The thread is working..." << std::endl;
  }
};

int main(int argc, char *argv[])
{
  std::unique_ptr< MyThread > pThread = std::make_unique< MyThread >();
  pThread->SetSleepTimer(60000);

  pThread->Start();

  for (int i = 0; i < 5; ++i) {
    std::cout << "The main thread is working..." << std::endl;
    std::cout << i << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(i));
  }
  pThread->Stop();

  return 0;
}

