/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

import QtQuick 2.6
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.1

ApplicationWindow {
    function dumpobj(obj) {
        var des = ""
        for(var name in obj){
          if (typeof obj[name] == "object" && obj[name] !== null) {
            console.log(name + ":" + obj[name])
          }
        }
    }

    function dumpdata(obj, depth, pre) {
        var str = ""
        var l = 0
        var i = 0
        for (i = 0; i < depth; i++) {
            str += " "
        }
        if ("contentItem" in obj) {
            console.log(str + "contentItem:")
            dumpdata(obj.contentItem, depth + 1, pre + "_" + "c")
        }

        console.log(str + pre + "_" + obj)
        if ("data" in obj) {
            l = obj.data.length;
            if (l > 0) {
                console.log(str + "data:")
            }
            for (i = 0; i < l; i++) {
                dumpdata(obj.data[i], depth + 1, pre + "_" + i)
            }
        }
    }

    function update_status(txt) {
        text_status.text = txt
    }

    function update_runlist(row, col, txt) {
        var ele_loc = [
            [1, 0], [1, 1], [1, 2],
            [3, 0], [3, 1], [3, 2],
            [4, 0], [4, 1], [4, 2],
            [5, 0], [5, 1], [5, 2],
            [6, 0], [6, 1], [6, 2],
            [7, 0], [7, 1], [7, 2],
            [8, 0], [8, 1], [8, 2],
            [9, 0], [9, 1], [9, 2],
        ]
        var loc = ele_loc[ 3 * row + col]
        run_list.data[0].data[loc[0]].data[0].data[loc[1]].data[1].text = txt
    }

    onClosing: {
        commu.exit_process()
    }

    id: window
    width: 510
    height: 420
    visible: true
    title: qsTr("代理搜索V1.0.0")
    onOpenglContextCreated: drawer.open()

    //! [orientation]
    readonly property bool inPortrait: window.width < window.height
    //! [orientation]

    ToolBar {
        id: overlayHeader
        z: 1
        width: parent.width
        parent: window.overlay
        height: 0
    }

    Drawer {
        id: drawer
        y: overlayHeader.height
        width: window.width / 3
        height: window.height - overlayHeader.height
        modal: inPortrait
        interactive: inPortrait
        position: inPortrait ? 0 : 1
        visible: inPortrait

        ListView {
            id: menulist
            anchors.fill: parent
            headerPositioning: ListView.OverlayHeader
            header: Pane {
                id: header
                z: 2
                width: parent.width
                contentHeight: logo.height
                Image {
                    id: logo
                    width: parent.width
                    source: "images/qt-logo.png"
                    fillMode: implicitWidth > width ? Image.PreserveAspectFit : Image.Pad
                }
                MenuSeparator {
                    parent: header
                    width: parent.width
                    anchors.verticalCenter: parent.bottom
                    visible: !menulist.atYBeginning
                }
            }

            footer: ItemDelegate {
                id: footer
                text: qsTr("超哥的代理工具")
                width: parent.width
                MenuSeparator {
                    parent: footer
                    width: parent.width
                    anchors.verticalCenter: parent.top
                }
            }
            highlight: Rectangle { color: "#00CED1" }
            model: Qt.fontFamilies()
            delegate: ItemDelegate {
                id: listitem
                text: {
                    if (index == 0)
                        return "配置"
                    else if (index == 1)
                        return "关闭菜单"
                    else if (index == 2)
                        return "关于"
                    else if (index == 3)
                        return "退出程序"
                }
                width: parent.width

                MessageDialog {
                    id: dial_about
                    modality: Qt.ApplicationModal
                    title: "关于"
                    text: "<html>ProxyFinder是一款代理搜索工具<br>作者：超哥<br>QQ：571652571<br>使用方法及最新版本见" +
                          "<a href=https://www.jianshu.com/u/03f17f676926>https://www.jianshu.com/u/03f17f676926</a></html>"
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        menulist.currentIndex = index
                        if (text == "关闭菜单") {
                            drawer.close();
                        } else if (text == "关于") {
                            dial_about.open()
                        } else if (text == "配置") {
                            drawer.close()
                            flickable.visible = false
                            Qt.createComponent("config.qml").createObject(window,
                                                {"x": window.width / 10, "y": window.height / 10});
                        } else if (text == "退出程序") {
                            commu.exit_process()
                        }
                    }
                }
            }
        }
    }

    Flickable {
        id: flickable
        anchors.fill: parent
        topMargin: 20
        leftMargin: 20
        rightMargin: 20

        MouseArea {
            onExited: drawer.close()
            anchors.fill: parent
        }

        Button {
            id: btn_home
            width: 20
            height: 20
            hoverEnabled: true
            Image {
                anchors.fill: parent
                source: "images/qt-home.png"
            }
            Rectangle {
                anchors.fill: parent
                color: "black"
                opacity: parent.pressed ? 0.5 : 0
                ToolTip {
                    text: "主菜单"
                    visible: btn_home.hovered
                }
            }
            onClicked: drawer.open()
        }

        Button {
            id: btn_config
            anchors.left: btn_home.right
            anchors.leftMargin: 20
            width: 20
            height: 20
            Image {
                anchors.fill: parent
                source: "images/qt-config.png"
            }
            Rectangle {
                anchors.fill: parent
                color: "black"
                opacity: parent.pressed ? 0.5 : 0
                ToolTip {
                    text: "配置"
                    visible: btn_config.hovered
                }
            }
            onClicked: {
                flickable.visible = false
                Qt.createComponent("config.qml").createObject(window,
                        {"x": window.width / 10, "y": window.height / 10});
            }
        }

        Button {
            id: btn_run
            anchors.left: btn_config.right
            anchors.leftMargin: 20
            width: 20
            height: 20
            Image {
                anchors.fill: parent
                source: "images/qt-run.png"
            }
            Rectangle {
                anchors.fill: parent
                color: "black"
                opacity: parent.pressed ? 0.5 : 0
                ToolTip {
                    text: "搜索"
                    visible: btn_run.hovered
                }
            }
            onClicked: {
                commu.do_search_proxy();
            }
        }

        Button {
            id: btn_cancel
            anchors.left: btn_run.right
            anchors.leftMargin: 20
            width: 20
            height: 20
            Image {
                anchors.fill: parent
                source: "images/qt-cancel.png"
            }
            Rectangle {
                anchors.fill: parent
                color: "black"
                opacity: parent.pressed ? 0.5 : 0
                ToolTip {
                    text: "取消"
                    visible: btn_cancel.hovered
                }
            }
            onClicked: {
                commu.cancel_search_proxy();
            }
        }

        ListView {
            id: run_list
            anchors.topMargin: 20
            anchors.bottomMargin: 40
            anchors.top: btn_home.bottom
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            contentWidth: headerItem.width

            header: Row {
                function itemAt(index) { return repeater_row.itemAt(index) }
                Repeater {
                    id: repeater_row
                    model: ["代理地址", "真实IP", "延迟(毫秒)"]
                    Label {
                        leftPadding: 30
                        text: modelData
                        font.bold: true
                        font.pixelSize: 20
                        width: (run_list.width - 40) / 3
                        background: Rectangle { color: "silver" }
                    }
                }
            }

            model: 8
            delegate: Column {
                id: delegate
                width: parent.width
                property int row: index
                Row {
                    Repeater {
                        id: repeater_col
                        model: 3
                        ItemDelegate {
                            property int column: index
                            text: {
                                qsTr("------------")
                            }
                            width: run_list.headerItem.itemAt(column).width
                            height: run_list.height / 10
                            onClicked: {
                                window.menu_proxy_row = delegate.row
                                menu_proxy.popup()
                            }
                        }
                    }
                }

                Rectangle {
                    color: "silver"
                    width: parent.width
                    height: 1
                }
            }

        }
        TextEdit {
            id: text_status
            anchors.top: run_list.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            text: "空闲"
        }

        Menu {
            id: menu_proxy
            MenuItem {
                text: "设置代理"
                onTriggered: {
                    commu.do_apply_proxy(window.menu_proxy_row)
                }
            }
            MenuItem {
                text: "取消代理"
                onTriggered: {
                    commu.do_cancel_proxy()
                }
            }
        }
    }

    property int menu_proxy_row: 0
}


