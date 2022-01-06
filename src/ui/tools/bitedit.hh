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
// ui/tools/bitedit.hh -- header for the bit editor tool

#ifndef HEXBED_UI_TOOLS_BITEDIT_HH
#define HEXBED_UI_TOOLS_BITEDIT_HH

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/dialog.h>
#include <wx/stattext.h>

#include "common/types.hh"
#include "ui/context.hh"

namespace hexbed {

namespace ui {

class BitEditorTool : public wxDialog, public HexBedViewer {
  public:
    BitEditorTool(HexBedMainFrame* parent,
                  std::shared_ptr<HexBedContextMain> context);
    void onUpdateCursor(HexBedPeekRegion peek) override;

  private:
    void OnBitFlip(int bit, bool newState);
    HexBedDocument* document_;
    bufsize offset_;
    wxCheckBox* bitChecks_[8];
    wxStaticText* bitLabels_[8];
    wxStaticText* byteLabel_;
    byte buf_;
    HexBedViewerRegistration reg_;
    bool wasAllowed_;
};

};  // namespace ui

};  // namespace hexbed

#endif /* HEXBED_UI_TOOLS_BITEDIT_HH */
