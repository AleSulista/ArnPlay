// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Alessandro Henriques Teixeira — Studio Arn

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import StudioArn.ArnPlay

ApplicationWindow {
    id: window
    width: 1100
    height: 680
    minimumWidth: 760
    minimumHeight: 480
    visible: true
    title: player.mediaTitle.length ? player.mediaTitle + " — ArnPlay" : "ArnPlay — Studio Arn"
    color: "#090909"

    readonly property color gold: "#d6ad55"
    readonly property color panel: "#151515"
    readonly property color textPrimary: "#f5f5f5"
    readonly property color textMuted: "#a7a7a7"
    property int currentMediaIndex: -1
    property bool playlistVisible: false
    property bool controlsVisible: true

    ListModel { id: playlistModel }

    function displayName(url) {
        let value = url.toString()
        value = decodeURIComponent(value.substring(value.lastIndexOf("/") + 1))
        return value.length ? value : "Mídia"
    }

    function addMedia(url, playNow) {
        playlistModel.append({ "mediaUrl": url.toString(), "mediaName": displayName(url) })
        if (playNow || currentMediaIndex < 0)
            playIndex(playlistModel.count - 1)
    }

    function playIndex(index) {
        if (index < 0 || index >= playlistModel.count) return
        currentMediaIndex = index
        player.open(playlistModel.get(index).mediaUrl)
        revealControls()
    }

    function playNext() {
        if (playlistModel.count > 0)
            playIndex((currentMediaIndex + 1) % playlistModel.count)
    }

    function playPrevious() {
        if (playlistModel.count > 0)
            playIndex((currentMediaIndex - 1 + playlistModel.count) % playlistModel.count)
    }

    function toggleFullscreen() {
        visibility = visibility === Window.FullScreen ? Window.Windowed : Window.FullScreen
        revealControls()
    }

    function revealControls() {
        controlsVisible = true
        controlsTimer.restart()
    }

    Timer {
        id: controlsTimer
        interval: 1000
        repeat: false
        onTriggered: {
            if (player.duration > 0)
                controlsVisible = false
        }
    }

    FileDialog {
        id: openDialog
        title: "Abrir mídia"
        fileMode: FileDialog.OpenFiles
        nameFilters: ["Mídia (*.mp4 *.mkv *.mov *.avi *.webm *.mp3 *.flac *.wav *.m4a *.ogg)", "Todos os arquivos (*)"]
        onAccepted: {
            for (let i = 0; i < selectedFiles.length; ++i)
                addMedia(selectedFiles[i], i === 0)
        }
    }

    FileDialog {
        id: subtitleDialog
        title: "Carregar legenda"
        nameFilters: ["Legendas (*.srt *.ass *.ssa *.vtt *.sub)", "Todos os arquivos (*)"]
        onAccepted: player.addSubtitle(selectedFile)
    }

    menuBar: MenuBar {
        background: Rectangle { color: window.panel }
        Menu {
            title: "Arquivo"
            Action { text: "Abrir mídia…"; shortcut: StandardKey.Open; onTriggered: openDialog.open() }
            Action { text: "Carregar legenda…"; shortcut: "Ctrl+Shift+O"; onTriggered: subtitleDialog.open() }
            MenuSeparator {}
            Action { text: "Sair"; shortcut: StandardKey.Quit; onTriggered: Qt.quit() }
        }
        Menu {
            title: "Reprodução"
            Action { text: player.playing ? "Pausar" : "Reproduzir"; shortcut: "Space"; onTriggered: player.togglePause() }
            Action { text: "Faixa de áudio seguinte"; onTriggered: player.cycleAudioTrack() }
            Action { text: "Legenda seguinte"; onTriggered: player.cycleSubtitleTrack() }
            Action { text: "Tela cheia"; shortcut: "F"; onTriggered: toggleFullscreen() }
        }
        Menu { title: "Exibir"; Action { text: "Mostrar/ocultar playlist"; shortcut: "Ctrl+L"; onTriggered: playlistVisible = !playlistVisible } }
        Menu { title: "Ajuda"; Action { text: "Sobre o ArnPlay" } }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 58
            color: window.panel
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 20
                anchors.rightMargin: 20
                Rectangle {
                    Layout.preferredWidth: 36; Layout.preferredHeight: 36; radius: 18
                    color: "transparent"; border.color: window.gold; border.width: 2
                    Text { anchors.centerIn: parent; text: "A▶"; color: window.gold; font.bold: true; font.pixelSize: 12 }
                }
                Column {
                    Text { text: "ARNPLAY"; color: window.textPrimary; font.bold: true; font.pixelSize: 18; font.letterSpacing: 2 }
                    Text { text: "STUDIO ARN"; color: window.gold; font.pixelSize: 9; font.letterSpacing: 2 }
                }
                Item { Layout.fillWidth: true }
                Text { text: player.mediaTitle || "Nenhuma mídia aberta"; color: window.textMuted; elide: Text.ElideMiddle; Layout.maximumWidth: 480 }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                MpvPlayer { id: player; anchors.fill: parent }
                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.NoButton
                    hoverEnabled: true
                    onPositionChanged: window.revealControls()
                }
                DropArea {
                    anchors.fill: parent
                    onDropped: function(drop) {
                        if (!drop.hasUrls) return
                        for (let i = 0; i < drop.urls.length; ++i)
                            addMedia(drop.urls[i], i === 0)
                        drop.acceptProposedAction()
                    }
                }
                Column {
                    anchors.centerIn: parent
                    spacing: 18
                    visible: player.duration <= 0
                    Text { anchors.horizontalCenter: parent.horizontalCenter; text: "ARNPLAY"; color: window.textPrimary; font.bold: true; font.pixelSize: 42; font.letterSpacing: 7 }
                    Text { anchors.horizontalCenter: parent.horizontalCenter; text: "Arraste suas mídias ou abra um arquivo"; color: window.gold; font.pixelSize: 16 }
                    Button { anchors.horizontalCenter: parent.horizontalCenter; text: "Abrir mídia"; onClicked: openDialog.open() }
                }

                PlayerControls {
                    id: overlayControls
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    anchors.leftMargin: 18
                    anchors.rightMargin: 18
                    anchors.bottomMargin: 16
                    player: player
                    gold: window.gold
                    panel: window.panel
                    textPrimary: window.textPrimary
                    textMuted: window.textMuted
                    opacity: window.controlsVisible || player.duration <= 0 ? 1 : 0
                    visible: opacity > 0.01
                    z: 20
                    Behavior on opacity { NumberAnimation { duration: 220; easing.type: Easing.OutCubic } }
                    HoverHandler {
                        onHoveredChanged: {
                            if (hovered)
                                window.revealControls()
                            else
                                controlsTimer.restart()
                        }
                        onPointChanged: window.revealControls()
                    }
                    onPreviousRequested: playPrevious()
                    onNextRequested: playNext()
                    onFullscreenRequested: toggleFullscreen()
                    onPlaylistRequested: playlistVisible = !playlistVisible
                    onSubtitleRequested: subtitleDialog.open()
                }
            }

            Rectangle {
                visible: playlistVisible && window.visibility !== Window.FullScreen
                Layout.preferredWidth: visible ? 300 : 0
                Layout.fillHeight: true
                color: "#101010"
                border.color: "#292929"
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: "PLAYLIST"; color: window.gold; font.bold: true; font.letterSpacing: 2; Layout.fillWidth: true }
                        ToolButton { text: "+"; onClicked: openDialog.open() }
                        ToolButton {
                            text: "×"
                            onClicked: playlistVisible = false
                            ToolTip.visible: hovered
                            ToolTip.text: "Fechar playlist"
                        }
                    }
                    ListView {
                        id: playlistView
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        model: playlistModel
                        clip: true
                        spacing: 4
                        delegate: Rectangle {
                            required property int index
                            required property string mediaName
                            width: playlistView.width
                            height: 48
                            radius: 6
                            color: index === currentMediaIndex ? "#332b19" : mouse.containsMouse ? "#202020" : "transparent"
                            border.color: index === currentMediaIndex ? window.gold : "transparent"
                            Text { anchors.fill: parent; anchors.margins: 10; verticalAlignment: Text.AlignVCenter; text: mediaName; color: window.textPrimary; elide: Text.ElideMiddle }
                            MouseArea { id: mouse; anchors.fill: parent; hoverEnabled: true; onDoubleClicked: playIndex(index) }
                        }
                    }
                    Label { Layout.fillWidth: true; text: playlistModel.count + (playlistModel.count === 1 ? " item" : " itens"); color: window.textMuted; horizontalAlignment: Text.AlignRight }
                }
            }
        }

    }

    Shortcut { sequence: "Left"; onActivated: player.seek(-5) }
    Shortcut { sequence: "Right"; onActivated: player.seek(5) }
    Shortcut { sequence: "M"; onActivated: player.volume = player.volume > 0 ? 0 : 80 }
    Shortcut { sequence: "Escape"; enabled: window.visibility === Window.FullScreen; onActivated: window.showNormal() }

    Connections {
        target: player
        function onErrorOccurred(message) { errorText.text = message; errorPopup.open() }
    }
    Dialog { id: errorPopup; title: "ArnPlay"; standardButtons: Dialog.Ok; Label { id: errorText; color: window.textPrimary } }
}
