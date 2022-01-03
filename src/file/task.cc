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
// file/task.cc -- impl for the HexBed progress tracker

#include "file/task.hh"

#include "common/logger.hh"
#include "file/document.hh"

#if HEXBED_MULTITHREADED
#include <future>
#include <thread>
#endif

namespace hexbed {

HexBedTask::HexBedTask(HexBedContext* context, bufsize size, bool canCancel)
    : cancelled_(false),
      progress_(0),
      complete_(size),
      handler_(context->getTaskHandler()),
      canCancel_(canCancel) {}

void HexBedTask::run(std::function<void(HexBedTask&)> fn) {
    HexBedTask& self = *this;
#if HEXBED_MULTITHREADED
    if (handler_) handler_->onTaskBegin(this, complete_);
    cancelled_.store(false);
    progress_.store(0);
    std::promise<void> p;
    std::future<void> f = p.get_future();
    std::thread t([&p, &self, fn] {
        try {
            fn(self);
            p.set_value();
        } catch (...) {
            try {
                p.set_exception(std::current_exception());
            } catch (...) {
            }
        }
        if (self.handler_) self.handler_->onTaskEnd(&self);
    });
    if (handler_) handler_->onTaskWait(this);
    t.join();
    f.get();
#else
    cancelled_ = false;
    progress = 0;
    fn(self);
#endif
}

bufsize HexBedTask::progress() const noexcept {
#if HEXBED_MULTITHREADED
    return progress_.load();
#else
    return progress_;
#endif
}

bufsize HexBedTask::complete() const noexcept { return complete_; }

void HexBedTask::cancel() noexcept {
#if HEXBED_MULTITHREADED
    cancelled_.store(true);
#else
    cancelled_ = true;
#endif
}

bool HexBedTask::isCancelled() const noexcept {
#if HEXBED_MULTITHREADED
    return cancelled_.load();
#else
    return cancelled_;
#endif
}

bool HexBedTask::canCancel() const noexcept { return canCancel_; }

template <typename T>
static T atomic_fetch_max(std::atomic<T>& atomic, const T& value) noexcept {
    T prev = atomic.load();
    while (prev < value && !atomic.compare_exchange_weak(prev, value))
        ;
    return prev;
}

void HexBedTask::progress(bufsize prog) noexcept {
#if HEXBED_MULTITHREADED
    atomic_fetch_max(progress_, prog);
    if (handler_) handler_->onTaskProgress(this, prog);
#else
    progress_ = prog;
#endif
}

};  // namespace hexbed
