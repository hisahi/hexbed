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
// file/context.hh -- header for the HexBed context class

#ifndef HEXBED_FILE_CONTEXT_HH
#define HEXBED_FILE_CONTEXT_HH

#include "common/types.hh"

namespace hexbed {

struct HexBedDocument;
struct HexBedTaskHandler;

enum class FailureResponse { Abort, Retry, Ignore };

class HexBedContext {
  public:
    inline virtual HexBedTaskHandler* getTaskHandler() { return nullptr; }

    inline virtual bool shouldBackup() { return false; }
    inline virtual FailureResponse ifBackupFails(const string& message) {
        return FailureResponse::Abort;
    }

    inline void announceFileChanged(HexBedDocument* doc) {
        announceBytesChanged(doc, 0);
    }
    inline virtual void announceBytesChanged(HexBedDocument* doc,
                                             bufsize start) {}
    inline virtual void announceBytesChanged(HexBedDocument* doc, bufsize start,
                                             bufsize length) {}
    inline virtual void announceUndoChange(HexBedDocument* doc) {}
};

};  // namespace hexbed

#endif /* HEXBED_FILE_CONTEXT_HH */
