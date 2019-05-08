/*
 * Copyright (c) 2018 Kay Gawlik <kaydev@amarunet.de> <kay.gawlik@beuth-hochschule.de> <kay.gawlik@charite.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include <QObject>



/**
 * @ingroup HelperClasses
 * @brief Blocks the signal until the end of the lifetime of this object
 *
 */
class SignalBlocker
{
public:
	SignalBlocker(QObject* obj) : obj(obj)                          { obj->blockSignals(true ); }
	~SignalBlocker()                                                { obj->blockSignals(false); }

private:
	QObject* obj;
};
