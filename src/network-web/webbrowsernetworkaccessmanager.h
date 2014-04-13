// This file is part of RSS Guard.
//
// Copyright (C) 2011-2014 by Martin Rotter <rotter.martinos@gmail.com>
//
// RSS Guard is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// RSS Guard is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with RSS Guard. If not, see <http://www.gnu.org/licenses/>.

#ifndef WEBBROWSERNETWORKACCESSMANAGER_H
#define WEBBROWSERNETWORKACCESSMANAGER_H

#include "network-web/basenetworkaccessmanager.h"

#include <QPointer>


// This is network access manager for web browsers.
class WebBrowserNetworkAccessManager : public BaseNetworkAccessManager {
    Q_OBJECT

  public:
    // Constructors and destructors.
    explicit WebBrowserNetworkAccessManager(QObject *parent = 0);
    virtual ~WebBrowserNetworkAccessManager();

    // Returns pointer to global network access manager
    // used by ALL web browsers.
    // NOTE: All web browsers use shared network access manager,
    // which makes setting of custom network settings easy.
    static WebBrowserNetworkAccessManager *instance();

  protected slots:
    void onAuthenticationRequired(QNetworkReply * reply, QAuthenticator *authenticator);

  private:
    static QPointer<WebBrowserNetworkAccessManager> s_instance;
};

#endif // WEBBROWSERNETWORKACCESSMANAGER_H