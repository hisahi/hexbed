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
// plugins/dialog.hh -- header for the plugin configure dialog

#ifndef HEXBED_PLUGINS_DIALOG_HH
#define HEXBED_PLUGINS_DIALOG_HH

#include <wx/dialog.h>

namespace hexbed {

namespace plugins {

template <bool preCommit>
class PluginConfigureDialog : public wxDialog {
  public:
    using wxDialog::wxDialog;

  protected:
    void OnOK(wxCommandEvent& event) {
        if constexpr (preCommit) TransferDataFromWindow();
        if (CheckValid()) EndModal(wxID_OK);
    }
    virtual bool CheckValid() noexcept { return true; }
};

};  // namespace plugins

};  // namespace hexbed

#endif /* HEXBED_PLUGINS_DIALOG_HH */
