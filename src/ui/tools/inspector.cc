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
// ui/tools/inspector.cc -- impl for the data inspector tool

#include "ui/tools/inspector.hh"

#include <wx/msgdlg.h>
#include <wx/settings.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/valgen.h>

#include "app/config.hh"
#include "common/hexconv.hh"
#include "ui/hexbed.hh"
#include "ui/plugins/inspector.hh"

namespace hexbed {

namespace ui {

class DataEntryDialog : public wxDialog {
  public:
    DataEntryDialog(wxWindow* parent,
                    hexbed::plugins::DataInspectorPlugin* plugin,
                    const wxString& type, const wxString& value,
                    hexbed::plugins::DataInspectorSettings* settings);
    const_bytespan GetValue() const noexcept;

  protected:
    void OnOK(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);

  private:
    void EndDialog(int result);
    bool CheckInput();
    void OnChangedInput(wxCommandEvent& event);

    hexbed::plugins::DataInspectorPlugin* plugin_;
    std::size_t dataSize_;
    std::unique_ptr<byte[]> data_;
    hexbed::plugins::DataInspectorSettings* settings_;
    std::size_t dataLen_{0};
    wxButton* okButton_;
    wxTextCtrl* textCtrl_;
};

DataEntryDialog::DataEntryDialog(
    wxWindow* parent, hexbed::plugins::DataInspectorPlugin* plugin,
    const wxString& type, const wxString& value,
    hexbed::plugins::DataInspectorSettings* settings)
    : wxDialog(parent, wxID_ANY, _("Enter data"), wxDefaultPosition,
               wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
      plugin_(plugin),
      dataSize_(plugin->getRequestedDataBufferSize()),
      data_(std::make_unique<byte[]>(dataSize_)),
      settings_(settings) {
    SetReturnCode(wxID_CANCEL);

    wxBoxSizer* top = new wxBoxSizer(wxVERTICAL);
    wxStdDialogButtonSizer* buttons = new wxStdDialogButtonSizer();

    okButton_ = new wxButton(this, wxID_OK);
    okButton_->Bind(wxEVT_BUTTON, &DataEntryDialog::OnOK, this);

    wxButton* cancelButton = new wxButton(this, wxID_CANCEL);
    cancelButton->Bind(wxEVT_BUTTON, &DataEntryDialog::OnCancel, this);

    buttons->SetAffirmativeButton(okButton_);
    buttons->SetNegativeButton(cancelButton);
    buttons->Realize();

    textCtrl_ = new wxTextCtrl(this, wxID_ANY, value, wxDefaultPosition,
                               wxDefaultSize, wxTE_PROCESS_ENTER);
    textCtrl_->Bind(wxEVT_TEXT, &DataEntryDialog::OnChangedInput, this);
    textCtrl_->Bind(wxEVT_TEXT_ENTER, &DataEntryDialog::OnOK, this);
    textCtrl_->SelectAll();

    top->Add(new wxStaticText(this, wxID_ANY, type, wxDefaultPosition,
                              wxDefaultSize, wxST_ELLIPSIZE_END),
             wxSizerFlags().Expand());
    top->Add(textCtrl_, wxSizerFlags().Expand());
    top->Add(buttons, wxSizerFlags().Expand());
    okButton_->Enable(CheckInput());

    SetSizer(top);
    Fit();
    SetSizeHints(GetSize().GetWidth(), GetSize().GetHeight(), -1,
                 GetSize().GetHeight());
    Layout();
}

const_bytespan DataEntryDialog::GetValue() const noexcept {
    return const_bytespan(data_.get(), dataLen_);
}

void DataEntryDialog::EndDialog(int r) {
    SetReturnCode(r);
    if (IsModal()) EndModal(r);
}

bool DataEntryDialog::CheckInput() { return !textCtrl_->IsEmpty(); }

void DataEntryDialog::OnOK(wxCommandEvent& event) {
    if (!CheckInput()) return;
    std::size_t l = dataSize_;
    if (!plugin_->convertToBytes(l, data_.get(),
                                 textCtrl_->GetValue().ToStdString().c_str(),
                                 *settings_)) {
        wxMessageBox(_("The given input is invalid for this data type."),
                     "HexBed", wxOK | wxICON_ERROR);
        return;
    }
    dataLen_ = std::min(dataSize_, l);
    EndDialog(wxID_OK);
}

void DataEntryDialog::OnCancel(wxCommandEvent& event) {
    EndDialog(wxID_CANCEL);
}

void DataEntryDialog::OnChangedInput(wxCommandEvent& event) {
    okButton_->Enable(CheckInput());
}

DataInspector::DataInspector(HexBedMainFrame* parent,
                             std::shared_ptr<HexBedContextMain> context)
    : wxDialog(parent, wxID_ANY, _("Data inspector"), wxDefaultPosition,
               wxSize(400, 300), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
      HexBedViewer(MAX_LOOKAHEAD),
      context_(context),
      alloc_(64) {
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    listView_ = new wxListView(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                               wxLC_REPORT);
    sizer->Add(listView_, wxSizerFlags().Expand().Proportion(1));
    wxCheckBox* checkBox = new wxCheckBox(
        this, wxID_ANY, _("Little endian"), wxDefaultPosition, wxDefaultSize, 0,
        wxGenericValidator(&settings_.littleEndian));
    sizer->Add(checkBox, wxSizerFlags().Expand());
    SetSizer(sizer);
    Layout();
    reg_ = HexBedViewerRegistration(context, this);
    checkBox->TransferDataToWindow();
    checkBox->Bind(wxEVT_CHECKBOX, &DataInspector::OnUpdateSetting, this);
    listView_->Bind(wxEVT_LIST_ITEM_ACTIVATED, &DataInspector::OnDoubleClick,
                    this);
    strBuf_.reserve(alloc_);
    UpdatePlugins();
}

void DataInspector::OnUpdateSetting(wxCommandEvent& event) {
    TransferDataFromWindow();
    context_->pokeViewer(this);
}

void DataInspector::UpdatePlugins() {
    plugins_.clear();
    listView_->ClearAll();
    listView_->AppendColumn(_("Type"), wxLIST_FORMAT_LEFT, 200);
    listView_->AppendColumn(_("Value"), wxLIST_FORMAT_LEFT, -1);
    for (std::size_t i = 0, e = hexbed::plugins::dataInspectorPluginCount();
         i < e; ++i) {
        hexbed::plugins::DataInspectorPlugin& plugin =
            hexbed::plugins::dataInspectorPluginByIndex(i);
        long index = listView_->InsertItem(
            i, plugin.isLocalizable() ? wxGetTranslation(plugin.getTitle())
                                      : plugin.getTitle());
        plugins_.emplace(&plugin, index);
        listView_->SetItemPtrData(index, reinterpret_cast<wxUIntPtr>(&plugin));
    }
    context_->pokeViewer(this);
}

void DataInspector::OnDoubleClick(wxListEvent& event) {
    if (!document_ || document_->readOnly()) return;
    long index = event.GetIndex();
    auto plugin = static_cast<hexbed::plugins::DataInspectorPlugin*>(
        reinterpret_cast<void*>(event.GetData()));
    if (plugin && !plugin->isReadOnly()) {
        if (offset_ + plugin->getRequestedDataBufferSize() > document_->size())
            return;
        DataEntryDialog dialog(this, plugin, listView_->GetItemText(index, 0),
                               listView_->GetItemText(index, 1), &settings_);
        if (dialog.ShowModal() == wxID_OK)
            document_->impose(offset_, dialog.GetValue());
    }
}

static void EnableListItem(wxListCtrl* listView, long i, bool flag) {
    listView->SetItemTextColour(
        i, flag ? listView->GetTextColour()
                : wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
}

void DataInspector::UpdateValues(const_bytespan data) {
    for (const auto& pair : plugins_) {
        hexbed::plugins::DataInspectorPlugin* plugin = pair.first;
        long index = pair.second;
        if (document_) {
            std::size_t strbufsize = plugin->getRequestedStringBufferSize();
            std::size_t ds =
                std::min(plugin->getRequestedDataBufferSize(), data.size());
            if (ds < plugin->getRequestedDataBufferSize()) {
                listView_->SetItem(index, 1, "<<not enough data>>");
                EnableListItem(listView_, index, false);
                continue;
            }
            if (alloc_ < strbufsize) {
                strBuf_.reserve(alloc_);
                alloc_ = strbufsize;
            }
            bool ok = plugin->convertFromBytes(alloc_, strBuf_.data(),
                                               const_bytespan{data.data(), ds},
                                               settings_);
            if (ok)
                listView_->SetItem(index, 1, wxString(strBuf_.data()));
            else
                listView_->SetItem(index, 1, "<<invalid>>");
            EnableListItem(listView_, index, ok);
        } else {
            listView_->SetItem(index, 1, wxEmptyString);
        }
    }
}

void DataInspector::onUpdateCursor(HexBedPeekRegion peek) {
    document_ = peek.document;
    offset_ = peek.offset;
    UpdateValues(peek.data);
}

};  // namespace ui

};  // namespace hexbed
