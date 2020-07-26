#pragma once

#include <queue>

#include "skse64/gamethreads.h"

class ISafeTasks
{
public:

    void AddTask(TaskDelegate*task)
    {
        m_taskLock.Enter();
        m_taskQueue.push(task);
        m_taskLock.Leave();
    }

    bool IsTaskQueueEmpty()
    {
        m_taskLock.Enter();
        bool r = m_taskQueue.size() == 0;
        m_taskLock.Leave();
        return r;
    }

    void ProcessTasks()
    {
        while (!IsTaskQueueEmpty())
        {
            m_taskLock.Enter();
            auto task = m_taskQueue.front();
            m_taskQueue.pop();
            m_taskLock.Leave();

            task->Run();
            task->Dispose();
        }
    }

    void ProcessTasksUnsafe()
    {
        m_taskLock.Enter();

        while (m_taskQueue.size())
        {
            auto task = m_taskQueue.front();
            m_taskQueue.pop();

            task->Run();
            task->Dispose();
        }

        m_taskLock.Leave();
    }

    void ClearTasks()
    {
        m_taskLock.Enter();
        while (m_taskQueue.size()) {
            auto task = m_taskQueue.front();
            m_taskQueue.pop();

            task->Dispose();
        }
        m_taskLock.Leave();
    }

private:
    std::queue<TaskDelegate*> m_taskQueue;
    ICriticalSection m_taskLock;

};
