# ArnPlay — Studio Arn (0.7.4)

Player multimídia leve e moderno para macOS Intel, com interface Qt 6/QML e reprodução por libmpv. A identidade visual segue o padrão preto, branco e dourado do ArnFrame.

Criado e desenvolvido por **Alessandro Henriques Teixeira — Studio Arn**.  
Copyright © 2026. Distribuído sob a licença **GNU GPLv3 ou posterior**.

## Download

[Baixar ArnPlay 0.7.4 corrigido para macOS Intel](https://github.com/AleSulista/ArnPlay/releases/download/v0.7.4-corrigido/ArnPlay-0.7.4-Intel.dmg)

O DMG é compilado automaticamente em um executor macOS Intel a partir deste código-fonte.

## Primeira versão

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
- Painel translúcido de efeitos de vídeo inspirado no VLC, com cinco abas
- Ajustes ao vivo de brilho, contraste, saturação, gama, tonalidade e nitidez
- Recorte manual, rotação, espelhamento, zoom, preto e branco, negativo e sépia
- Posterização, desentrelaçamento, redução de ruído, remoção de bandas e granulação
- Graves, agudos, normalização e sincronização de áudio
- Recursos organizados na barra de menus nativa do macOS
- Tela inicial limpa, abertura de mídia na barra de controles e menu pelo botão direito
- Visualizador musical sobreposto à capa, com 48, 64 ou 96 barras finas e separadas
- Barra superior e controles ocultados juntos após 1 segundo sem movimento real do cursor, sem piscar
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

## Criar o aplicativo e o DMG no macOS Intel

Depois de testar a compilação normal, gere o aplicativo independente e o instalador com:

```bash
cd ~/Documents/ArnPlay-0.7.4
chmod +x scripts/package-macos.sh
./scripts/package-macos.sh
```

O instalador será criado em `dist/ArnPlay-0.7.4-Intel.dmg`. Ele inclui o Qt, a libmpv e as dependências de reprodução encontradas no Homebrew. O pacote recebe uma assinatura local ad hoc para teste; distribuição pública sem o aviso do Gatekeeper exige certificado Apple Developer ID e notarização pela Apple.

Se o CMake não localizar a libmpv, confirme que `pkg-config --modversion mpv` retorna uma versão instalada.

## Validação recomendada

1. Confirmar a ocultação dos controles com o cursor parado sobre diferentes áreas do vídeo.
2. Testar cada aba de efeitos com um vídeo local no Mac Intel.
3. Validar o aplicativo independente e o DMG antes da publicação.

Downloads de conteúdo on-line serão habilitados somente quando autorizados pela plataforma e pelo titular.
