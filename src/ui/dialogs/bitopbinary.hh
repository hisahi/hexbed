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
// ui/dialog/bitopbinary.hh -- header for the block binary bit op dialog

#ifndef HEXBED_UI_DIALOG_BITOPBINARY_HH
#define HEXBED_UI_DIALOG_BITOPBINARY_HH

#include <wx/choice.h>
#include <wx/dialog.h>

#include "app/bitop.hh"
#include "common/types.hh"
#include "ui/editor-fwd.hh"
#include "ui/hexbed-fwd.hh"
#include "ui/saeditor.hh"

namespace hexbed {

namespace ui {

class BitwiseBinaryOpDialog : public wxDialog {
  public:
    BitwiseBinaryOpDialog(HexBedMainFrame* parent, HexBedContextMain* context,
                          std::shared_ptr<HexBedDocument> document);
    BitwiseBinaryOp GetOperation() const noexcept;

  protected:
    void OnOK(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);

  private:
    void EndDialog(int result);
    bool CheckInput();
    void OnChangedInput(wxCommandEvent& event);

    HexBedMainFrame* parent_;
    HexBedContextMain* context_;
    std::shared_ptr<HexBedDocument> document_;
    HexBedStandaloneEditor* editor_;
    wxChoice* opChoice_;
    wxButton* okButton_;
    BitwiseBinaryOp choice_{BitwiseBinaryOp::Add};
};

};  // namespace ui

};  // namespace hexbed

#endif /* HEXBED_UI_DIALOG_BITOPBINARY_HH */
