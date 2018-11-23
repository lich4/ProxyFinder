import QtQuick 2.6
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.1
import QtQuick.Window 2.2
import QtQuick 2.4
import Material 0.2
import Material.Extras 0.1
import Material.ListItems 0.1 as ListItem

ColumnLayout{
    id: root_basic_config
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: dp(32)
        RowLayout {
            id: row1
            anchors.left: parent.left
            anchors.right: parent.right
            TextField {
                id: text_search_source
                anchors.left: parent.left
                anchors.right: parent.right
                text: "proxy"
                placeholderText: "代理搜索源"
                floatingLabel: true
                onTextChanged: {
                    if (!root_basic_config.init) {
                        return
                    }
                    commu.set_search_source(text_search_source.text)
                }
            }
        }
        RowLayout {
            id: row2
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: row1.bottom
            anchors.topMargin: dp(16)
            TextField {
                id: text_validate_source
                anchors.left: parent.left
                anchors.right: text_validate_key.left
                anchors.rightMargin: dp(16)
                text: "http://www.guimp.com"
                placeholderText: "验证URL"
                floatingLabel: true
                onTextChanged: {
                    if (!root_basic_config.init) {
                        return
                    }
                    commu.set_validate_source(text_validate_source.text)
                }
            }
            TextField {
                id: text_validate_key
                anchors.right : parent.right
                text: "html"
                placeholderText: "关键字"
                floatingLabel: true
                onTextChanged: {
                    if (!root_basic_config.init) {
                        return
                    }
                    commu.set_validate_key(text_validate_key.text)
                }
            }
        }
        RowLayout {
            id: row3
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: row2.bottom
            anchors.topMargin: dp(16)
            Label {
                id: label_search_timeout
                text: "搜索超时时间:%1毫秒".arg(text_search_timeout.value)
                Layout.alignment:  Qt.AlignBottom
            }
            Slider {
                id: text_search_timeout
                anchors.left: label_search_timeout.right
                anchors.right: parent.right
                anchors.leftMargin: dp(32)
                Layout.alignment: Qt.AlignCenter
                value: 5000
                tickmarksEnabled: true
                numericValueLabel: true
                stepSize: 200
                minimumValue: 200
                maximumValue: 5000
                onValueChanged: {
                    if (!root_basic_config.init) {
                        return
                    }
                    commu.set_search_timeout(text_search_timeout.value)
                }
            }
        }
        RowLayout {
            id: row4
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: row3.bottom
            Label {
                id: label_validate_timeout
                text: "验证超时时间:%1毫秒".arg(text_validate_timeout.value)
                Layout.alignment:  Qt.AlignBottom
            }
            Slider {
                id: text_validate_timeout
                anchors.left: label_validate_timeout.right
                anchors.right: parent.right
                anchors.leftMargin: dp(32)
                Layout.alignment: Qt.AlignCenter
                value: 5000
                tickmarksEnabled: true
                numericValueLabel: true
                stepSize: 200
                minimumValue: 200
                maximumValue: 5000
                onValueChanged: {
                    if (!root_basic_config.init) {
                        return
                    }
                    commu.set_validate_timeout(text_validate_timeout.value)
                }
            }
        }
        RowLayout {
            id: row5
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: row4.bottom
            Label {
                id: label_validate_count
                text: "验证次数:%1".arg(text_validate_count.value)
                Layout.alignment:  Qt.AlignBottom
            }
            Slider {
                id: text_validate_count
                anchors.left: label_validate_count.right
                anchors.right: parent.right
                anchors.leftMargin: dp(32)
                Layout.alignment: Qt.AlignCenter
                value: 3
                tickmarksEnabled: true
                numericValueLabel: true
                stepSize: 1
                minimumValue: 1
                maximumValue: 9
                onValueChanged: {
                    if (!root_basic_config.init) {
                        return
                    }
                    commu.set_validate_count(text_validate_count.value)
                }
            }
        }
        RowLayout {
            id: row6
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: row5.bottom
            Label {
                id: label_thread_count
                text: "线程数:%1".arg(text_thread_count.value)
                Layout.alignment:  Qt.AlignBottom
            }
            Slider {
                id: text_thread_count
                anchors.left: label_thread_count.right
                anchors.right: parent.right
                anchors.leftMargin: dp(32)
                Layout.alignment: Qt.AlignCenter
                value: 60
                tickmarksEnabled: true
                numericValueLabel: true
                stepSize: 10
                minimumValue: 10
                maximumValue: 100
                onValueChanged: {
                    if (!root_basic_config.init) {
                        return
                    }
                    commu.set_thread_count(text_thread_count.value)
                }
            }
        }
        RowLayout {
            anchors.left: parent.left
            anchors.right: parent.right
            CheckBox {
                id: check_dns
                anchors.left: parent.left
                text:"使用DNS解析IP"
                checked: false
                onCheckedChanged: {
                    if (!root_basic_config.init) {
                        return
                    }
                    commu.set_fulldns(checked)
                }
            }
            TextField {
                id: text_dns_source
                anchors.left: check_dns.right
                anchors.right: parent.right
                anchors.leftMargin: dp(32)
                text: "https://public-dns.info/nameservers.txt"
                placeholderText: "DNS源"
                floatingLabel: true
                onTextChanged: {
                    if (!root_basic_config.init) {
                        return
                    }
                    commu.set_dns_source(text_dns_source.text)
                }
            }
        }
        RowLayout {
            anchors.left: parent.left
            anchors.right: parent.right
            TextField {
                id: text_httpserv_port
                anchors.left: parent.left
                anchors.right: parent.right
                text: "81"
                placeholderText: "HTTP服务器端口"
                floatingLabel: true
                onTextChanged: {
                    if (!root_basic_config.init) {
                        return
                    }
                    commu.set_httpserv_port(text_httpserv_port.text)
                }
            }
        }
    }
    Component.onCompleted: {
        restore()
    }
    property var init: false
    function restore() {
        text_search_source.text = commu.get_search_source()
        text_validate_source.text = commu.get_validate_source()
        text_validate_key.text = commu.get_validate_key()
        text_search_timeout.value = commu.get_search_timeout()
        text_validate_timeout.value = commu.get_validate_timeout()
        text_validate_count.value = commu.get_validate_count()
        text_thread_count.value = commu.get_thread_count()
        text_dns_source.text = commu.get_dns_source()
        text_httpserv_port.text = commu.get_httpserv_port()
        init = true
    }
}

