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
// ui/tools/textconv.hh -- header for the text converter tool

#ifndef HEXBED_UI_TOOLS_TEXTCONV_HH
#define HEXBED_UI_TOOLS_TEXTCONV_HH

#include <wx/choice.h>
#include <wx/dialog.h>
#include <wx/textctrl.h>

#include "common/types.hh"
#include "ui/context.hh"
#include "ui/hexbed-fwd.hh"
#include "ui/saeditor.hh"

namespace hexbed {

namespace ui {

enum struct NewlineMode {
    Raw,
    LF,
    CRLF,
    CR,
};

class TextConverterTool : public wxDialog {
  public:
    TextConverterTool(HexBedMainFrame* parent,
                      std::shared_ptr<HexBedContextMain> context,
                      std::shared_ptr<HexBedDocument> document);
    void UpdateConfig();

  private:
    void OnTextToBytes(wxCommandEvent&);
    void OnBytesToText(wxCommandEvent&);
    void OnCopySelection(wxCommandEvent&);
    void OnPasteSelection(wxCommandEvent&);

    HexBedMainFrame* parent_;
    std::shared_ptr<HexBedContextMain> context_;
    std::shared_ptr<HexBedDocument> document_;
    HexBedEditorRegistration registration_;

    HexBedStandaloneEditor* editor_;
    wxTextCtrl* textInput_;
    wxChoice* encodingList_;

    string encoding_;
    NewlineMode newlines_{NewlineMode::Raw};
};

};  // namespace ui

};  // namespace hexbed

#endif /* HEXBED_UI_TOOLS_TEXTCONV_HH */
