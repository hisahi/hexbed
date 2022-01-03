/****************************************************************************/
/*                                                                          */
/* HexBed -- Hex editor                                                     */
/* Copyright (c) 2021-2022 Sampo Hippeläinen (hisahi)                       */
/*                                                                          */
/* This program is free software: you can redistribute it and/or modify     */
/* it under the terms of the GNU General Public License as published by     */
/* the Free Software Foundation, either version 3 of the License, or        */
/* (at your option) any later version.                                      */
/*                                                                          */
/* This program is distributed in the hope that it will be useful,          */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of           */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            */
/* GNU General Public License for more details.                             */
/*                                                                          */
/* You should have received a copy of the GNU General Public License        */
/* along with this program.  If not, see <https://www.gnu.org/licenses/>.   */
/*                                                                          */
/****************************************************************************/
// file/task.hh -- header for the HexBed progress tracker

#ifndef HEXBED_FILE_TASK_HH
#define HEXBED_FILE_TASK_HH

#define HEXBED_MULTITHREADED 1

#include <functional>
#include <memory>
#if HEXBED_MULTITHREADED
#include <atomic>
#endif

#include "common/types.hh"
#include "file/context.hh"

namespace hexbed {

class HexBedTaskHandler;

class HexBedTask {
  public:
    // use size=0 for indeterminate length
    HexBedTask(HexBedContext* context, bufsize size, bool canCancel);
    void run(std::function<void(HexBedTask&)> fn);

    bufsize progress() const noexcept;
    void progress(bufsize prog) noexcept;
    bufsize complete() const noexcept;

    void cancel() noexcept;
    bool isCancelled() const noexcept;
    bool canCancel() const noexcept;

  private:
#if HEXBED_MULTITHREADED
    std::atomic<bool> cancelled_;
    std::atomic<bufsize> progress_;
#else
    bool cancelled_;
    bufsize progress_;
#endif
    bufsize complete_;
    HexBedTaskHandler* handler_;
    bool canCancel_;
};

class HexBedTaskHandler {
  public:
    // called on main thread
    virtual void onTaskBegin(HexBedTask* task, bufsize complete) = 0;
    // called from subthread
    virtual void onTaskProgress(HexBedTask* task, bufsize progress) = 0;
    // called on main thread
    virtual void onTaskWait(HexBedTask* task) = 0;
    // called from subthread
    virtual void onTaskEnd(HexBedTask* task) = 0;
};

};  // namespace hexbed

#endif /* HEXBED_FILE_TASK_HH */
