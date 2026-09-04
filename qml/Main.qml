// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Alessandro Henriques Teixeira — Studio Arn

import QtQuick
import QtCore
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
    property real lastPointerX: -10000
    property real lastPointerY: -10000
    property url downloadFolder: StandardPaths.writableLocation(StandardPaths.DownloadLocation)

    ListModel { id: playlistModel }

    Component.onCompleted: {
        const pendingMedia = appController.takePendingMedia()
        for (let i = 0; i < pendingMedia.length; ++i)
            addMedia(pendingMedia[i], i === 0)
    }

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

    function openOnlineUrl() {
        let value = onlineUrlField.text.trim()
        if (!/^https?:\/\//i.test(value)) {
            onlineUrlError.text = "Cole um endereço iniciado por https://"
            return
        }
        onlineUrlError.text = ""
        addMedia(value, true)
        onlineUrlDialog.close()
        onlineUrlField.clear()
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
        player.setMusicVisualizer(musicVisualizerAction.checked, visibility === Window.FullScreen ? 96 : width < 900 ? 48 : 64)
        revealControls()
    }

    function revealControls() {
        controlsVisible = true
        controlsTimer.restart()
    }

    function pointerMoved(x, y) {
        if (Math.abs(x - lastPointerX) < 2 && Math.abs(y - lastPointerY) < 2)
            return
        lastPointerX = x
        lastPointerY = y
        revealControls()
    }

    function applyBasicVideo() {
        player.setVideoBasic(brightnessSlider.value, contrastSlider.value,
                             saturationSlider.value, gammaSlider.value,
                             hueSlider.value, sharpnessSlider.value,
                             debandCheck.checked, grainSlider.value)
    }
    function applyCropVideo() { player.setVideoCrop(cropLeft.value, cropRight.value, cropTop.value, cropBottom.value) }
    function applyGeometryVideo() {
        player.setVideoGeometry(rotationBox.currentIndex * 90, mirrorHorizontal.checked,
                                mirrorVertical.checked, zoomSlider.value)
    }
    function applyColorVideo() {
        player.setVideoColor(grayscaleCheck.checked, negativeCheck.checked,
                             sepiaSlider.value, posterizeSpin.value)
    }
    function applyOtherVideo() {
        player.setVideoOther(deinterlaceCheck.checked, denoiseSlider.value,
                             removeBandingCheck.checked)
    }
    function resetVideoUi() {
        brightnessSlider.value = 0; contrastSlider.value = 0; saturationSlider.value = 0
        gammaSlider.value = 0; hueSlider.value = 0; sharpnessSlider.value = 0
        debandCheck.checked = false; grainSlider.value = 0
        cropLeft.value = 0; cropRight.value = 0; cropTop.value = 0; cropBottom.value = 0
        rotationBox.currentIndex = 0; mirrorHorizontal.checked = false
        mirrorVertical.checked = false; zoomSlider.value = 1
        grayscaleCheck.checked = false; negativeCheck.checked = false
        sepiaSlider.value = 0; posterizeSpin.value = 0
        deinterlaceCheck.checked = false; denoiseSlider.value = 0
        removeBandingCheck.checked = false
        player.resetVideoAdjustments()
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
        nameFilters: [
            "Todos os formatos de mídia (*.3gp *.aac *.ac3 *.aif *.aiff *.alac *.ape *.asf *.avi *.divx *.dts *.dv *.flac *.flv *.m2ts *.m4a *.m4v *.mka *.mkv *.mov *.mp2 *.mp3 *.mp4 *.mpeg *.mpg *.mts *.mxf *.oga *.ogg *.ogm *.ogv *.opus *.rm *.rmvb *.ts *.vob *.wav *.webm *.wma *.wmv)",
            "Vídeos (*.3gp *.asf *.avi *.divx *.dv *.flv *.m2ts *.m4v *.mkv *.mov *.mp4 *.mpeg *.mpg *.mts *.mxf *.ogv *.rm *.rmvb *.ts *.vob *.webm *.wmv)",
            "Áudios (*.aac *.ac3 *.aif *.aiff *.alac *.ape *.dts *.flac *.m4a *.mka *.mp2 *.mp3 *.oga *.ogg *.opus *.wav *.wma)",
            "Todos os arquivos (*)"
        ]
        onAccepted: {
            for (let i = 0; i < selectedFiles.length; ++i)
                addMedia(selectedFiles[i], i === 0)
        }
    }

    Dialog {
        id: onlineUrlDialog
        title: "Abrir mídia on-line"
        modal: true
        anchors.centerIn: parent
        width: Math.min(560, window.width - 40)
        standardButtons: Dialog.NoButton
        onOpened: {
            onlineUrlError.text = ""
            onlineUrlField.forceActiveFocus()
        }

        contentItem: ColumnLayout {
            spacing: 12
            Label {
                Layout.fillWidth: true
                text: "Cole um link do YouTube, Rumble ou outra fonte compatível."
                color: window.textPrimary
                wrapMode: Text.WordWrap
            }
            TextField {
                id: onlineUrlField
                Layout.fillWidth: true
                placeholderText: "https://www.youtube.com/watch?v=..."
                selectByMouse: true
                onAccepted: window.openOnlineUrl()
            }
            Label {
                id: onlineUrlError
                Layout.fillWidth: true
                color: "#e57373"
                wrapMode: Text.WordWrap
            }
            Label {
                Layout.fillWidth: true
                text: "A reprodução depende do yt-dlp. Esta função não baixa nem salva o vídeo."
                color: window.textMuted
                font.pixelSize: 12
                wrapMode: Text.WordWrap
            }
            RowLayout {
                Layout.alignment: Qt.AlignRight
                Button { text: "Cancelar"; onClicked: onlineUrlDialog.close() }
                Button { text: "Reproduzir"; onClicked: window.openOnlineUrl() }
            }
        }
    }

    FileDialog {
        id: subtitleDialog
        title: "Carregar legenda"
        nameFilters: ["Legendas (*.srt *.ass *.ssa *.vtt *.sub)", "Todos os arquivos (*)"]
        onAccepted: player.addSubtitle(selectedFile)
    }

    FolderDialog {
        id: downloadFolderDialog
        title: "Escolher pasta para salvar"
        currentFolder: window.downloadFolder
        onAccepted: window.downloadFolder = selectedFolder
    }

    menuBar: MenuBar {
        background: Rectangle { color: window.panel }
        Menu {
            title: "Arquivo"
            Action { text: "Abrir mídia…"; shortcut: StandardKey.Open; onTriggered: openDialog.open() }
            Action { text: "Abrir URL…"; shortcut: "Ctrl+U"; onTriggered: onlineUrlDialog.open() }
            Action { text: "Baixar mídia autorizada…"; shortcut: "Ctrl+D"; onTriggered: downloadDialog.open() }
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
        Menu {
            title: "Vídeo"
            Action { text: "Ajustes de imagem…"; onTriggered: videoAdjustDialog.open() }
            Action { text: "Restaurar ajustes de vídeo"; onTriggered: resetVideoUi() }
        }
        Menu {
            title: "Áudio"
            Action { text: "Melhorias de áudio…"; onTriggered: audioAdjustDialog.open() }
            Action {
                id: musicVisualizerAction
                text: "Visualizador musical"
                checkable: true
                checked: true
                onToggled: player.setMusicVisualizer(checked, window.visibility === Window.FullScreen ? 96 : window.width < 900 ? 48 : 64)
            }
            Action { text: "Restaurar ajustes de áudio"; onTriggered: { player.resetAudioAdjustments(); bassSlider.value = 0; trebleSlider.value = 0; normalizeCheck.checked = false; delaySlider.value = 0 } }
        }
        Menu { title: "Ajuda"; Action { text: "Sobre o ArnPlay"; onTriggered: aboutDialog.open() } }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            property real chromeHeight: window.controlsVisible || player.duration <= 0 ? 58 : 0
            Layout.fillWidth: true
            Layout.preferredHeight: chromeHeight
            opacity: window.controlsVisible || player.duration <= 0 ? 1 : 0
            clip: true
            color: window.panel
            Behavior on opacity { NumberAnimation { duration: 220; easing.type: Easing.OutCubic } }
            Behavior on chromeHeight { NumberAnimation { duration: 220; easing.type: Easing.OutCubic } }
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

                Rectangle {
                    anchors.centerIn: parent
                    width: loadingContent.implicitWidth + 34
                    height: loadingContent.implicitHeight + 24
                    radius: 10
                    color: "#dd151515"
                    border.color: window.gold
                    visible: player.loading
                    z: 30

                    Row {
                        id: loadingContent
                        anchors.centerIn: parent
                        spacing: 12
                        BusyIndicator { running: player.loading; width: 30; height: 30 }
                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: "Carregando mídia…"
                            color: window.textPrimary
                            font.pixelSize: 14
                        }
                    }
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
                TapHandler {
                    acceptedButtons: Qt.RightButton
                    onTapped: playerContextMenu.popup(point.position.x, point.position.y)
                }
                Menu {
                    id: playerContextMenu
                    MenuItem { text: "Abrir mídia…"; onTriggered: openDialog.open() }
                    MenuItem { text: "Abrir URL…"; onTriggered: onlineUrlDialog.open() }
                    MenuItem { text: "Baixar mídia autorizada…"; onTriggered: downloadDialog.open() }
                    MenuSeparator {}
                    MenuItem { text: player.playing ? "Pausar" : "Reproduzir"; onTriggered: player.togglePause() }
                    MenuItem { text: "Carregar legenda…"; onTriggered: subtitleDialog.open() }
                    MenuItem { text: playlistVisible ? "Ocultar playlist" : "Mostrar playlist"; onTriggered: playlistVisible = !playlistVisible }
                    MenuSeparator {}
                    MenuItem { text: "Ajustes de vídeo…"; onTriggered: videoAdjustDialog.open() }
                    MenuItem { text: "Melhorias de áudio…"; onTriggered: audioAdjustDialog.open() }
                    MenuItem { text: "Tela cheia"; onTriggered: toggleFullscreen() }
                }
                HoverHandler {
                    id: playbackHover
                    onPointChanged: window.pointerMoved(point.scenePosition.x, point.scenePosition.y)
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
                    visible: player.duration <= 0
                    Text { anchors.horizontalCenter: parent.horizontalCenter; text: "ARNPLAY"; color: window.textPrimary; font.bold: true; font.pixelSize: 42; font.letterSpacing: 7 }
                    Text { anchors.horizontalCenter: parent.horizontalCenter; text: "Arraste uma mídia ou use a barra de controles"; color: window.gold; font.pixelSize: 16 }
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
                    onPreviousRequested: playPrevious()
                    onNextRequested: playNext()
                    onFullscreenRequested: toggleFullscreen()
                    onPlaylistRequested: playlistVisible = !playlistVisible
                    onSubtitleRequested: subtitleDialog.open()
                    onOpenMediaRequested: openDialog.open()
                    onOpenUrlRequested: onlineUrlDialog.open()
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
        target: appController
        function onMediaOpenRequested(url) { window.addMedia(url, true) }
    }

    Connections {
        target: player
        function onErrorOccurred(message) { errorText.text = message; errorPopup.open() }
        function onMediaTitleChanged() {
            if (currentMediaIndex >= 0 && currentMediaIndex < playlistModel.count && player.mediaTitle.length)
                playlistModel.setProperty(currentMediaIndex, "mediaName", player.mediaTitle)
        }
    }
    Connections {
        target: mediaDownloader
        function onErrorOccurred(message) { errorText.text = message; errorPopup.open() }
        function onFinished(message) { infoText.text = message; infoPopup.open() }
    }

    Dialog {
        id: downloadDialog
        title: "Baixar mídia autorizada"
        width: Math.min(600, window.width - 40)
        modal: true
        standardButtons: Dialog.NoButton
        contentItem: ColumnLayout {
            spacing: 12
            Label { text: "Link da mídia"; color: window.textPrimary }
            TextField { id: downloadUrl; Layout.fillWidth: true; placeholderText: "https://..."; selectByMouse: true }
            RowLayout {
                Layout.fillWidth: true
                Label { text: "Formato:"; color: window.textPrimary }
                ComboBox { id: downloadFormat; model: ["Vídeo MP4", "Somente áudio MP3"] }
                Item { Layout.fillWidth: true }
                Button { text: "Escolher pasta…"; onClicked: downloadFolderDialog.open() }
            }
            Label { Layout.fillWidth: true; text: "Salvar em: " + window.downloadFolder; color: window.textMuted; elide: Text.ElideMiddle }
            CheckBox {
                id: authorizationCheck
                Layout.fillWidth: true
                text: "Confirmo que tenho autorização para baixar este conteúdo"
            }
            ProgressBar { Layout.fillWidth: true; from: 0; to: 100; value: mediaDownloader.progress; visible: mediaDownloader.running }
            Label { Layout.fillWidth: true; text: mediaDownloader.status; color: window.gold; wrapMode: Text.WordWrap }
            RowLayout {
                Layout.alignment: Qt.AlignRight
                Button { text: "Fechar"; enabled: !mediaDownloader.running; onClicked: downloadDialog.close() }
                Button { text: "Cancelar download"; visible: mediaDownloader.running; onClicked: mediaDownloader.cancel() }
                Button {
                    text: "Baixar"
                    enabled: authorizationCheck.checked && !mediaDownloader.running && downloadUrl.text.trim().length > 0
                    onClicked: mediaDownloader.download(downloadUrl.text, window.downloadFolder, downloadFormat.currentIndex === 1)
                }
            }
        }
    }

    Dialog {
        id: videoAdjustDialog
        title: "Efeitos de vídeo"
        modal: false
        width: Math.min(720, window.width - 32)
        height: Math.min(520, window.height - 80)
        anchors.centerIn: parent
        standardButtons: Dialog.NoButton
        background: Rectangle { radius: 12; color: "#dc171717"; border.color: "#806f5628"; border.width: 1 }
        contentItem: ColumnLayout {
            spacing: 10
            TabBar {
                id: videoTabs; Layout.fillWidth: true
                TabButton { text: "Básico" }
                TabButton { text: "Recortar" }
                TabButton { text: "Geometria" }
                TabButton { text: "Cor" }
                TabButton { text: "Outros" }
            }
            StackLayout {
                currentIndex: videoTabs.currentIndex; Layout.fillWidth: true; Layout.fillHeight: true
                ScrollView {
                    contentWidth: availableWidth
                    ColumnLayout {
                        width: parent.width; spacing: 6
                        Label { text: "Ajuste de imagem"; color: window.gold; font.bold: true }
                        Label { text: "Brilho  " + Math.round(brightnessSlider.value); color: window.textPrimary }
                        Slider { id: brightnessSlider; Layout.fillWidth: true; from: -100; to: 100; onMoved: window.applyBasicVideo() }
                        Label { text: "Contraste  " + Math.round(contrastSlider.value); color: window.textPrimary }
                        Slider { id: contrastSlider; Layout.fillWidth: true; from: -100; to: 100; onMoved: window.applyBasicVideo() }
                        Label { text: "Saturação  " + Math.round(saturationSlider.value); color: window.textPrimary }
                        Slider { id: saturationSlider; Layout.fillWidth: true; from: -100; to: 100; onMoved: window.applyBasicVideo() }
                        Label { text: "Gama  " + Math.round(gammaSlider.value); color: window.textPrimary }
                        Slider { id: gammaSlider; Layout.fillWidth: true; from: -100; to: 100; onMoved: window.applyBasicVideo() }
                        Label { text: "Tonalidade  " + Math.round(hueSlider.value); color: window.textPrimary }
                        Slider { id: hueSlider; Layout.fillWidth: true; from: -100; to: 100; onMoved: window.applyBasicVideo() }
                        Label { text: "Nitidez  " + sharpnessSlider.value.toFixed(1); color: window.textPrimary }
                        Slider { id: sharpnessSlider; Layout.fillWidth: true; from: 0; to: 2; stepSize: 0.1; onMoved: window.applyBasicVideo() }
                        CheckBox { id: debandCheck; text: "Remoção de bandas"; onToggled: window.applyBasicVideo() }
                        Label { text: "Granulação do filme  " + Math.round(grainSlider.value); color: window.textPrimary }
                        Slider { id: grainSlider; Layout.fillWidth: true; from: 0; to: 30; onMoved: window.applyBasicVideo() }
                    }
                }
                ColumnLayout {
                    Label { text: "Recorte manual em pixels"; color: window.gold; font.bold: true }
                    GridLayout { columns: 2
                        Label { text: "Esquerda"; color: window.textPrimary }
                        SpinBox { id: cropLeft; from: 0; to: 4000; editable: true; onValueModified: window.applyCropVideo() }
                        Label { text: "Direita"; color: window.textPrimary }
                        SpinBox { id: cropRight; from: 0; to: 4000; editable: true; onValueModified: window.applyCropVideo() }
                        Label { text: "Superior"; color: window.textPrimary }
                        SpinBox { id: cropTop; from: 0; to: 4000; editable: true; onValueModified: window.applyCropVideo() }
                        Label { text: "Inferior"; color: window.textPrimary }
                        SpinBox { id: cropBottom; from: 0; to: 4000; editable: true; onValueModified: window.applyCropVideo() }
                    }
                    Label { text: "Aplicação imediata, sem alterar o arquivo original."; color: window.textMuted }
                    Item { Layout.fillHeight: true }
                }
                ColumnLayout {
                    Label { text: "Transformação"; color: window.gold; font.bold: true }
                    RowLayout {
                        Label { text: "Rotação"; color: window.textPrimary }
                        ComboBox { id: rotationBox; model: ["0°", "90°", "180°", "270°"]; onActivated: window.applyGeometryVideo() }
                    }
                    CheckBox { id: mirrorHorizontal; text: "Espelhar horizontalmente"; onToggled: window.applyGeometryVideo() }
                    CheckBox { id: mirrorVertical; text: "Espelhar verticalmente"; onToggled: window.applyGeometryVideo() }
                    Label { text: "Zoom  " + zoomSlider.value.toFixed(2) + "×"; color: window.textPrimary }
                    Slider { id: zoomSlider; Layout.fillWidth: true; from: 0.5; to: 3; stepSize: 0.05; value: 1; onMoved: window.applyGeometryVideo() }
                    Item { Layout.fillHeight: true }
                }
                ColumnLayout {
                    Label { text: "Tratamento de cor"; color: window.gold; font.bold: true }
                    CheckBox { id: grayscaleCheck; text: "Preto e branco"; onToggled: window.applyColorVideo() }
                    CheckBox { id: negativeCheck; text: "Negativo"; onToggled: window.applyColorVideo() }
                    Label { text: "Sépia  " + Math.round(sepiaSlider.value * 100) + "%"; color: window.textPrimary }
                    Slider { id: sepiaSlider; Layout.fillWidth: true; from: 0; to: 1; stepSize: 0.01; onMoved: window.applyColorVideo() }
                    RowLayout {
                        Label { text: "Posterização (níveis)"; color: window.textPrimary }
                        SpinBox { id: posterizeSpin; from: 0; to: 64; editable: true; onValueModified: window.applyColorVideo() }
                    }
                    Item { Layout.fillHeight: true }
                }
                ColumnLayout {
                    Label { text: "Processamento"; color: window.gold; font.bold: true }
                    CheckBox { id: deinterlaceCheck; text: "Desentrelaçamento"; onToggled: window.applyOtherVideo() }
                    CheckBox { id: removeBandingCheck; text: "Remoção de bandas reforçada"; onToggled: window.applyOtherVideo() }
                    Label { text: "Redução de ruído  " + denoiseSlider.value.toFixed(1); color: window.textPrimary }
                    Slider { id: denoiseSlider; Layout.fillWidth: true; from: 0; to: 10; stepSize: 0.1; onMoved: window.applyOtherVideo() }
                    Item { Layout.fillHeight: true }
                }
            }
            RowLayout {
                Item { Layout.fillWidth: true }
                Button { text: "Restaurar padrões"; onClicked: window.resetVideoUi() }
                Button { text: "Fechar"; onClicked: videoAdjustDialog.close() }
            }
        }
    }

    Dialog {
        id: audioAdjustDialog
        title: "Melhorias de áudio"
        width: Math.min(520, window.width - 40)
        standardButtons: Dialog.Close
        contentItem: ColumnLayout {
            Label { text: "Graves: " + Math.round(bassSlider.value) + " dB"; color: window.textPrimary }
            Slider { id: bassSlider; Layout.fillWidth: true; from: -12; to: 12; value: 0 }
            Label { text: "Agudos: " + Math.round(trebleSlider.value) + " dB"; color: window.textPrimary }
            Slider { id: trebleSlider; Layout.fillWidth: true; from: -12; to: 12; value: 0 }
            CheckBox { id: normalizeCheck; text: "Normalizar volume" }
            Label { text: "Sincronização: " + delaySlider.value.toFixed(2) + " s"; color: window.textPrimary }
            Slider { id: delaySlider; Layout.fillWidth: true; from: -5; to: 5; stepSize: 0.05; value: 0 }
            Button {
                Layout.alignment: Qt.AlignRight
                text: "Aplicar"
                onClicked: player.setAudioAdjustments(Math.round(bassSlider.value), Math.round(trebleSlider.value), normalizeCheck.checked, delaySlider.value)
            }
        }
    }

    Dialog {
        id: aboutDialog
        title: "Sobre o ArnPlay"
        standardButtons: Dialog.Ok
        Label {
            width: 420
            text: "ArnPlay 0.7.4 — Studio Arn\n\nPlayer multimídia para macOS Intel, criado e desenvolvido por Alessandro Henriques Teixeira.\n\nQt 6/QML + libmpv\nLicença GNU GPLv3 ou posterior."
            color: window.textPrimary
            wrapMode: Text.WordWrap
        }
    }
    Dialog { id: infoPopup; title: "ArnPlay"; standardButtons: Dialog.Ok; Label { id: infoText; color: window.textPrimary } }
    Dialog {
        id: errorPopup
        title: "ArnPlay"
        width: Math.min(540, window.width - 40)
        standardButtons: Dialog.Ok
        Label { id: errorText; width: parent.width; color: window.textPrimary; wrapMode: Text.WordWrap }
    }
}
