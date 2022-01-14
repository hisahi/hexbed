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
// ui/applock.hh -- header for application single-instance lock

#ifndef HEXBED_UI_APPLOCK_HH
#define HEXBED_UI_APPLOCK_HH

#include <wx/ipc.h>
#include <wx/snglinst.h>
#include <wx/wx.h>

#include <string>

namespace hexbed {

class AppLock;

class AppLockConnection : public wxConnection {
  public:
    inline AppLockConnection(AppLock* lock) : lock_(lock) {}

  protected:
    bool OnExec(const wxString& topic, const wxString& data);

  private:
    AppLock* lock_;
};

class AppLockServer : public wxServer {
  public:
    inline AppLockServer(AppLock* lock) : lock_(lock) {}
    inline wxConnectionBase* OnAcceptConnection(const wxString& topic) {
        return new AppLockConnection(lock_);
    }

  private:
    AppLock* lock_;
};

class AppLock {
  public:
    AppLock(std::function<void(const wxString&)>);

    bool acquire(const wxString& token);
    void release();
    void knock(const wxString& token);

  private:
    wxString mutexName_;
    std::function<void(const wxString&)> knock_;
    wxSingleInstanceChecker single_;
    AppLockServer* server_;
};

};  // namespace hexbed

#endif /* HEXBED_UI_APPLOCK_HH */
