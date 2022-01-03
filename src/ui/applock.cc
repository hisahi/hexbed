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
// ui/applock.cc -- impl for application single-instance lock

#include "ui/applock.hh"

namespace hexbed {

static std::string getMutexName() { return "/tmp/hexbed.lock"; }

AppLock::AppLock(std::function<void(const std::string&)> f)
    : mutexName_(getMutexName()), knock_(f) {
    server_ = new AppLockServer(this);
}

bool AppLock::acquire(const std::string& token) {
    if (single_.IsAnotherRunning()) {
        wxClient* client = new wxClient;
        wxConnectionBase* conn =
            client->MakeConnection("localhost", mutexName_, "applock");
        conn->Execute("!" + token);
        delete conn;
        delete client;
        return false;
    }
    server_->Create(mutexName_);
    return true;
}

void AppLock::release() {
    // wxSingleInstanceChecker self-destructs
    delete server_;
}

void AppLock::knock(const std::string& token) { knock_(token); }

bool AppLockConnection::OnExec(const wxString& topic, const wxString& data) {
    lock_->knock(data.ToStdString().substr(1));
    return true;
}

};  // namespace hexbed
