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
// ui/context.cc -- implementation for the main context

#include "ui/context.hh"

#include <wx/app.h>
#include <wx/event.h>
#include <wx/progdlg.h>

#include "ui/editor.hh"
#include "ui/hexbed.hh"

namespace hexbed {

wxDEFINE_EVENT(TASK_SUBTHREAD_EVENT, wxThreadEvent);

class HexBedTaskHandlerMain : public HexBedTaskHandler, wxEvtHandler {
  public:
    // called on main thread
    void onTaskBegin(HexBedTask* task, bufsize complete) {
        if (task_) return;
        bufsize bw = std::bit_width(complete) + 1;
        constexpr bufsize mbw =
            std::bit_width<bufsize>(std::numeric_limits<int>::max());
        shift_ = bw > mbw ? bw - mbw : 0;
        end_ = (complete >> shift_) + 1;
        task_ = task;
        timer_.StartOnce(complete ? 500 : 250);
    }

    void OnTimeOut(wxTimerEvent& event) {
        try {
            int flags = wxPD_APP_MODAL | wxPD_AUTO_HIDE | wxPD_ELAPSED_TIME |
                        wxPD_ESTIMATED_TIME | wxPD_REMAINING_TIME;
            if (task_->canCancel()) flags |= wxPD_CAN_ABORT;
            dialog_ = std::make_unique<wxProgressDialog>(
                "HexBed", _("Action in progress"), end_ - 1, main_, flags);
            if (prog_) dialog_->Update(prog_);
        } catch (...) {
        }
    }

    void OnThreadMsg(wxThreadEvent& event) {
        int x = event.GetInt();
        if (x >= end_) {
            prog_ = x;
            if (!dialog_) timer_.Stop();
            dialog_ = nullptr;
            task_ = nullptr;
        } else if (dialog_ && !dialog_->Update(x) && x < end_)
            task_->cancel();
    }

    // called on main thread
    void onTaskWait(HexBedTask* task) {
        if (task_ == task) {
            while (task_) {
                wxTheApp->Dispatch();
            }
        }
    }

    // called from subthread
    void onTaskProgress(HexBedTask* task, bufsize progress) {
        if (task_ == task) {
            wxThreadEvent* e = new wxThreadEvent(TASK_SUBTHREAD_EVENT);
            e->SetInt(static_cast<int>(progress >> shift_));
            wxTheApp->QueueEvent(e);
        }
    }

    // called from subthread
    void onTaskEnd(HexBedTask* task) {
        if (task_ == task) {
            wxThreadEvent* e = new wxThreadEvent(TASK_SUBTHREAD_EVENT);
            e->SetInt(end_);
            wxTheApp->QueueEvent(e);
        }
    }

    HexBedTaskHandlerMain(hexbed::ui::HexBedMainFrame* main)
        : main_(main), timer_(this) {
        Bind(wxEVT_TIMER, &HexBedTaskHandlerMain::OnTimeOut, this);
        wxTheApp->Bind(TASK_SUBTHREAD_EVENT,
                       &HexBedTaskHandlerMain::OnThreadMsg, this);
    }

  private:
    hexbed::ui::HexBedMainFrame* main_;
    HexBedTask* task_{nullptr};
    bufsize shift_{0};
    int prog_{0};
    int end_{0};
    std::unique_ptr<wxProgressDialog> dialog_;
    wxTimer timer_;
};

HexBedContextMain::HexBedContextMain(hexbed::ui::HexBedMainFrame* main)
    : main_(main), task_(std::make_unique<HexBedTaskHandlerMain>(main)) {}

HexBedTaskHandler* HexBedContextMain::getTaskHandler() { return task_.get(); }

void HexBedContextMain::announceBytesChanged(HexBedDocument* doc,
                                             bufsize start) {
    auto it = open_.find(doc);
    if (it != open_.end())
        for (hexbed::ui::HexEditorParent* editor : it->second)
            editor->HintBytesChanged(start);
}

void HexBedContextMain::announceBytesChanged(HexBedDocument* doc, bufsize start,
                                             bufsize length) {
    if (!length) return;
    auto it = open_.find(doc);
    if (it != open_.end()) {
        if (length == 1) {
            for (hexbed::ui::HexEditorParent* editor : it->second)
                editor->HintByteChanged(start);
        } else {
            for (hexbed::ui::HexEditorParent* editor : it->second)
                editor->HintBytesChanged(start, start + length - 1);
        }
    }
}

void HexBedContextMain::announceUndoChanged(HexBedDocument* doc) {
    auto it = open_.find(doc);
    if (it != open_.end()) main_->UpdateMenuEnabledUndo(*it->second[0]);
}

void HexBedContextMain::addWindow(hexbed::ui::HexEditorParent* editor) {
    HexBedDocument* document = &editor->document();
    auto it = open_.find(document);
    if (it != open_.end())
        it->second.push_back(editor);
    else
        open_.emplace(document,
                      std::vector<hexbed::ui::HexEditorParent*>{editor});
}

void HexBedContextMain::removeWindow(
    hexbed::ui::HexEditorParent* editor) noexcept {
    HexBedDocument* document = &editor->document();
    auto it = open_.find(document);
    if (it != open_.end()) {
        std::vector<hexbed::ui::HexEditorParent*>& editors = it->second;
        editors.erase(std::remove(editors.begin(), editors.end(), editor),
                      editors.end());
        if (editors.empty()) open_.erase(document);
    }
}

void HexBedContextMain::updateWindows() {
    for (const auto& pair : open_) {
        for (hexbed::ui::HexEditorParent* editor : pair.second) {
            editor->ReloadConfig();
        }
    }
}

byte* HexBedContextMain::getSearchBuffer(bufsize n) {
    searchBuffer_.resize(n);
    searchBuffer_.shrink_to_fit();
    return searchBuffer_.data();
}

const_bytespan HexBedContextMain::getSearchString() const noexcept {
    return const_bytespan{searchBuffer_.begin(), searchBuffer_.end()};
}

byte* HexBedContextMain::getReplaceBuffer(bufsize n) {
    replaceBuffer_.resize(n);
    replaceBuffer_.shrink_to_fit();
    return replaceBuffer_.data();
}

const_bytespan HexBedContextMain::getReplaceString() const noexcept {
    return const_bytespan{replaceBuffer_.begin(), replaceBuffer_.end()};
}

};  // namespace hexbed
