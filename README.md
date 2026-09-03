# ArnPlay — Studio Arn (0.4.0)

Player multimídia leve e moderno para macOS Intel, com interface Qt 6/QML e reprodução por libmpv. A identidade visual segue o padrão preto, branco e dourado do ArnFrame.

Criado e desenvolvido por **Alessandro Henriques Teixeira — Studio Arn**.  
Copyright © 2026. Distribuído sob a licença **GNU GPLv3 ou posterior**.

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

Se o CMake não localizar a libmpv, confirme que `pkg-config --modversion mpv` retorna uma versão instalada.

## Próximas etapas

1. Validar a reprodução no Mac Intel de desenvolvimento.
2. Criar a logo circular definitiva e o ícone `.icns`.
3. Adicionar playlist, arrastar e soltar, seleção de áudio e legendas.
4. Preparar empacotamento independente em `.app` e `.dmg`.
5. Adicionar login seguro pelo navegador para recursos que realmente exijam autenticação.

Downloads de conteúdo on-line serão habilitados somente quando autorizados pela plataforma e pelo titular.
