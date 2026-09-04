# Histórico de versões

## ArnPlay 0.7.4 corrigido — 4 de setembro de 2026

- Corrigido o ciclo que fazia os controles piscarem com o cursor parado.
- Efeitos de vídeo agora são aplicados ao vivo pelo libmpv.
- Novo painel translúcido com abas Básico, Recortar, Geometria, Cor e Outros.
- Adicionados nitidez, tonalidade, corte, espelhamento, zoom, preto e branco,
  negativo, sépia, posterização, desentrelaçamento, redução de ruído,
  remoção de bandas e granulação.
- Corrigida de 0.7.1 para 0.7.4 a versão mostrada na janela Sobre.

## ArnPlay 0.7.4 — 3 de setembro de 2026

Primeira versão pública completa para macOS Intel x86_64, testada em um MacBook Pro Intel com macOS Sequoia 15.7.3.

### Destaques

- Reprodução local baseada em libmpv, com interface Qt 6/QML.
- Suporte aos formatos reconhecidos pelo libmpv/FFmpeg, incluindo MP4, MKV, MOV, AVI, MTS/M2TS, MP3, FLAC e outros.
- Reprodução de links compatíveis com yt-dlp, incluindo YouTube e Rumble quando disponíveis regionalmente.
- Abertura direta de arquivos pelo Finder e associação como aplicativo padrão.
- Playlist lateral, arrastar e soltar, legendas e seleção de faixas.
- Controles sobrepostos com ocultação após um segundo de inatividade.
- Volume amplificado até 200%.
- Ajustes de brilho, contraste, saturação, gama, rotação, graves, agudos e sincronização.
- Visualizador musical dourado sobreposto à capa.
- Download manual em MP4 ou MP3 somente mediante confirmação de autorização.
- Novo ícone circular do ArnPlay dentro do padrão visual do macOS, distinto do ArnFrame.

### Distribuição atual

- Arquitetura: macOS Intel x86_64.
- Ambiente validado: macOS Sequoia 15.7.3.
- O DMG atual requer Qt 6, libmpv e yt-dlp instalados pelo Homebrew.
- O aplicativo ainda não possui notarização Apple.

### Créditos e licença

Criação, direção do projeto e desenvolvimento por **Alessandro Henriques Teixeira — Studio Arn**. Código distribuído sob GNU GPLv3 ou posterior.
