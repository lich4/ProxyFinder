
import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Dialogs 1.1
import QtQuick.Layouts 1.1
import QtQuick.Window 2.0

ColumnLayout {
    id: config_root
    width: 400
    height: 200

    RowLayout {
        Label {
            text: "代理搜索源(分隔符;)："
        }
        TextField {
            id: text_search_source
            Layout.fillWidth: true
            text: config_root.search_source
        }
    }
    RowLayout {
        Label {
            text: "验证URL"
        }
        TextField {
            id: text_validate_source
            Layout.fillWidth: true
            text: config_root.validate_source
        }
    }
    RowLayout {
        Label {
            text: "关键字"
        }
        TextField {
            id: text_validate_key
            Layout.fillWidth: true
            text: config_root.validate_key
        }
    }
    RowLayout {
        Label {
            text: "搜索超时时间(ms)："
        }
        TextField {
            id: text_search_timeout
            Layout.fillWidth: true
            text: config_root.search_timeout
        }
    }
    RowLayout {
        Label {
            text: "验证超时时间(ms)："
        }
        TextField {
            id: text_validate_timeout
            Layout.fillWidth: true
            text: config_root.validate_timeout
        }
    }
    RowLayout {
        Label {
            text: "验证次数："
        }
        TextField {
            id: text_validate_count
            Layout.fillWidth: true
            text: config_root.validate_count
        }
    }
    RowLayout {
        Label {
            text: "线程数："
        }
        TextField {
            id: text_thread_count
            Layout.fillWidth: true
            text: config_root.thread_count
        }
    }
    RowLayout {
        Label {
            text: "DNS源："
        }
        TextField {
            id: text_dns_source
            Layout.fillWidth: true
            text: config_root.dns_source
        }
    }
    RowLayout {
        CheckBox {
            text:"使用DNS解析为IP：";
            checked: false
            onClicked: commu.set_fulldns(checked)
        }
    }
    RowLayout {
        Button {
            text: "设置"
            onClicked: {
                // on_config_change
                config_root.visible = false
                flickable.visible = true
                config_root.updateto();
            }
        }
        Button {
            text: "取消"
            onClicked: {
                config_root.visible = false
                flickable.visible = true
            }
        }
    }

    function updateto() {
        commu.set_search_source(text_search_source.text)
        commu.set_validate_source(text_validate_source.text)
        commu.set_validate_key(text_validate_key.text)
        commu.set_search_timeout(text_search_timeout.text)
        commu.set_validate_timeout(text_validate_timeout.text)
        commu.set_validate_count(text_validate_count.text)
        commu.set_thread_count(text_thread_count.text)
        commu.set_dns_source(text_dns_source.text)
    }

    function updatefrom() {
        text_search_source.text = commu.get_search_source()
        text_validate_source.text = commu.get_validate_source()
        text_validate_key.text = commu.get_validate_key()
        text_search_timeout.text = commu.get_search_timeout()
        text_validate_timeout.text = commu.get_validate_timeout()
        text_validate_count.text = commu.get_validate_count()
        text_thread_count.text = commu.get_thread_count()
        text_dns_source.text = commu.get_dns_source()
    }

    Component.onCompleted: {
        config_root.updatefrom()
    }
}

