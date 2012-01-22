/* 
 * Copyright © 2012 Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Benjamin Segovia <benjamin.segovia@intel.com>
 */

#include "sys/tasking.hpp"
#include "sys/tasking_utility.hpp"
#include "sys/ref.hpp"
#include "sys/thread.hpp"
#include "sys/mutex.hpp"
#include "sys/sysinfo.hpp"
#include "math/random.hpp"

#include "utest/utest.hpp"

#define START_UTEST(TEST_NAME)                          \
void TEST_NAME(void)                                    \
{                                                       \
  std::cout << std::endl << "starting " <<              \
              #TEST_NAME << std::endl;

#define END_UTEST(TEST_NAME)                            \
  std::cout << "ending " << #TEST_NAME << std::endl;    \
}

using namespace pf;

///////////////////////////////////////////////////////////////////////////////
// Very simple test which basically does nothing
///////////////////////////////////////////////////////////////////////////////
class TaskDone : public Task {
public:
  virtual Task* run(void) {
    TaskingSystemInterruptMain();
    return NULL;
  }
};

START_UTEST(TestDummy)
  Task *done = GBE_NEW(TaskDone);
  Task *nothing = GBE_NEW(TaskDummy);
  nothing->starts(done);
  done->scheduled();
  nothing->scheduled();
  TaskingSystemEnter();
END_UTEST(TestDummy)

///////////////////////////////////////////////////////////////////////////////
// Simplest taskset test. An array is filled by each worker
///////////////////////////////////////////////////////////////////////////////
class TaskSetSimple : public TaskSet {
public:
  INLINE TaskSetSimple(size_t elemNum, uint32 *array_) :
    TaskSet(elemNum), array(array_) {}
  virtual void run(size_t elemID) { array[elemID] = 1u; }
  uint32 *array;
};

START_UTEST(TestTaskSet)
  const size_t elemNum = 1 << 20;
  uint32 *array = GBE_NEW_ARRAY(uint32, elemNum);
  for (size_t i = 0; i < elemNum; ++i) array[i] = 0;
  double t = getSeconds();
  Task *done = GBE_NEW(TaskDone);
  Task *taskSet = GBE_NEW(TaskSetSimple, elemNum, array);
  taskSet->starts(done);
  done->scheduled();
  taskSet->scheduled();
  TaskingSystemEnter();
  t = getSeconds() - t;
  std::cout << t * 1000. << " ms" << std::endl;
  for (size_t i = 0; i < elemNum; ++i)
    FATAL_IF(array[i] == 0, "TestTaskSet failed");
  GBE_DELETE_ARRAY(array);
END_UTEST(TestTaskSet)

///////////////////////////////////////////////////////////////////////////////
// We create a binary tree of tasks here. Each task spawn a two children upto a
// given maximum level. Then, a atomic value is updated per leaf. In that test,
// all tasks complete the ROOT directly
///////////////////////////////////////////////////////////////////////////////
enum { maxLevel = 20u };

/*! One node task per node in the tree. Task completes the root */
class TaskNode : public Task {
public:
  INLINE TaskNode(Atomic &value_, uint32 lvl_, Task *root_=NULL) :
    value(value_), lvl(lvl_) {
    this->root = root_ == NULL ? this : root_;
  }
  virtual Task* run(void);
  Atomic &value;
  Task *root;
  uint32 lvl;
};

