/* $BEGIN_LICENSE

This file is part of Musique.
Copyright 2013, Flavio Tordini <flavio.tordini@gmail.com>

Musique is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Musique is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Musique.  If not, see <http://www.gnu.org/licenses/>.

$END_LICENSE */

#include "globalshortcutbackend.h"
#include "globalshortcuts.h"

GlobalShortcutBackend::GlobalShortcutBackend(GlobalShortcuts *parent)
    : QObject(parent),
    manager_(parent),
    active_(false) { }

bool GlobalShortcutBackend::Register() {
    bool ret = DoRegister();
    if (ret)
        active_ = true;
    return ret;
}

void GlobalShortcutBackend::Unregister() {
    DoUnregister();
    active_ = false;
}

void GlobalShortcutBackend::Reregister() {
    Unregister();
    Register();
}
