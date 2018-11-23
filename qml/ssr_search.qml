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
    id: root_ssr_search
    anchors.fill: parent
    anchors.margins: dp(32)
    ProgressBar {
        id: progress_ssr_search
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        Layout.fillWidth: true
        color: theme.accentColor
        indeterminate: true
        visible: false
    }
    ListView {
        id: ssr_list
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: progress_ssr_search.bottom
        anchors.bottom: parent.bottom
        model: ssr_list_model
        delegate: ListItem.Standard {
            text: txt
        }
    }
    ListModel {
        id: ssr_list_model
        Component.onCompleted: {
            commu.register_ssr_search(root_ssr_search)
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
        progress_ssr_search.visible = true
        ssr_list_model.clear()
    }
    function clear() {
        ssr_list_model.clear()
    }
    function restore() {
        commu.get_ssr_search_lock()
        progress_ssr_search.visible = commu.get_ssr_search_progress_visible()
        var itemsize = commu.get_ssr_search_listview_size()
        for (var i = 0; i < itemsize; i++) {
            update(-1, commu.get_ssr_search_listview_item(i))
        }
        commu.get_ssr_search_unlock()
    }
    function stop() {
        progress_ssr_search.visible = false
    }
    function update(lineno, text) {
        if (lineno === -1) { // only add
            ssr_list_model.append(ssr_list_model.create(text))
        } else { // replace
            ssr_list_model.setProperty(lineno, "txt", text)
        }
    }
}
