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
// ui/dialog/radixpicker.hh -- header for the dialog radix picker

#ifndef HEXBED_UI_DIALOG_RADIXPICKER_HH
#define HEXBED_UI_DIALOG_RADIXPICKER_HH

#include <wx/panel.h>
#include <wx/radiobut.h>
#include <wx/stattext.h>

namespace hexbed {

namespace ui {

wxDECLARE_EVENT(hEVT_RADIXCHANGE, wxCommandEvent);

class RadixPicker : public wxPanel {
  public:
    RadixPicker(wxWindow* parent);
    unsigned GetRadix() const noexcept;

  protected:
    void OnRadioButton(wxCommandEvent& event);

  private:
    void UpdateValue();

    unsigned value_;
    wxRadioButton* oct_;
    wxRadioButton* dec_;
    wxRadioButton* hex_;
};

};  // namespace ui

};  // namespace hexbed

#endif /* HEXBED_UI_DIALOG_RADIXPICKER_HH */
