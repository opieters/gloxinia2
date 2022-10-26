/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "treemodel.h"
#include "treeitem.h"
#include "gcsensor.h"
#include "gcnode.h"

#include <QtWidgets>

TreeModel::TreeModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    rootItem = new TreeItem("Example", TreeItem::NodeType::Root);
    //setupModelData(data.split('\n'), rootItem);
}

TreeModel::~TreeModel()
{
    delete rootItem;
}

int TreeModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return rootItem->columnCount();
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    TreeItem *item = getItem(index);

    if(item == nullptr){
        return QVariant();
    }

    QVariant data = item->data();
    GCSensor* s;
    GCNode* n;

    switch(role)
    {
        case Qt::DisplayRole:
            switch(item->getNType())
            {
                case TreeItem::NodeType::Sensor:
                    s = data.value<GCSensor*>();
                    if(s != nullptr) { return s->toString(); }
                    break;
                case TreeItem::NodeType::Root:
                    return "Root Node";
                    break;
                case TreeItem::NodeType::Node:
                    n = data.value<GCNode*>();
                    if(n != nullptr) { return n->toString(); }
                    break;
            }
        case Qt::EditRole:
            return item->data();
        case Qt::DecorationRole:
            return item->getIcon();
        default:
            return QVariant();
    }


    return QVariant();
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
}

TreeItem *TreeModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
        if (item)
            return item;
    }
    return rootItem;
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

    TreeItem *parentItem = getItem(parent);
    if (!parentItem)
        return QModelIndex();

    TreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    return QModelIndex();
}


bool TreeModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    TreeItem *parentItem = getItem(parent);
    if (!parentItem)
        return false;

    beginInsertRows(parent, position, position + rows - 1);
    const bool success = parentItem->insertChildren(position,
                                                    rows,
                                                    rootItem->columnCount());
    endInsertRows();

    return success;
}

QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem* childItem = getItem(index);
    TreeItem* parentItem = childItem != nullptr ? childItem->parent() : nullptr;

    if (parentItem == rootItem || !parentItem)
        return QModelIndex();

    return createIndex(parentItem->childNumber(), 0, parentItem);
}



bool TreeModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    TreeItem *parentItem = getItem(parent);
    if (!parentItem)
        return false;

    beginRemoveRows(parent, position, position + rows - 1);
    const bool success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    return success;
}

int TreeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() > 0)
        return 0;

    const TreeItem *parentItem = getItem(parent);

    return parentItem ? parentItem->childCount() : 0;
}

bool TreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
    {
        return false;
    }

    TreeItem *item = getItem(index);
    // TODO: free memory here if value is non-null pointer?
    bool result = item->setData(value);

    if (result)
    {
        emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
    }

    return result;
}


bool TreeModel::fromTextConfig(const QStringList &lines)
{
    int number = 0;
    int position;
    QString line;
    QStringList arguments;

    bool openQuote = false;
    int startIdx, stopIdx;

    // loop over all lines
    while(number < lines.count())
    {
        position = 0;
        line = lines[number];

        // if the first char is a #, skip
        if((line.length() > position) && (lines[number].at(position) == '#'))
        {
            number++;
            continue;
        }

        // split the data (\\s+ is also possible, but will not preserve whitespace)
        arguments = line.split(" ");

        // look for backslash to escape a space
        int i = 0;
        while(i < arguments.count())
        {
            if((arguments[i].length() > 0) && (arguments[i].at(arguments[i].length()-1) == '\\'))
            {
                arguments[i].remove(arguments[i].length()-1, 1);
                arguments[i].append(" ");
                arguments[i].append(arguments[i+1]);
                arguments.remove(i+1);
            }

            i++;
        }

        // remove empty cells
        /*i = 0;
        while(i < arguments.count())
        {
            if(arguments[i].isEmpty())
            {
                arguments.remove(i);
                i--;
            }

            i++;
        }*/

        if(arguments.count() <=1 ){
            number++;
            continue;
        }

        if(arguments[arguments.count()-1].at(arguments[arguments.count()-1].length()-1) != ';'){
            number++;
            continue;
        }

        // remove semicolon at the end
        if(arguments[arguments.count()-1].length() > 1)
            arguments[arguments.count()-1].remove(arguments[arguments.count()-1].length()-1, 1);
        else
            arguments.remove(arguments.count()-1);

        // parse actual content
        if(arguments[0] == "N")
        {
            parseNode(arguments, rootItem);
        }
        if((arguments[0] == "S") && (rootItem->childCount() > 0))
        {
            parseSensor(arguments, rootItem->child(rootItem->childCount() - 1 ));
        }

        // move to the next line
        number++;
    }

    return true;
}

bool TreeModel::parseNode(const QStringList &args, TreeItem* parent)
{
    bool success = false;

    if(parent == nullptr)
        return false;

    if(args.count() < 4){
        return false;
    }

    GCNode node;
    success = node.fromConfigString(args);

    // add node to structure
    if(success)
    {
        beginInsertRows(QModelIndex(), parent->childCount(), parent->childCount() + 1);
        parent->insertChildren(parent->childCount(), 1, rootItem->columnCount());
        parent->child(parent->childCount() - 1)->setData(QVariant::fromValue(new GCNode(node)));
        endInsertRows();
    }

    return success;

}
bool TreeModel::parseSensor(const QStringList &args, TreeItem* parent)
{
    bool status = false;
    GCSensor* sensorPointer;

    if(parent == nullptr)
        return false;

    if(args.count() < 2){
        return false;
    }

    //S SHT35 0 temp/rh 0x44 SS H 0.5 0;
    if(args[1] == "SHT35")
    {
        GCSensorSHT35 s;
        status = s.fromConfigString(args);

        if(status)
        {
            sensorPointer = new GCSensorSHT35(s);
            parent->insertChildren(parent->childCount(), 1, rootItem->columnCount());
            parent->child(parent->childCount() - 1)->setData(QVariant::fromValue(sensorPointer));
        }
    } else if(args[1] == "APDS9306")
    {
        GCSensorAPDS9306 s;
        status = s.fromConfigString(args);

        if(status)
        {
            sensorPointer = new GCSensorAPDS9306(s);
            parent->insertChildren(parent->childCount(), 1, rootItem->columnCount());
            parent->child(parent->childCount() - 1)->setData(QVariant::fromValue(sensorPointer));
        }
    }

    return status;
}


QStringList TreeModel::toTextConfig(void)
{
    QVariant nodeData;
    TreeItem* nodeItem;

    QStringList textConfig;

    for(int i = 0; i < rootItem->childCount(); i++)
    {
        nodeItem = rootItem->child(i);
        nodeData = nodeItem->data();
        GCNode* node = nodeData.value<GCNode*>();
        if(node == nullptr)
            continue;

        textConfig.append(node->toConfigString());

        QVariant sensorData;
        for(int j = 0; j < nodeItem->childCount(); j++){
            sensorData = nodeItem->child(j)->data();
            GCSensor* sensor = sensorData.value<GCSensor*>();

            if(sensor == nullptr)
                continue;

            textConfig.append(sensor->toConfigString());
        }

        textConfig.append("\n");
    }

    return textConfig;
}

