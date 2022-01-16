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
// ui/tools/textconv.cc -- impl for the text converter tool

#include "ui/tools/textconv.hh"

#include <wx/artprov.h>
#include <wx/button.h>
#include <wx/choice.h>
#include <wx/msgdlg.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#include <type_traits>

#include "app/config.hh"
#include "app/encoding.hh"
#include "common/buffer.hh"
#include "ui/encoding.hh"
#include "ui/hexbed.hh"
#include "ui/settings/validate.hh"

namespace hexbed {

namespace ui {

template <typename T, typename T2>
wxButton* new_wxButton(wxWindow* parent, wxWindowID id, const wxString& label,
                       T callable, T2* handler, long style = 0) {
    wxButton* button = new wxButton(parent, id, label, wxDefaultPosition,
                                    wxDefaultSize, style);
    button->Bind(wxEVT_BUTTON, callable, handler);
    return button;
}

TextConverterTool::TextConverterTool(HexBedMainFrame* parent,
                                     std::shared_ptr<HexBedContextMain> context,
                                     std::shared_ptr<HexBedDocument> document)
    : wxDialog(parent, wxID_ANY, _("Text converter"), wxDefaultPosition,
               wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
      parent_(parent),
      context_(context),
      document_(document) {
    wxBoxSizer* top = new wxBoxSizer(wxVERTICAL);

    textInput_ =
        new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
                       wxDefaultSize, wxTE_MULTILINE | wxHSCROLL);

    decltype(document) copy = document;
    editor_ = new HexBedStandaloneEditor(this, context.get(), std::move(copy));

    wxBoxSizer* rowTop = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* rowMid = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* rowBot = new wxBoxSizer(wxHORIZONTAL);

    wxString choices = "Default ()";
    encodingList_ = new wxChoice(
        this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 1, &choices, 0,
        ChoiceValidator<string>(&encoding_, std::vector<string>{string()}));

    rowTop->Add(new wxStaticText(this, wxID_ANY, _("Encoding")),
                wxSizerFlags().Center().Border(wxALL, 4));
    rowTop->Add(encodingList_);

    wxButton* btnDown =
        new_wxButton(this, wxID_ANY, wxEmptyString,
                     &TextConverterTool::OnTextToBytes, this, wxBU_NOTEXT);
    btnDown->SetToolTip(_("Convert text to bytes"));
    wxButton* btnUp =
        new_wxButton(this, wxID_ANY, wxEmptyString,
                     &TextConverterTool::OnBytesToText, this, wxBU_NOTEXT);
    btnUp->SetToolTip(_("Convert bytes to text"));

    wxBitmap arrowDown = wxArtProvider::GetBitmap(wxART_GO_DOWN);
    if (arrowDown.IsOk() && arrowDown.GetWidth() >= 16 &&
        arrowDown.GetHeight() >= 16)
        btnDown->SetBitmap(arrowDown);
    else
        btnDown->SetLabel("\\/");

    wxBitmap arrowUp = wxArtProvider::GetBitmap(wxART_GO_UP);
    if (arrowUp.IsOk() && arrowUp.GetWidth() >= 16 && arrowUp.GetHeight() >= 16)
        btnUp->SetBitmap(arrowUp);
    else
        btnUp->SetLabel("/\\");

    rowMid->AddStretchSpacer();
    rowMid->Add(btnDown, wxSizerFlags().Center());
    rowMid->AddStretchSpacer();
    rowMid->Add(btnUp, wxSizerFlags().Center());
    rowMid->AddStretchSpacer();

    rowBot->Add(new wxStaticText(this, wxID_ANY, _("New lines")),
                wxSizerFlags().Center().Border(wxALL, 4));
    wxString newlineChoices[4] = {/// newline mode
                                  _("Raw"),
                                  /// newline mode
                                  _("LF"),
                                  /// newline mode
                                  _("CR+LF"),
                                  /// newline mode
                                  _("CR")};
    rowBot->Add(
        new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 4,
                     newlineChoices, 0,
                     ChoiceIndexValidator<std::underlying_type_t<NewlineMode>>(
                         reinterpret_cast<std::underlying_type_t<NewlineMode>*>(
                             &newlines_))),
        wxSizerFlags().Center());
    rowBot->AddStretchSpacer();
    wxButton* copyButton =
        new_wxButton(this, wxID_ANY, _("From Editor"),
                     &TextConverterTool::OnCopySelection, this);
    copyButton->SetToolTip(
        _("Copies the selected block of bytes from the open file into the text "
          "converter"));
    wxButton* replButton =
        new_wxButton(this, wxID_ANY, _("To Editor"),
                     &TextConverterTool::OnPasteSelection, this);
    replButton->SetToolTip(
        _("Copies the bytes from the text converter and inserts into or "
          "replaces the selection in the open file"));

