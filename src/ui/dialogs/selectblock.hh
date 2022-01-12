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
// ui/dialog/selectblock.hh -- header for the Select block dialog

#ifndef HEXBED_UI_DIALOG_SELECTBLOCK_HH
#define HEXBED_UI_DIALOG_SELECTBLOCK_HH

#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/radiobut.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#include "common/types.hh"
#include "ui/dialogs/radixpicker.hh"

namespace hexbed {

namespace ui {

class SelectBlockDialog : public wxDialog {
  public:
    SelectBlockDialog(wxWindow* parent, bufsize cur, bufsize len, bufsize end);
    bufsize GetOffset() const noexcept;
    bufsize GetLength() const noexcept;
    void UpdateMetrics(bufsize cur, bufsize len, bufsize end);

  protected:
    void OnOK(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);

    void OnEnterFirst(wxCommandEvent& event);
    void OnTextInput(wxCommandEvent& event);
    void OnOffsetChange(wxCommandEvent& event);
    void OnRadixChange(wxCommandEvent& event);

  private:
    void EndDialog(int result);

    bool CheckInput();
    void ConvertBase(unsigned base);
    void ConvertNewSub(bool sub);

    wxTextCtrl* text_;
    wxTextCtrl* text2_;
    RadixPicker* radix_;
    wxButton* okButton_;
    wxRadioButton* off0_;
    wxRadioButton* off1_;

    bufsize cur_;
    bufsize len_;
    bufsize end_;
    unsigned base_{16};
    bool sub_{false};
};

};  // namespace ui

};  // namespace hexbed

#endif /* HEXBED_UI_DIALOG_SELECTBLOCK_HH */
