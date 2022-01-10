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
// ui/settings/validate.hh -- header for common preference validators

#ifndef HEXBED_UI_SETTINGS_VALIDATE_HH
#define HEXBED_UI_SETTINGS_VALIDATE_HH

#include <wx/choice.h>
#include <wx/valgen.h>
#include <wx/validate.h>

#include <algorithm>
#include <string>
#include <vector>

namespace hexbed {

namespace ui {

class GenericLongValidator : public wxGenericValidator {
  public:
    GenericLongValidator(long* store);
    wxObject* Clone() const override;
    bool TransferFromWindow() override;
    bool TransferToWindow() override;

  private:
    int temp_;
    long* store_;
};

class GenericStringValidator : public wxGenericValidator {
  public:
    GenericStringValidator(std::string* store);
    wxObject* Clone() const override;
    bool TransferFromWindow() override;
    bool TransferToWindow() override;

  private:
    wxString temp_;
    std::string* store_;
};

template <typename T>
class ChoiceValidator : public wxValidator {
  public:
    ChoiceValidator(T* store, std::vector<T>&& values)
        : wxValidator(), store_(store), values_(std::move(values)) {}

    wxObject* Clone() const { return new ChoiceValidator(*this); }

    bool TransferFromWindow() {
        wxChoice* picker = wxDynamicCast(GetWindow(), wxChoice);
        if (picker && picker->GetSelection() != wxNOT_FOUND) {
            *store_ = values_[picker->GetSelection()];
            return true;
        } else
            return false;
    }

    bool TransferToWindow() {
        wxChoice* picker = wxDynamicCast(GetWindow(), wxChoice);
        if (picker) {
            auto it = std::find(values_.begin(), values_.end(), *store_);
            picker->SetSelection(it == values_.end() ? -1
                                                     : it - values_.begin());
            return true;
        } else
            return false;
    }

    bool Validate(wxWindow* parent) {
        wxChoice* picker = wxDynamicCast(GetWindow(), wxChoice);
        if (!picker) return true;
        return picker->GetSelection() != wxNOT_FOUND;
    }

    void AddItem(const T& value, const wxString& text) {
        wxItemContainer* picker = dynamic_cast<wxItemContainer*>(GetWindow());
        values_.reserve(values_.size() + 1);
        picker->Append(text);
        values_.push_back(value);
    }

  private:
    T* store_;
    std::vector<T> values_;
};

template <typename T>
class ChoiceIndexValidator : public wxValidator {
  public:
    ChoiceIndexValidator(T* store) : wxValidator(), store_(store) {}

    wxObject* Clone() const { return new ChoiceIndexValidator(*this); }

    bool TransferFromWindow() {
        wxChoice* picker = wxDynamicCast(GetWindow(), wxChoice);
        if (picker && picker->GetSelection() != wxNOT_FOUND) {
            *store_ = picker->GetSelection();
            return true;
        } else
            return false;
    }

    bool TransferToWindow() {
        wxChoice* picker = wxDynamicCast(GetWindow(), wxChoice);
        if (picker) {
            picker->SetSelection(*store_);
            return true;
        } else
            return false;
    }

    bool Validate(wxWindow* parent) {
        wxChoice* picker = wxDynamicCast(GetWindow(), wxChoice);
        if (!picker) return true;
        return picker->GetSelection() != wxNOT_FOUND;
    }

  private:
    T* store_;
};

class ColourValidator : public wxValidator {
  public:
    ColourValidator(long* store);
    wxObject* Clone() const override;
    bool TransferFromWindow() override;
    bool TransferToWindow() override;
    bool Validate(wxWindow* parent) override;

  private:
    long* store_;
};

class FontValidator : public wxValidator {
  public:
    FontValidator(std::string* store);
    wxObject* Clone() const override;
    bool TransferFromWindow() override;
    bool TransferToWindow() override;
    bool Validate(wxWindow* parent) override;

  private:
    std::string* store_;
};

};  // namespace ui

};  // namespace hexbed

#endif /* HEXBED_UI_SETTINGS_VALIDATE_HH */
