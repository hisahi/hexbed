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
// ui/textinput.hh -- header for the text input class

#ifndef HEXBED_UI_TEXTINPUT_HH
#define HEXBED_UI_TEXTINPUT_HH

#include <wx/choice.h>
#include <wx/panel.h>
#include <wx/string.h>
#include <wx/textctrl.h>
#include <wx/window.h>

#include "common/types.hh"
#include "file/document-fwd.hh"

namespace hexbed {
namespace ui {

class HexBedTextInput : public wxPanel {
  public:
    HexBedTextInput(wxWindow* parent, wxString* string, std::string* encoding,
                    bool* caseInsensitive);

    void UpdateConfig();
    bool Commit(HexBedDocument* document);
    bool NonEmpty() const noexcept;

  private:
    std::string* pEncoding_;
    wxString* pText_;
    wxTextCtrl* textCtrl_;
    wxChoice* choice_;

    void ForwardEvent(wxCommandEvent& event);
};

};  // namespace ui
};  // namespace hexbed

#endif /* HEXBED_UI_TEXTINPUT_HH */
