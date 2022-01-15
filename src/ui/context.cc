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
#include <wx/msgdlg.h>
#include <wx/progdlg.h>

#include "app/config.hh"
#include "common/logger.hh"
#include "ui/editor.hh"
#include "ui/hexbed.hh"

namespace hexbed {

wxDEFINE_EVENT(TASK_SUBTHREAD_EVENT, wxThreadEvent);

HexBedViewer::HexBedViewer(bufsize lookahead) : lookahead_(lookahead) {}

HexBedEditorRegistration::HexBedEditorRegistration()
    : ctx_(nullptr), ptr_(nullptr) {}

HexBedEditorRegistration::HexBedEditorRegistration(
    std::shared_ptr<HexBedContextMain> context,
    hexbed::ui::HexEditorParent* editor)
    : ctx_(context), ptr_(editor) {
    ctx_->addWindow(ptr_);
}

HexBedEditorRegistration::~HexBedEditorRegistration() {
    if (ctx_ && ptr_) ctx_->removeWindow(ptr_);
}

HexBedViewerRegistration::HexBedViewerRegistration()
    : ctx_(nullptr), ptr_(nullptr) {}

HexBedViewerRegistration::HexBedViewerRegistration(
    std::shared_ptr<HexBedContextMain> context, HexBedViewer* viewer)
    : ctx_(context), ptr_(viewer) {
    ctx_->addViewer(ptr_);
}

HexBedViewerRegistration::~HexBedViewerRegistration() {
    if (ctx_ && ptr_) ctx_->removeViewer(ptr_);
}

void HexBedViewer::onUpdateCursor(HexBedPeekRegion peek) {
    LOG_DEBUG("pure virtual onUpdateCursor");
}

class HexBedTaskHandlerMain : public HexBedTaskHandler, wxEvtHandler {
  public:
    // called on main thread
    void onTaskBegin(HexBedTask* task, bufsize complete) {
        if (task_) return;
        bufsize bw = std::bit_width(complete) + 1;
        constexpr bufsize mbw =
            std::bit_width<bufsize>(std::numeric_limits<short>::max());
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
            if (prog_ && prog_ < end_) {
                if (!dialog_->Update(prog_))
                    task_->cancel();
            }
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
        } else if (dialog_ && x < end_ && !dialog_->Update(x))
            task_->cancel();
    }

    // called on main thread
    void onTaskWait(HexBedTask* task) {
        if (task_ == task) {
            while (task_) {
                wxTheApp->Dispatch();
            }
            timer_.Stop();
            dialog_ = nullptr;
            prog_ = 0;
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

bool HexBedContextMain::shouldBackup() { return config().backupFiles; }

FailureResponse HexBedContextMain::ifBackupFails(const string& message) {
    wxMessageDialog dial(
        main_, wxString::Format(_("Backup failed:\n%s"), message), "HexBed",
        wxYES_NO | wxCANCEL | wxCANCEL_DEFAULT | wxICON_EXCLAMATION);
    if (dial.SetYesNoCancelLabels(_("&Retry"), _("&Ignore"), _("&Abort")))
        dial.SetMessage(dial.GetMessage() + "\n\n" +
                        _("'Retry' to try again, 'Ignore' to save anyway or "
                          "'Abort' to cancel."));
    else
        dial.SetMessage(dial.GetMessage() + "\n\n" +
                        _("Retry? 'Yes' to try again, 'No' to save anyway or "
                          "'Cancel' to cancel."));
    switch (dial.ShowModal()) {
    case wxID_YES:
        return FailureResponse::Retry;
    case wxID_NO:
        return FailureResponse::Ignore;
    default:
    case wxID_CANCEL:
        return FailureResponse::Abort;
    }
}

static HexBedPeekRegion makePeekBuffer(HexBedDocument* doc, bufsize start,
                                       bytespan buffer) {
    return HexBedPeekRegion{
        doc, start,
        doc ? const_bytespan{buffer.begin(), doc->read(start, buffer)}
            : const_bytespan{}};
}

void HexBedContextMain::announceBytesChanged(HexBedDocument* doc,
                                             bufsize start) {
    auto it = open_.find(doc);
    if (it != open_.end())
        for (hexbed::ui::HexEditorParent* editor : it->second)
            editor->HintBytesChanged(start);
    if (doc == lastdoc_ && !viewers_.empty()) {
        byte tmp[MAX_LOOKAHEAD];
        HexBedPeekRegion peek =
            makePeekBuffer(doc, lastoff_, bytespan{tmp, tmp + sizeof(tmp)});
        for (HexBedViewer* viewer : viewers_) {
            bufsize la = viewer->lookahead();
            if (start < lastoff_ + la) viewer->onUpdateCursor(peek);
        }
    }
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
    if (doc == lastdoc_ && !viewers_.empty()) {
        byte tmp[MAX_LOOKAHEAD];
        HexBedPeekRegion peek =
            makePeekBuffer(doc, lastoff_, bytespan{tmp, tmp + sizeof(tmp)});
        for (HexBedViewer* viewer : viewers_) {
            bufsize la = viewer->lookahead();
            if (!(start + length < lastoff_ || start >= lastoff_ + la))
                viewer->onUpdateCursor(peek);
        }
    }
}

void HexBedContextMain::announceUndoChanged(HexBedDocument* doc) {
    auto it = open_.find(doc);
    if (it != open_.end()) main_->OnUndoRedo(*it->second[0]);
}

void HexBedContextMain::announceCursorUpdate(HexBedPeekRegion peek) {
    lastdoc_ = peek.document;
    lastoff_ = peek.offset;
    for (HexBedViewer* viewer : viewers_) viewer->onUpdateCursor(peek);
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
        std::erase(editors, editor);
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

void HexBedContextMain::addViewer(HexBedViewer* viewer) {
    viewers_.push_back(viewer);
    pokeViewer(viewer);
}

void HexBedContextMain::removeViewer(HexBedViewer* viewer) noexcept {
    std::erase(viewers_, viewer);
}

void HexBedContextMain::pokeViewer(HexBedViewer* viewer) {
    byte tmp[MAX_LOOKAHEAD];
    if (lastdoc_)
        viewer->onUpdateCursor(makePeekBuffer(
            lastdoc_, lastoff_, bytespan{tmp, tmp + sizeof(tmp)}));
}

hexbed::ui::HexEditorParent* HexBedContextMain::activeWindow() noexcept {
    return main_->GetCurrentEditor();
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
