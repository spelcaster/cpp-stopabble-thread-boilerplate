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
  std::condition_variable mStopCondition;
  std::promise< void > mStop;
  std::future< void > mStopObj;
  void run();
  bool shouldStop();
  virtual void work() = 0;

public:
  Thread();
  virtual ~Thread();

  void Start();
  void Stop();
  void Join();
  void SetSleepTimer(uint32_t timer);

private:
  std::mutex mMtx;
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
  if (shouldStop())
    return;

  mStop.set_value();

  std::lock_guard<std::mutex> lck(mMtx);
  mStopCondition.notify_one();
}

void Thread::Join()
{
  Stop();

  if (mThread->joinable())
  {
    mThread->join();
  }
}

bool Thread::shouldStop()
{
  auto stopStatus = mStopObj.wait_for(std::chrono::milliseconds(1));
  return stopStatus != std::future_status::timeout;
}

void Thread::run()
{
  std::unique_lock<std::mutex> lck(mMtx);

  while (!shouldStop())
  {
    work();

    auto stopCond = mStopCondition.wait_for(
      lck, std::chrono::milliseconds(mSleepTimer));

    if (stopCond != std::cv_status::timeout)
    {
      break;
    }
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

