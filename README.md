# ArnPlay — Studio Arn (0.7.4)

Player multimídia leve e moderno para macOS Intel, com interface Qt 6/QML e reprodução por libmpv. A identidade visual segue o padrão preto, branco e dourado do ArnFrame.

Criado e desenvolvido por **Alessandro Henriques Teixeira — Studio Arn**.  
Copyright © 2026. Distribuído sob a licença **GNU GPLv3 ou posterior**.

## Download

[Baixar ArnPlay 0.7.4 para macOS Intel](https://github.com/AleSulista/ArnPlay/raw/main/downloads/ArnPlay-0.7.4-Intel.dmg)

- Arquivo: `ArnPlay-0.7.4-Intel.dmg`
- SHA-256: `6c7419ad2ec6e8eec29aa1a9c8bd2c92f023f8fa122995a0455c6c461a335d5d`
- Requer macOS 12 ou posterior em Mac Intel x86_64.
- Requer Qt 6, libmpv e yt-dlp instalados pelo Homebrew.

## Recursos da versão 0.7.4

- Abre arquivos locais de áudio e vídeo
- Reproduz, pausa e encerra a reprodução
- Avança e retrocede 10 segundos
- Controla posição e volume
- Exibe duração e título da mídia
- Interface redimensionável em QML
- Renderização integrada por OpenGL/libmpv
- Playlist lateral e abertura de vários arquivos
- Arrastar e soltar mídias
- Tela cheia e atalhos de teclado
- Carregamento de legenda externa
- Troca de faixa de áudio e legenda
- Velocidades de 0,5× a 2×
- Barra compacta sobre o vídeo, com ocultação automática real
- Controles essenciais em uma única linha e apenas um botão CC
- Playlist fechada ao iniciar e controlada pelo botão ☷
- Ícone Studio Arn gerado nativamente pelo macOS para o Dock e Finder
- Barra desaparece 1 segundo após o mouse sair ou parar
- Amplificação de volume até 200%, com indicação dourada acima de 100%
- Reprodução de links on-line compatíveis, incluindo YouTube e Rumble, sem download
- Localização automática do `yt-dlp` do Homebrew ao abrir pelo Finder ou por Aplicativos
- Abertura automática da mídia ao usar “Abrir com ArnPlay” ou defini-lo como player padrão
- Ícone circular próprio do ArnPlay, distinto do ArnFrame
- Indicador de carregamento, títulos on-line na playlist e mensagens de falha mais claras
- Download manual de vídeo MP4 ou áudio MP3, somente após confirmação de autorização
- Ajustes de brilho, contraste, saturação, gama e rotação
- Graves, agudos, normalização e sincronização de áudio
- Recursos organizados na barra de menus nativa do macOS
- Tela inicial limpa, abertura de mídia na barra de controles e menu pelo botão direito
- Visualizador musical sobreposto à capa, com 48, 64 ou 96 barras finas e separadas
- Barra superior e controles ocultados juntos após 1 segundo sem atividade
- Seleção ampliada para MTS/M2TS, AVI e demais formatos reconhecidos pelo libmpv/FFmpeg

## Ambiente inicial

- MacBook Pro Intel x86_64
- macOS Sequoia 15.7.3
- Homebrew
- Qt 6
- CMake e Ninja

## Dependências

```bash
brew install qt cmake ninja mpv yt-dlp pkg-config
```

## Compilar

```bash
cd ArnPlay
cmake -S . -B build -G Ninja -DCMAKE_PREFIX_PATH="$(brew --prefix qt)"
cmake --build build
open build/ArnPlay.app
```

## Instalação do DMG para macOS Intel

Esta compilação foi validada em um MacBook Pro Intel com macOS Sequoia 15.7.3. Antes de abrir o aplicativo distribuído no DMG, instale as dependências:

```bash
brew install qt mpv yt-dlp
```

Abra `ArnPlay-0.7.4-Intel.dmg` e arraste o ArnPlay para a pasta Aplicativos. O DMG atual não inclui as bibliotecas do Homebrew. Como o aplicativo ainda não possui assinatura Apple Developer ID nem notarização, o macOS poderá solicitar confirmação na primeira abertura.

Se o CMake não localizar a libmpv, confirme que `pkg-config --modversion mpv` retorna uma versão instalada.

## Projeto e direitos

ArnPlay foi criado, dirigido e desenvolvido por **Alessandro Henriques Teixeira — Studio Arn**. O código original é distribuído sob a GNU GPLv3 ou posterior. Qt, libmpv, FFmpeg e yt-dlp pertencem aos seus respectivos projetos e permanecem sujeitos às próprias licenças.

Consulte [AUTHORS.md](AUTHORS.md), [LICENSE](LICENSE) e [CHANGELOG.md](CHANGELOG.md).

## Próximas etapas

1. Produzir um DMG autossuficiente, assinado e notarizado.
2. Expandir a biblioteca e o gerenciamento de playlists.
3. Continuar aprimorando controles de imagem e áudio.

Downloads de conteúdo on-line serão habilitados somente quando autorizados pela plataforma e pelo titular.
