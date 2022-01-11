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
// ui/tools/inspector.hh -- header for the data inspector tool

#ifndef HEXBED_UI_TOOLS_INSPECTOR_HH
#define HEXBED_UI_TOOLS_INSPECTOR_HH

#include <wx/dialog.h>
#include <wx/listctrl.h>

#include <unordered_map>

#include "common/types.hh"
#include "ui/context.hh"
#include "ui/plugins/inspector.hh"

namespace hexbed {

namespace ui {

class DataInspector : public wxDialog, public HexBedViewer {
  public:
    DataInspector(HexBedMainFrame* parent,
                  std::shared_ptr<HexBedContextMain> context);
    void onUpdateCursor(HexBedPeekRegion peek) override;
    void UpdatePlugins();

  private:
    void UpdateValues(const_bytespan data);
    void OnUpdateSetting(wxCommandEvent& event);
    void OnDoubleClick(wxListEvent& event);

    HexBedDocument* document_;
    std::shared_ptr<HexBedContextMain> context_;
    bufsize offset_;
    HexBedViewerRegistration reg_;
    wxListView* listView_;
    std::unordered_map<hexbed::plugins::DataInspectorPlugin*, long> plugins_;
    hexbed::plugins::DataInspectorSettings settings_;
    std::vector<char> strBuf_;
    std::size_t alloc_;
};

};  // namespace ui

};  // namespace hexbed

#endif /* HEXBED_UI_TOOLS_BITEDIT_HH */
