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
    id: root_dns_resolve
    anchors.fill: parent
    anchors.margins: dp(32)
    ProgressBar {
        id: progress_dns_resolve
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        Layout.fillWidth: true
        color: theme.accentColor
        indeterminate: true
        visible: false
    }
    ListView {
        id: dns_list
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: progress_dns_resolve.bottom
        anchors.bottom: parent.bottom
        model: dns_list_model
        delegate: ListItem.Standard {
            text: txt
        }
    }
    ListModel {
        id: dns_list_model
        Component.onCompleted: {
            commu.register_dns_resolve(root_dns_resolve)
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
        progress_dns_resolve.visible = true
        dns_list_model.clear()
    }
    function restore() {
        commu.get_dns_resolve_lock()
        progress_dns_resolve.visible = commu.get_dns_resolve_progress_visible()
        var itemsize = commu.get_dns_resolve_listview_size()
        for (var i = 0; i < itemsize; i++) {
            update(-1, commu.get_dns_resolve_listview_item(i))
        }
        commu.get_dns_resolve_unlock()
    }
    function stop() {
        progress_dns_resolve.visible = false
    }
    function update(lineno, text) {
        if (lineno === -1) { // only add
            dns_list_model.append(dns_list_model.create(text))
        } else { // replace
            dns_list_model.setProperty(lineno, "txt", text)
        }
    }
}