    rowBot->Add(copyButton, wxSizerFlags().Center());
    rowBot->Add(replButton, wxSizerFlags().Center());

    top->Add(rowTop, wxSizerFlags().Expand());
    top->Add(textInput_, wxSizerFlags().Expand().Proportion(1));
    top->Add(rowMid, wxSizerFlags().Expand());
    top->Add(editor_, wxSizerFlags().Expand().Proportion(1));
    top->Add(rowBot, wxSizerFlags().Expand());

    registration_ = HexBedEditorRegistration(context, editor_);
    SetSizer(top);
    UpdateConfig();
}

static std::initializer_list<string> mbcsKeys_{MBCS_ENCODING_KEYS()};
static std::initializer_list<wxString> mbcsNames_{MBCS_ENCODING_NAMES()};
static std::initializer_list<string> sbcsKeys_{SBCS_ENCODING_KEYS()};
static std::initializer_list<wxString> sbcsNames_{SBCS_ENCODING_NAMES()};

void TextConverterTool::UpdateConfig() {
    ChoiceValidator<string>* validate =
        wxDynamicCast(encodingList_->GetValidator(), ChoiceValidator<string>);

    validate->TruncateItems(1);
    for (std::size_t i = 0; i < mbcsKeys_.size(); ++i)
        validate->AddItem(mbcsKeys_.begin()[i], mbcsNames_.begin()[i]);
    for (std::size_t i = 0; i < sbcsKeys_.size(); ++i)
        validate->AddItem(sbcsKeys_.begin()[i], sbcsNames_.begin()[i]);
    for (std::size_t i = 0, e = hexbed::plugins::charsetPluginCount(); i < e;
         ++i) {
        const auto& pair = hexbed::plugins::charsetPluginByIndex(i);
        validate->AddItem(pair.first, pair.second);
    }

    std::size_t n = sbcsKeys_.size();
    std::size_t i = 0;
    const string& encoding = config().charset;
    for (i = 0; i < n; ++i) {
        if (encoding == sbcsKeys_.begin()[i]) break;
    }
    if (i >= n) i = 0;
    encodingList_->SetString(
        0, wxString::Format(_("Default (%s)"), sbcsNames_.begin()[i]));
    TransferDataToWindow();
}

static bool newlineConvertPop(NewlineMode nl, bool& nlcache, bool& nlnextn,
                              char32_t c, char32_t& u) {
    if (c == '\n' || c == '\r') {
        switch (nl) {
        case NewlineMode::Raw:
            break;
        case NewlineMode::LF:
            if (c == U'\r') {
                nlcache = true;
                c = U'\n';
                break;
            } else if (c == U'\n') {
                if (nlcache) {
                    nlcache = false;
                    return false;
                }
            }
            break;
        case NewlineMode::CRLF:
            if (c == U'\r' || c == U'\n') {
                if (nlcache) {
                    nlcache = false;
                    return false;
                }
                nlcache = true;
                nlnextn = true;
                c = U'\r';
                break;
            }
            break;
        case NewlineMode::CR:
            if (c == U'\n') {
                nlcache = true;
                c = U'\r';
                break;
            } else if (c == U'\r') {
                if (nlcache) {
                    nlcache = false;
                    return false;
                }
            }
            break;
        }
    }
    nlcache = false;
    u = c;
    return true;
}

static void newlineConvertPush(NewlineMode nl, bool& nlcache,
                               u32ostringstream& ss, char32_t c) {
    if (c == U'\n' || c == U'\r') {
        switch (nl) {
        case NewlineMode::Raw:
            break;
        case NewlineMode::LF:
            if (c == U'\r') {
                nlcache = true;
                ss << U'\n';
                return;
            } else if (c == U'\n') {
                if (nlcache) {
                    nlcache = false;
                    return;
                }
            }
            break;
        case NewlineMode::CRLF:
            if (c == U'\r' || c == U'\n') {
                if (nlcache) {
                    nlcache = false;
                    return;
                }
                nlcache = true;
                ss << U'\r' << U'\n';
                return;
            }
            break;
        case NewlineMode::CR:
            if (c == U'\n') {
                nlcache = true;
                ss << U'\r';
                return;
            } else if (c == U'\r') {
                if (nlcache) {
                    nlcache = false;
                    return;
                }
            }
            break;
        }
    }
    nlcache = false;
    ss << c;
}

void TextConverterTool::OnTextToBytes(wxCommandEvent&) {
    TransferDataFromWindow();
    CharEncodeStatus status;
    NewlineMode nl = newlines_;
    std::u32string u =
        wstringToU32string(textInput_->GetValue().ToStdWstring());
    const string& encoding = encoding_.empty() ? config().charset : encoding_;
    bool ok = document_->pry(
        0, document_->size(),
        [nl, &u, &encoding, &status](HexBedTask& task,
                                     std::function<void(const_bytespan)> out) {
            bool nlcache = false, nlnextn = false;
            std::size_t n = u.size();
            const char32_t* arr = u.data();
            status = getCharacterEncodingByName(encoding).encode(
                [nl, &n, &nlcache, &nlnextn, &arr](u32span s) -> bufsize {
                    if (nl == NewlineMode::Raw) {
                        std::size_t r = std::min<std::size_t>(n, s.size());
                        for (std::size_t i = 0; i < r; ++i, --n) s[i] = *arr++;
                        return r;
                    }
                    std::size_t i = 0, q = s.size();
                    char32_t u;
                    while ((n || nlnextn) && i < q) {
                        if (nlnextn) {
                            s[i++] = U'\n';
                            nlnextn = false;
                            continue;
                        }
                        if (newlineConvertPop(nl, nlcache, nlnextn, *arr++, u))
                            s[i++] = u;
                        --n;
                    }
                    return i;
                },
                out);
            if (!status.ok) task.cancel();
        });
    if (!ok) {
        wxMessageBox(_("The entered text cannot be represented in the selected "
                       "encoding."),
                     "HexBed", wxOK | wxICON_ERROR);
        return;
    }
}

void TextConverterTool::OnBytesToText(wxCommandEvent&) {
    TransferDataFromWindow();
    u32ostringstream ss;
    HexBedDocument& doc = *document_;
    bufsize o = 0;
    NewlineMode nl = newlines_;
    bool nlcache = false;
    const string& encoding = encoding_.empty() ? config().charset : encoding_;
    CharDecodeStatus status = getCharacterEncodingByName(encoding).decode(
        [&doc, &o](bytespan b) -> bufsize {
            bufsize n = doc.read(o, b);
            o += n;
            return n;
        },
        [nl, &ss, &nlcache](const_u32span u) {
            if (nl == NewlineMode::Raw) {
                ss.write(u.data(), u.size());
                return;
            }
            for (std::size_t i = 0, n = u.size(); i < n; ++i)
                newlineConvertPush(nl, nlcache, ss, u[i]);
        });
    if (!status.ok) {
        wxMessageBox(_("The bytes do not represent a valid encoding."),
                     "HexBed", wxOK | wxICON_ERROR);
        return;
    }
    textInput_->SetValue(u32stringToWstring(ss.str()));
}

void TextConverterTool::OnCopySelection(wxCommandEvent&) {
    hexbed::ui::HexEditorParent* editor = parent_->GetCurrentEditor();
    if (editor) {
        bufsize sel, seln;
        bool seltext;
        editor->GetSelection(sel, seln, seltext);
        const HexBedDocument& doc = editor->document();
        document_->pry(
            0, document_->size(),
            [sel, seln, &doc](HexBedTask& task,
                              std::function<void(const_bytespan)> outp) {
                bufsize o = sel, n = seln;
                byte buf[BUFFER_SIZE];
                while (!task.isCancelled() && n) {
                    task.progress(o);
                    bufsize rem = std::min<bufsize>(n, sizeof(buf));
                    bufsize q = doc.read(o, bytespan{buf, rem});
                    outp(const_bytespan{buf, q});
                    o += q;
                    n -= q;
                }
            },
            seln);
    }
}

void TextConverterTool::OnPasteSelection(wxCommandEvent&) {
    hexbed::ui::HexEditorParent* editor = parent_->GetCurrentEditor();
    if (editor) {
        bufsize nn = document_->size();
        bufsize sel, seln;
        bool seltext;
        editor->GetSelection(sel, seln, seltext);
        const HexBedDocument& doc = *document_;
        editor->document().pry(
            sel, seln,
            [nn, &doc](HexBedTask& task,
                       std::function<void(const_bytespan)> outp) {
                bufsize o = 0, n = nn;
                byte buf[BUFFER_SIZE];
                while (!task.isCancelled() && n) {
                    task.progress(o);
                    bufsize rem = std::min<bufsize>(n, sizeof(buf));
                    bufsize q = doc.read(o, bytespan{buf, rem});
                    outp(const_bytespan{buf, q});
                    o += q;
                    n -= q;
                }
            },
            nn);
    }
}

};  // namespace ui

};  // namespace hexbed
