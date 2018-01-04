#include <future>
#include <thread>
#include <chrono>
#include <iostream>
#include <memory>

class Thread
{
protected:
  std::unique_ptr< std::thread > mThread;
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
};

Thread::Thread()
{
  mStopObj = mStop.get_future();
}

Thread::~Thread()
{
  Join();
}

void Thread::Start()
{
  std::cout << "Starting the thread..." << std::endl;
  mThread = std::make_unique< std::thread >(&Thread::run, this);
}

void Thread::Stop()
{
  if (!shouldStop())
  {
    mStop.set_value();
  }
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
  while (!shouldStop())
  {
    work();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
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

  pThread->Start();

  std::this_thread::sleep_for(std::chrono::seconds(5));

  return 0;
}

