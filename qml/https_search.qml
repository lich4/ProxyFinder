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

ColumnLayout {
    id: root_https_search
    anchors.fill: parent
    anchors.margins: dp(32)
    property var lastcheck: -1
    ProgressBar {
        id: progress_https_search
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        Layout.fillWidth: true
        color: theme.accentColor
        indeterminate: true
        visible: false
    }
    ListView {
        id: proxy_list
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: progress_https_search.bottom
        anchors.bottom: parent.bottom
        model: proxy_list_model
        delegate: ListItem.Standard {
            text: txt
            secondaryItem: Switch {
                id: enablingSwitch
                anchors.verticalCenter: parent.verticalCenter
                checked: ischecked
                onClicked: {
                    checked = !checked
                }
            }
            onClicked: {
                enablingSwitch.checked = !enablingSwitch.checked
                if (enablingSwitch.checked) {
                    if (root_https_search.lastcheck != -1) {
                        proxy_list_model.setProperty(root_https_search.lastcheck, "ischecked", false)
                    }
                    root_https_search.lastcheck = index
                    commu.set_https_search_lastcheck(index) // set lastcheck and proxy
                    commu.do_apply_proxy(index)
                } else {
                    root_https_search.lastcheck = -1
                    commu.do_cancel_proxy()
                }
            }
        }
    }
    ListModel {
        id: proxy_list_model
        Component.onCompleted: {
            commu.register_https_search(root_https_search)
            restore()
        }
        function create(text) {
            return {
                txt: text,
                ischecked: false
            };
        }
    }
    function start() {
        progress_https_search.visible = true
        proxy_list_model.clear()
    }
    function clear() {
        proxy_list_model.clear()
    }
    function restore() {
        commu.get_https_search_lock()
        progress_https_search.visible = commu.get_https_search_progress_visible()
        root_https_search.lastcheck = commu.get_https_search_lastcheck()
        var itemsize = commu.get_https_search_listview_size()
        for (var i = 0; i < itemsize; i++) {
            update(-1, commu.get_https_search_listview_item(i))
        }
        if (root_https_search.lastcheck != -1) {
            proxy_list_model.setProperty(root_https_search.lastcheck, "ischecked", true)
        }
        commu.get_https_search_unlock()
    }
    function stop() {
        progress_https_search.visible = false
    }
    function update(lineno, text) {
        if (lineno === -1) { // only add
            proxy_list_model.append(proxy_list_model.create(text))
        } else { // replace
            proxy_list_model.setProperty(lineno, "txt", text)
        }
    }
}
