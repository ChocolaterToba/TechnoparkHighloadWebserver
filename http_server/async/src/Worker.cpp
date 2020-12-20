#include <thread>
#include <mutex>
#include <functional>
#include <memory>
#include <vector>
#include <queue>

#include "Worker.hpp"
#include "Task.hpp"
#include "msleep.hpp"

Worker::Worker(std::queue<Task>& tasks,
               std::shared_ptr<std::mutex> tasksMutex) :
        tasks(tasks),
        tasksMutex(tasksMutex),
        state(NoTask),
        stop(true) {}

Worker::Worker(Worker&& other) :
        Worker(other.tasks, other.tasksMutex) {
    other.Stop();
}

Worker& Worker::operator=(Worker&& other) {
    Stop();
    tasks = other.tasks;
    tasksMutex = other.tasksMutex;
    currentTask = Task();
    state = NoTask;
    other.Stop();
    return *this;
}

Worker::~Worker() {
    Stop();
}

void Worker::TakeNewTask() {
    if (state == NoTask) {
        while (!stop) {
            if (!tasks.empty()) {
                if (tasksMutex->try_lock()) {
                    currentTask = std::move(tasks.front());
                    tasks.pop();
                    tasksMutex->unlock();
                    state = TaskRecieved;
                    break;
                } else {
                    msleep(30);
                }
            } else {
                msleep(30);
            }
        }
    } else {
        throw std::runtime_error(std::string(
            "Worker: TakeNewTask: state is not NoTask!"));
    }
}

void Worker::RunPreFunc() {
    if (state == TaskRecieved) {
        state = PreFuncRunning;
        currentTask.SetMainFunc(
            currentTask.GetPreFunc()(headers, data,
                                     currentTask.GetInput()));
        state = PreFuncRan;
    } else {
        throw std::runtime_error(std::string(
            "Worker: RunPreFunc: state is not TaskRecieved!"));
    }
}
void Worker::RunMainFunc() {
    if (state == PreFuncRan) {
        state = MainFuncRunning;
        currentTask.GetMainFunc()(headers, data,
                                  currentTask.GetInput(),
                                  currentTask.GetOutput());
        state = MainFuncRan;
    } else {
        throw std::runtime_error(std::string(
            "Worker: RunMainFunc: state is not PreFuncRan!"));
    }
}
void Worker::RunPostFunc() {
    if (state == MainFuncRan) {
        state = PostFuncRunning;
        currentTask.GetPostFunc()(headers, data, currentTask.GetOutput());
        state = NoTask;
    } else {
        throw std::runtime_error(std::string(
            "Worker: RunPostFunc: state is not MainFuncRan!"));
    }
}

void Worker::Start() {
    if (stop) {
        stop = false;
        state = NoTask;
        workerThread = std::thread(&Worker::Loop, this);
    }
}

void Worker::Loop() {
    while (!stop) {
        TakeNewTask();
        RunPreFunc();
        currentTask.SetOutput(currentTask.GetInput());  // TODO: remove this when satisfying next TODO
        RunMainFunc();   // TODO: async clients logic
        RunPostFunc();
    }
}

void Worker::Stop() {
    if (!stop) {
        stop = true;
        workerThread.join();
        state = NoTask;

        headers.clear();
        data.clear();

    }
}
