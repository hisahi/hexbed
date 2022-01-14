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
// ui/settings/validate.cc -- impl for common preference validators

#include "ui/settings/validate.hh"

#include <wx/clrpicker.h>
#include <wx/fontpicker.h>

#include "common/logger.hh"
#include "ui/config.hh"

namespace hexbed {

namespace ui {

GenericLongValidator::GenericLongValidator(long* store)
    : wxGenericValidator(&temp_), store_(store) {}

wxObject* GenericLongValidator::Clone() const {
    return new GenericLongValidator(store_);
}

bool GenericLongValidator::TransferFromWindow() {
    if (!wxGenericValidator::TransferFromWindow()) return false;
    *store_ = static_cast<long>(temp_);
    return true;
}

bool GenericLongValidator::TransferToWindow() {
    temp_ = static_cast<int>(*store_);
    return wxGenericValidator::TransferToWindow();
}

GenericStringValidator::GenericStringValidator(string* store)
    : wxGenericValidator(&temp_), store_(store) {}

wxObject* GenericStringValidator::Clone() const {
    return new GenericStringValidator(store_);
}

bool GenericStringValidator::TransferFromWindow() {
    if (!wxGenericValidator::TransferFromWindow()) return false;
    *store_ = temp_;
    return true;
}

bool GenericStringValidator::TransferToWindow() {
    temp_ = *store_;
    return wxGenericValidator::TransferToWindow();
}

ColourValidator::ColourValidator(long* store) : wxValidator(), store_(store) {}

wxObject* ColourValidator::Clone() const { return new ColourValidator(*this); }

bool ColourValidator::TransferFromWindow() {
    wxColourPickerCtrl* picker = wxDynamicCast(GetWindow(), wxColourPickerCtrl);
    if (picker) {
        *store_ = picker->GetColour().GetRGB();
        return true;
    } else
        return false;
}

bool ColourValidator::TransferToWindow() {
    wxColourPickerCtrl* picker = wxDynamicCast(GetWindow(), wxColourPickerCtrl);
    if (picker) {
        picker->SetColour(wxColour(*store_));
        return true;
    } else
        return false;
}

bool ColourValidator::Validate(wxWindow* parent) { return true; }

FontValidator::FontValidator(string* store) : wxValidator(), store_(store) {}

wxObject* FontValidator::Clone() const { return new FontValidator(*this); }

bool FontValidator::TransferFromWindow() {
    wxFontPickerCtrl* picker = wxDynamicCast(GetWindow(), wxFontPickerCtrl);
    if (picker) {
        *store_ = hexFontToString(picker->GetSelectedFont());
        return true;
    } else
        return false;
}

bool FontValidator::TransferToWindow() {
    wxFontPickerCtrl* picker = wxDynamicCast(GetWindow(), wxFontPickerCtrl);
    if (picker) {
        picker->SetSelectedFont(getHexFontOrDefault(*store_));
        return true;
    } else
        return false;
}

bool FontValidator::Validate(wxWindow* parent) { return true; }

};  // namespace ui

};  // namespace hexbed