Task* TaskNode::run(void) {
  if (this->lvl == maxLevel)
    this->value++;
  else {
    Task *left  = GBE_NEW(TaskNode, this->value, this->lvl+1, this->root);
    Task *right = GBE_NEW(TaskNode, this->value, this->lvl+1, this->root);
    left->ends(this->root);
    right->ends(this->root);
    left->scheduled();
    right->scheduled();
  }
  return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// Same as TaskNode but we use a continuation passing style strategy to improve
// the system throughtput
///////////////////////////////////////////////////////////////////////////////

/*! One node task per node in the tree. Task completes the root */
class TaskNodeOpt : public Task {
public:
  INLINE TaskNodeOpt(Atomic &value_, uint32 lvl_, Task *root_=NULL) :
    value(value_), lvl(lvl_) {
    this->root = root_ == NULL ? this : root_;
  }
  virtual Task* run(void);
  Atomic &value;
  Task *root;
  uint32 lvl;
};

Task* TaskNodeOpt::run(void) {
  if (this->lvl == maxLevel) {
    this->value++;
    return NULL;
  } else {
    Task *left  = GBE_NEW(TaskNode, this->value, this->lvl+1, this->root);
    Task *right = GBE_NEW(TaskNode, this->value, this->lvl+1, this->root);
    left->ends(this->root);
    right->ends(this->root);
    left->scheduled();
    return right;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Same as TaskNode but here each task completes its parent task directly. This
// stresses the completion system but strongly limits cache line contention
///////////////////////////////////////////////////////////////////////////////

/*! One node task per node in the tree. Task completes its parent */
class TaskCascadeNode : public Task {
public:
  INLINE TaskCascadeNode(Atomic &value_, uint32 lvl_, Task *root_=NULL) :
    value(value_), lvl(lvl_) {}
  virtual Task* run(void);
  Atomic &value;
  uint32 lvl;
};

Task *TaskCascadeNode::run(void) {
  if (this->lvl == maxLevel)
    this->value++;
  else {
    Task *left  = GBE_NEW(TaskCascadeNode, this->value, this->lvl+1);
    Task *right = GBE_NEW(TaskCascadeNode, this->value, this->lvl+1);
    left->ends(this);
    right->ends(this);
    left->scheduled();
    right->scheduled();
  }
  return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// Same as TaskCascadeNode but with continuation passing style tasks
///////////////////////////////////////////////////////////////////////////////
class TaskCascadeNodeOpt : public Task {
public:
  INLINE TaskCascadeNodeOpt(Atomic &value_, uint32 lvl_, Task *root_=NULL) :
    value(value_), lvl(lvl_) {}
  virtual Task* run(void);
  Atomic &value;
  uint32 lvl;
};

Task *TaskCascadeNodeOpt::run(void) {
  if (this->lvl == maxLevel) {
    this->value++;
    return NULL;
  } else {
    Task *left  = GBE_NEW(TaskCascadeNode, this->value, this->lvl+1);
    Task *right = GBE_NEW(TaskCascadeNode, this->value, this->lvl+1);
    left->ends(this);
    right->ends(this);
    left->scheduled();
    return right;
  }
}

/*! For all tree tests */
template<typename NodeType>
START_UTEST(TestTree)
  Atomic value(0u);
  std::cout << "nodeNum = " << (2 << maxLevel) - 1 << std::endl;
  double t = getSeconds();
  Task *done = GBE_NEW(TaskDone);
  Task *root = GBE_NEW(NodeType, value, 0);
  root->starts(done);
  done->scheduled();
  root->scheduled();
  TaskingSystemEnter();
  t = getSeconds() - t;
  std::cout << t * 1000. << " ms" << std::endl;
  FATAL_IF(value != (1 << maxLevel), "TestTree failed");
END_UTEST(TestTree)

///////////////////////////////////////////////////////////////////////////////
// We try to stress the internal allocator here
///////////////////////////////////////////////////////////////////////////////
class TaskAllocate : public TaskSet {
public:
  TaskAllocate(size_t elemNum) : TaskSet(elemNum) {}
  virtual void run(size_t elemID);
  enum { allocNum = 1 << 10 };
  enum { iterNum = 1 << 5 };
};

void TaskAllocate::run(size_t elemID) {
  Task *tasks[allocNum];
  for (int j = 0; j < iterNum; ++j) {
    const int taskNum = rand() % allocNum;
    for (int i = 0; i < taskNum; ++i) tasks[i] = GBE_NEW(TaskDummy);
    for (int i = 0; i < taskNum; ++i) GBE_DELETE(tasks[i]);
  }
}

START_UTEST(TestAllocator)
  Task *done = GBE_NEW(TaskDone);
  Task *allocate = GBE_NEW(TaskAllocate, 1 << 10);
  double t = getSeconds();
  allocate->starts(done);
  done->scheduled();
  allocate->scheduled();
  TaskingSystemEnter();
  t = getSeconds() - t;
  std::cout << t * 1000. << " ms" << std::endl;
END_UTEST(TestAllocator)

///////////////////////////////////////////////////////////////////////////////
// We are making the queue full to make the system recurse to empty it
///////////////////////////////////////////////////////////////////////////////
class TaskFull : public Task {
public:
  enum { taskToSpawn = 1u << 16u };
  TaskFull(const char *name, Atomic &counter, int lvl = 0) :
    Task(name), counter(counter), lvl(lvl) {}
  virtual Task* run(void) {
    if (lvl == 0)
      for (size_t i = 0; i < taskToSpawn; ++i) {
        Task *task = GBE_NEW(TaskFull, "TaskFullLvl1", counter, 1);
        task->ends(this);
        task->scheduled();
      }
    else
      counter++;
    return NULL;
  }
  Atomic &counter;
  int lvl;
};

START_UTEST(TestFullQueue)
  Atomic counter(0u);
  double t = getSeconds();
  Task *done = GBE_NEW(TaskDone);
  for (size_t i = 0; i < 64; ++i) {
    Task *task = GBE_NEW(TaskFull, "TaskFull", counter);
    task->starts(done);
    task->scheduled();
  }
  done->scheduled();
  TaskingSystemEnter();
  t = getSeconds() - t;
  std::cout << t * 1000. << " ms" << std::endl;
  FATAL_IF (counter != 64 * TaskFull::taskToSpawn, "TestFullQueue failed");
END_UTEST(TestFullQueue)

///////////////////////////////////////////////////////////////////////////////
// We spawn a lot of affinity jobs to saturate the affinity queues
///////////////////////////////////////////////////////////////////////////////
class TaskAffinity : public Task {
public:
  enum { taskToSpawn = 2048u };
  TaskAffinity(Task *done, Atomic &counter, int lvl = 0) :
    Task("TaskAffinity"), counter(counter), lvl(lvl) {}
  virtual Task *run(void) {
    if (lvl == 1)
      counter++;
    else {
      const uint32 threadNum = TaskingSystemGetThreadNum();
      for (uint32 i = 0; i < taskToSpawn; ++i) {
        Task *task = GBE_NEW(TaskAffinity, done.ptr, counter, 1);
        task->setAffinity(i % threadNum);
        task->ends(this);
        task->scheduled();
      }
    }
    return NULL;
  }
  Atomic &counter;
  Ref<Task> done;
  int lvl;
};

START_UTEST(TestAffinity)
  enum { batchNum = 512 };
  for (int i = 0; i < 8; ++i) {
    Atomic counter(0u);
    double t = getSeconds();
    Ref<Task> done = GBE_NEW(TaskDone);
    for (size_t i = 0; i < batchNum; ++i) {
      Task *task = GBE_NEW(TaskAffinity, done.ptr, counter);
      task->starts(done.ptr);
      task->scheduled();
    }
    done->scheduled();
    TaskingSystemEnter();
    t = getSeconds() - t;
    std::cout << t * 1000. << " ms" << std::endl;
    std::cout << counter << std::endl;
    FATAL_IF (counter != batchNum * TaskAffinity::taskToSpawn, "TestAffinity failed");
  }
END_UTEST(TestAffinity)

///////////////////////////////////////////////////////////////////////////////
// Exponential Fibonnaci to stress the task spawning and the completions
///////////////////////////////////////////////////////////////////////////////
static Atomic fiboNum(0u);
class TaskFiboSpawn : public Task {
public:
  TaskFiboSpawn(uint64 rank, uint64 *root = NULL) :
    Task("TaskFiboSpawn"), rank(rank), root(root) {fiboNum++;}
  virtual Task* run(void);
  uint64 rank, sumLeft, sumRight;
  uint64 *root;
};

class FiboSumTask : public Task {
public:
  FiboSumTask(TaskFiboSpawn *fibo) : Task("FiboSumTask"), fibo(fibo) {}
  virtual Task* run(void);
  TaskFiboSpawn *fibo;
};

Task *TaskFiboSpawn::run(void) {
  if (rank > 1) {
    TaskFiboSpawn *left = GBE_NEW(TaskFiboSpawn, rank-1, &this->sumLeft);
    TaskFiboSpawn *right = GBE_NEW(TaskFiboSpawn, rank-2, &this->sumRight);
    FiboSumTask *sum = GBE_NEW(FiboSumTask, this);
    left->starts(sum);
    right->starts(sum);
    sum->ends(this);
    sum->scheduled();
    left->scheduled();
    return right;
  } else if (rank == 1) {
    if (root) *root = 1;
    return NULL;
  } else {
    if (root) *root = 0;
    return NULL;
  }
}

Task *FiboSumTask::run(void) {
  assert(fibo);
  if (fibo->root) *fibo->root = fibo->sumLeft + fibo->sumRight;
  return NULL;
}

static uint64 fiboLinear(uint64 rank)
{
  uint64 rn0 = 0, rn1 = 1;
  if (rank == 0) return rn0;
  if (rank == 1) return rn1;
  for (uint64 i = 2; i <= rank; ++i) {
    uint64 sum = rn0 + rn1;
    rn0 = rn1;
    rn1 = sum;
  }
  return rn1;
}

START_UTEST(TestFibo)
{
  const uint64 rank = rand() % 32;
  uint64 sum;
  double t = getSeconds();
  fiboNum = 0u;
  Ref<TaskFiboSpawn> fibo = GBE_NEW(TaskFiboSpawn, rank, &sum);
  Task *done = GBE_NEW(TaskDone);
  fibo->starts(done);
  fibo->scheduled();
  done->scheduled();
  TaskingSystemEnter();
  t = getSeconds() - t;
  std::cout << t * 1000. << " ms" << std::endl;
  std::cout << "Fibonacci Task Num: "<< fiboNum << std::endl;
  FATAL_IF (sum != fiboLinear(rank), "TestFibonacci failed");
}
END_UTEST(TestFibo)

///////////////////////////////////////////////////////////////////////////////
// Task with multiple dependencies
///////////////////////////////////////////////////////////////////////////////
class TaskMultiTrigger : public Task,
                         public MultiDependencyPolicy<TaskMultiTrigger>
{
public:
  TaskMultiTrigger(int32 *valueToSet) : valueToSet(valueToSet) {}
  virtual Task *run(void) { *valueToSet = 1; return NULL; }
private:
  int32 *valueToSet;
};

class TaskTriggered : public Task
{
public:
  TaskTriggered(const int32 *valueToRead, Atomic32 &dst) :
    valueToRead(valueToRead), dst(dst) {}
  virtual Task *run(void) { dst += *valueToRead; return NULL; }
private:
  const int32 *valueToRead;
  Atomic32 &dst;
};

START_UTEST(TestMultiDependency)
{
  static const uint32 multiTaskToSpawn = 512;
  static const uint32 triggeredTaskToSpawn = 512;
  static const uint32 valueToSetNum = multiTaskToSpawn;

  int32 *toSet = GBE_NEW_ARRAY(int32, valueToSetNum);
  Atomic32 dst(0);
  Task *doneTask = GBE_NEW(TaskDone);
  for (uint32 i = 0; i < valueToSetNum; ++i) toSet[i] = 0;
  for (uint32 i = 0; i < multiTaskToSpawn; ++i) {
    Ref<TaskMultiTrigger> task = GBE_NEW(TaskMultiTrigger, toSet + i);
    for (uint32 j = 0; j < triggeredTaskToSpawn; ++j) {
      Ref<Task> triggered = GBE_NEW(TaskTriggered, toSet + i, dst);
      Ref<Task> dummy = GBE_NEW(TaskDummy);
      task->multiStarts(dummy);
      dummy->starts(triggered);
      triggered->starts(doneTask);
      triggered->scheduled();
      dummy->scheduled();
    }
    task->scheduled();
  }
  doneTask->scheduled();
  TaskingSystemEnter();
  GBE_DELETE_ARRAY(toSet);
  const uint32 result = dst;
  std::cout << "result: " << result << std::endl;
  FATAL_IF(result != multiTaskToSpawn * triggeredTaskToSpawn,
           "MultiDependency failed");
}
END_UTEST(TestMultiDependency)

START_UTEST(TestMultiDependencyTwoStage)
{
  static const uint32 multiTaskToSpawn = 512;
  static const uint32 triggeredTaskToSpawn = 512;
  static const uint32 valueToSetNum = multiTaskToSpawn;

  int32 *toSet = GBE_NEW_ARRAY(int32, valueToSetNum);
  Atomic32 dst(0);
  Task *doneTask = GBE_NEW(TaskDone);
  for (uint32 i = 0; i < valueToSetNum; ++i) toSet[i] = 0;
  for (uint32 i = 0; i < multiTaskToSpawn; ++i) {
    Ref<TaskMultiTrigger> task = GBE_NEW(TaskMultiTrigger, toSet + i);
    for (uint32 j = 0; j < triggeredTaskToSpawn; ++j) {
      Ref<Task> triggered = GBE_NEW(TaskTriggered, toSet + i, dst);
      task->multiStarts(triggered);
      triggered->starts(doneTask);
      triggered->scheduled();
    }
    task->scheduled();
  }
  doneTask->scheduled();
  TaskingSystemEnter();
  GBE_DELETE_ARRAY(toSet);
  const uint32 result = dst;
  std::cout << "result: " << result << std::endl;
  FATAL_IF(result != multiTaskToSpawn * triggeredTaskToSpawn,
           "MultiDependency failed");
}
END_UTEST(TestMultiDependencyTwoStage)

START_UTEST(TestMultiDependencyRandomStart)
{
  static const uint32 multiTaskToSpawn = 512;
  static const uint32 triggeredTaskToSpawn = 512;
  static const uint32 valueToSetNum = multiTaskToSpawn;
  static const uint32 repeatNum = 8;
  Random rand;
  for (uint32 i = 0; i < repeatNum; ++i) {
    int32 *toSet = GBE_NEW_ARRAY(int32, valueToSetNum);
    Atomic32 dst(0);
    Ref<Task> doneTask = GBE_NEW(TaskDone);
    for (uint32 i = 0; i < valueToSetNum; ++i) toSet[i] = 0;
    for (uint32 i = 0; i < multiTaskToSpawn; ++i) {
      Ref<TaskMultiTrigger> task = GBE_NEW(TaskMultiTrigger, toSet + i);
      bool isScheduled = false;
      for (uint32 j = 0; j < triggeredTaskToSpawn; ++j) {
        Ref<Task> triggered = GBE_NEW(TaskTriggered, toSet + i, dst);
        task->multiStarts(triggered);
        triggered->starts(doneTask);
        triggered->scheduled();
        if (rand.getFloat() < 0.8 && isScheduled == false) {
          task->scheduled();
          isScheduled = true;
        }
      }
      if (isScheduled == false) task->scheduled();
    }
    doneTask->scheduled();
    TaskingSystemEnter();
    GBE_DELETE_ARRAY(toSet);
    const uint32 result = dst;
    std::cout << "result: " << result << std::endl;
    FATAL_IF(result != multiTaskToSpawn * triggeredTaskToSpawn,
             "MultiDependencyRandomStart failed");
  }
}
END_UTEST(TestMultiDependencyRandomStart)

///////////////////////////////////////////////////////////////////////////////
// Test tasking lock and unlock
///////////////////////////////////////////////////////////////////////////////
class TaskLockUnlock : public Task
{
public:
  TaskLockUnlock(int32 *shared) : shared(shared) {}
  virtual Task *run(void) {
    TaskingSystemLock();
    *shared = *shared + 1;
    TaskingSystemUnlock();
    return NULL;
  }
  int32 *shared;
};

START_UTEST(TestLockUnlock)
{
  static const uint32 taskNum = 1024;
  int32 shared = 0;
  Ref<Task> doneTask = GBE_NEW(TaskDone);
  for (uint32 i = 0; i < taskNum; ++i) {
    Task *updateTask = GBE_NEW(TaskLockUnlock, &shared);
    updateTask->starts(doneTask);
    updateTask->scheduled();
  }
  doneTask->scheduled();
  TaskingSystemEnter();
  FATAL_IF(shared != int32(taskNum), "TestLockUnlock failed");
}
END_UTEST(TestLockUnlock)

///////////////////////////////////////////////////////////////////////////////
// Test tasking profiler
///////////////////////////////////////////////////////////////////////////////
#if GBE_TASK_PROFILER
class UTestProfiler : public TaskProfiler
{
public:
  UTestProfiler(void) : 
    sleepNum(0), wakeUpNum(0),
    lockNum(0), unlockNum(0),
    runStartNum(0), runEndNum(0), endNum(0) {}
  virtual void onSleep(uint32 threadID)  {sleepNum++;}
  virtual void onWakeUp(uint32 threadID) {wakeUpNum++;}
  virtual void onLock(uint32 threadID)   {lockNum++;}
  virtual void onUnlock(uint32 threadID) {unlockNum++;}
  virtual void onRunStart(const char *taskName, uint32 threadID) {runStartNum++;}
  virtual void onRunEnd(const char *taskName, uint32 threadID)   {runEndNum++;}
  virtual void onEnd(const char *taskName, uint32 threadID)      {endNum++;}
  Atomic sleepNum;
  Atomic wakeUpNum;
  Atomic lockNum;
  Atomic unlockNum;
  Atomic runStartNum;
  Atomic runEndNum;
  Atomic endNum;
};

START_UTEST(TestProfiler)
{
  UTestProfiler *profiler = GBE_NEW(UTestProfiler);
  TaskingSystemSetProfiler(profiler);
  TestFibo();
  TestTaskSet();
  TestMultiDependency();
  TestLockUnlock();
#define OUTPUT_FIELD(FIELD) \
  std::cout << #FIELD ": " << profiler->FIELD << std::endl
  OUTPUT_FIELD(sleepNum);
  OUTPUT_FIELD(wakeUpNum);
  OUTPUT_FIELD(lockNum);
  OUTPUT_FIELD(unlockNum);
  OUTPUT_FIELD(runStartNum);
  OUTPUT_FIELD(runEndNum);
  OUTPUT_FIELD(endNum);    
#undef OUTPUT_FIELD
  TaskingSystemSetProfiler(NULL);
  GBE_DELETE(profiler);
}
END_UTEST(TestProfiler)
#endif /* GBE_TASK_PROFILER */

/*! Run all tasking tests */
void utest_tasking(void)
{
  TestDummy();
  TestTree<TaskNodeOpt>();
  TestTree<TaskNode>();
  TestTree<TaskCascadeNodeOpt>();
  TestTree<TaskCascadeNode>();
  TestTaskSet();
  TestAllocator();
  TestFullQueue();
  TestAffinity();
  TestFibo();
  TestMultiDependency();
  TestMultiDependencyTwoStage();
  TestMultiDependencyRandomStart();
  TestLockUnlock();
  TestProfiler();
}

UTEST_REGISTER(utest_tasking);
