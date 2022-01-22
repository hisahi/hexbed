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
// ui/dialog/jump.hh -- header for the jump (quick go to) dialog

#ifndef HEXBED_UI_DIALOG_JUMP_HH
#define HEXBED_UI_DIALOG_JUMP_HH

#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#include "common/types.hh"
#include "ui/dialogs/radixpicker.hh"

namespace hexbed {

namespace ui {

class OffsetJumpDialog : public wxDialog {
  public:
    OffsetJumpDialog(wxWindow* parent, bufsize cur, bufsize end);
    bufsize GetOffset() const noexcept;

  protected:
    void OnOK(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);

    void OnTextInput(wxCommandEvent& event);

  private:
    void EndDialog(int result);

    bool CheckInput();

    wxTextCtrl* text_;
    wxButton* okButton_;

    bufsize cur_;
    bufsize end_;
};

};  // namespace ui

};  // namespace hexbed

#endif /* HEXBED_UI_DIALOG_JUMP_HH */
