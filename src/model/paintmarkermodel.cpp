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

#include "paintmarkermodel.h"

#include <manager/octmarkermanager.h>
#include <markermodules/bscanmarkerbase.h>

QString PaintMarkerItem::getName() const
{
	return marker->getName();
}

PaintMarkerModel::PaintMarkerModel()
{
	OctMarkerManager& manager = OctMarkerManager::getInstance();
	const std::vector<BscanMarkerBase*>& octMarkers = manager.getBscanMarker();

	for(const BscanMarkerBase* marker : octMarkers)
		markers.emplace_back(marker);
}


QVariant PaintMarkerModel::data(const QModelIndex& index, int role) const
{
	if(!index.isValid())
		return QVariant();

	std::size_t row = static_cast<std::size_t>(index.row());
	if(row >= markers.size() || index.row() < 0)
		return QVariant();

	if(role == Qt::CheckStateRole)
	{
		if(index.column() == 1)
		{
			const PaintMarkerItem& item = markers.at(row);
			return item.isShowed() ? Qt::Checked : Qt::Unchecked;
		}
	}


	if(role == Qt::DisplayRole)
	{
		const PaintMarkerItem& item = markers.at(row);

		switch(index.column())
		{
			case 0:
				return item.getName();
		}
	}

	return QVariant();
}

bool PaintMarkerModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if(!index.isValid())
		return false;

	std::size_t row = static_cast<std::size_t>(index.row());
	if(row >= markers.size() || index.row() < 0)
		return false;

	if(role == Qt::CheckStateRole)
	{
		if(index.column() == 1)
		{
			PaintMarkerItem& item = markers.at(row);
			item.setShow(value.toBool());

			emit(viewChanged());
			return true;
		}
	}
	return false;
}

Qt::ItemFlags PaintMarkerModel::flags(const QModelIndex& index) const
{
	if(!index.isValid())
		return 0;

	if(index.column() == 1)
		return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;

	return QAbstractTableModel::flags(index);
}


int PaintMarkerModel::columnCount(const QModelIndex& /*parent*/) const
{
	return 2;
}

int PaintMarkerModel::rowCount(const QModelIndex& /*parent*/) const
{
	return static_cast<int>(markers.size());
}


QVariant PaintMarkerModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if(role != Qt::DisplayRole)
		return QVariant();

	if(orientation == Qt::Horizontal)
	{
		switch(section)
		{
			case 0:
				return tr("marker name");
			case 1:
				return tr("show");
		}
	}
	else
		return QString("%1").arg(section);

	return QVariant();
}

void PaintMarkerModel::slotClicked(QModelIndex index)
{

	OctMarkerManager& manager = OctMarkerManager::getInstance();

	manager.setBscanMarker(index.row());
}
