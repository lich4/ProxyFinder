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

ApplicationWindow {
    id: window
    title: "代理搜索V1.1.0"
    visible: true

    theme {
        primaryColor: "green"
        accentColor: "red"
        tabHighlightColor: "white"
    }

    property var gmaps: {
        "root": [
            "proxy_relative"
        ],
        "proxy_relative": {
            "name": "代理相关",
            "child": [
                "basic_config",
                "dns_resolve",
                "https_search"
            ]
        },
        "basic_config": {
            "name": "基本设置",
            "qml": "basic_config.qml"
        },
        "dns_resolve": {
            "name": "解析域名为IP",
            "qml": "dns_resolve.qml"
        },
        "https_search": {
            "name": "HTTP/HTTPS代理搜索",
            "qml": "https_search.qml"
        }
    }

    property string selectedComponent: "basic_config"

    initialPage: TabbedPage {
        id: page
        title: "超哥的代理搜索工具"
        actionBar.maxActionCount: 3
        actions: [
            Action {
                iconName: "icon_search"
                name: "开始搜索"
                onTriggered: {
                    if (window.selectedComponent === "dns_resolve") {
                        commu.do_dns_lookup()
                    } else if (window.selectedComponent == "https_search") {
                        commu.do_search_proxy()
                    }
                }
                hoverAnimation: true
            },

            Action {
                iconName: "stop"
                name: "停止搜索"
                onTriggered: {
                    commu.cancel_dns_lookup()
                    commu.cancel_search_proxy()
                }
                hoverAnimation: true
            },

            Action {
                iconName: "exit_to_app"
                name: "退出"
                onTriggered: {
                    commu.cancel_dns_lookup()
                    commu.cancel_search_proxy()
                    commu.exit_process()
                }

                hoverAnimation: true
            }
        ]
        backAction: navDrawer.action
        NavigationDrawer {
            id: navDrawer
            enabled: page.width < dp(500)
            onEnabledChanged: smallLoader.active = enabled
            Flickable {
                anchors.fill: parent
                contentHeight: Math.max(content.implicitHeight, height)
                Column {
                    id: content
                    anchors.fill: parent
                    Repeater {
                        model: gmaps.root
                        delegate: Column {
                            width: parent.width
                            ListItem.Subheader {
                                text: gmaps[modelData].name
                            }
                            Repeater {
                                model: gmaps[modelData].child
                                delegate: ListItem.Standard {
                                    text: gmaps[modelData].name
                                    selected: modelData == window.selectedComponent
                                    onClicked: {
                                        window.selectedComponent = modelData
                                        navDrawer.close()
                                        if (window.selectedComponent === "dns_resolve") {
                                            commu.do_dns_lookup()
                                        } else if (window.selectedComponent == "https_search") {
                                            commu.do_search_proxy()
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        Repeater {
            model: !navDrawer.enabled ? gmaps.root : 0
            delegate: Tab {
                title: gmaps[modelData].name
                property string selectedComponent: gmaps[modelData].child[0]
                property var section: gmaps[modelData].child
                sourceComponent: tabDelegate
            }
        }
        Loader {
            id: smallLoader
            anchors.fill: parent
            sourceComponent: tabDelegate
            property var section: []
            visible: active
            active: false
        }
    }
    Component {
        id: tabDelegate
        Item {
            Sidebar {
                id: sidebar
                expanded: !navDrawer.enabled
                Column {
                    width: parent.width
                    Repeater {
                        model: section
                        delegate: ListItem.Standard {
                            text: gmaps[modelData].name
                            selected: modelData == window.selectedComponent
                            onClicked: {
                                window.selectedComponent = modelData
                                if (window.selectedComponent === "dns_resolve") {
                                    commu.do_dns_lookup()
                                } else if (window.selectedComponent == "https_search") {
                                    commu.do_search_proxy()
                                }
                            }
                        }
                    }
                }
            }
            Flickable {
                id: flickable
                anchors {
                    left: sidebar.right
                    right: parent.right
                    top: parent.top
                    bottom: parent.bottom
                }
                clip: true
                contentHeight: Math.max(loader.implicitHeight + 40, height)
                Loader {
                    id: loader
                    anchors.fill: parent
                    asynchronous: true
                    visible: status == Loader.Ready
                    source: gmaps[window.selectedComponent].qml
                }
                ProgressCircle {
                    anchors.centerIn: parent
                    visible: loader.status == Loader.Loading
                }
            }
            Scrollbar {
                flickableItem: flickable
            }
        }
    }
}
