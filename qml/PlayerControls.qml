// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Alessandro Henriques Teixeira — Studio Arn

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    required property var player
    required property color gold
    required property color panel
    required property color textPrimary
    required property color textMuted

    signal previousRequested()
    signal nextRequested()
    signal fullscreenRequested()
    signal playlistRequested()
    signal subtitleRequested()

    implicitHeight: 96
    radius: 13
    color: "#e6151515"
    border.color: "#55d6ad55"
    border.width: 1

    function clock(seconds) {
        if (!isFinite(seconds) || seconds < 0) return "00:00"
        let s = Math.floor(seconds % 60).toString().padStart(2, "0")
        let m = Math.floor(seconds / 60 % 60).toString().padStart(2, "0")
        let h = Math.floor(seconds / 3600)
        return h > 0 ? h + ":" + m + ":" + s : m + ":" + s
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.leftMargin: 14
        anchors.rightMargin: 14
        anchors.topMargin: 8
        anchors.bottomMargin: 8
        spacing: 3

        Slider {
            Layout.fillWidth: true
            from: 0
            to: Math.max(1, player.duration)
            value: player.position
            onMoved: player.position = value
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 6
            Label { text: clock(player.position) + " / " + clock(player.duration); color: textMuted; Layout.preferredWidth: 112 }
            Item { Layout.fillWidth: true }
            RoundButton { text: "◀│"; onClicked: previousRequested(); ToolTip.visible: hovered; ToolTip.text: "Anterior" }
            RoundButton { text: "−10"; onClicked: player.seek(-10); ToolTip.visible: hovered; ToolTip.text: "Retroceder 10 segundos" }
            RoundButton { text: player.playing ? "Ⅱ" : "▶"; highlighted: true; onClicked: player.togglePause(); ToolTip.visible: hovered; ToolTip.text: player.playing ? "Pausar" : "Reproduzir" }
            RoundButton { text: "+10"; onClicked: player.seek(10); ToolTip.visible: hovered; ToolTip.text: "Avançar 10 segundos" }
            RoundButton { text: "│▶"; onClicked: nextRequested(); ToolTip.visible: hovered; ToolTip.text: "Próximo" }
            Item { Layout.fillWidth: true }
            ComboBox {
                Layout.preferredWidth: 76
                model: ["0.5×", "0.75×", "1×", "1.25×", "1.5×", "2×"]
                currentIndex: 2
                onActivated: {
                    const speeds = [0.5, 0.75, 1.0, 1.25, 1.5, 2.0]
                    player.setPlaybackSpeed(speeds[currentIndex])
                }
                ToolTip.visible: hovered
                ToolTip.text: "Velocidade"
            }
            Label { text: "🔊"; color: textPrimary }
            Slider { Layout.preferredWidth: 105; from: 0; to: 200; value: player.volume; onMoved: player.volume = value }
            Label {
                text: Math.round(player.volume) + "%"
                color: player.volume > 100 ? gold : textMuted
                Layout.preferredWidth: 42
                font.bold: player.volume > 100
            }
            RoundButton { text: "CC"; onClicked: subtitleRequested(); ToolTip.visible: hovered; ToolTip.text: "Carregar legenda" }
            RoundButton { text: "☷"; onClicked: playlistRequested(); ToolTip.visible: hovered; ToolTip.text: "Playlist" }
            RoundButton { text: "⛶"; onClicked: fullscreenRequested(); ToolTip.visible: hovered; ToolTip.text: "Tela cheia" }
        }
    }
}
