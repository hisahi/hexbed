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
// ui/dialog/insert.hh -- header for the Insert/Replace dialog

#ifndef HEXBED_UI_DIALOG_INSERT_HH
#define HEXBED_UI_DIALOG_INSERT_HH

#include <wx/dialog.h>
#include <wx/spinctrl.h>

#include "common/types.hh"
#include "ui/editor-fwd.hh"
#include "ui/hexbed-fwd.hh"
#include "ui/saeditor.hh"

namespace hexbed {

namespace ui {

class InsertBlockDialog : public wxDialog {
  public:
    InsertBlockDialog(HexBedMainFrame* parent,
                      std::shared_ptr<HexBedContextMain> context,
                      std::shared_ptr<HexBedDocument> document, bufsize seln);
    bufsize GetByteCount() const noexcept;

  protected:
    void OnOK(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);

  private:
    void EndDialog(int result);
    bool CheckInput();
    void OnChangedInput(wxCommandEvent& event);

    HexBedMainFrame* parent_;

    std::shared_ptr<HexBedContextMain> context_;
    std::shared_ptr<HexBedDocument> document_;
    HexBedEditorRegistration registration_;

    HexBedStandaloneEditor* editor_;
    wxSpinCtrl* spinner_;
    wxButton* okButton_;
};

};  // namespace ui

};  // namespace hexbed

#endif /* HEXBED_UI_DIALOG_INSERT_HH */
