//
// Zero Editor
// Copyright (C) 2018 Matthias Bolte <matthias.bolte@googlemail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include "settings.h"

// Use a QSettings pointer here to avoid potential static initialization order problems
QSettings *Settings::s_settings = NULL;

// static
void Settings::initialize()
{
    s_settings = new QSettings("Majestic42", "ZeroEditor"); // FIXME: This leaks memory
}
