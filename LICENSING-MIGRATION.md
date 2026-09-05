# Migração de licenciamento do ArnPlay

Copyright © 2026 Alessandro Henriques Teixeira — Studio Arn.

## Objetivo

A próxima geração do ArnPlay deverá permitir download, uso, estudo, modificação e redistribuição para finalidades não comerciais, mantendo a exploração comercial sujeita a autorização separada do titular.

A licença planejada para o código original de titularidade do Studio Arn é a **PolyForm Noncommercial License 1.0.0**.

## Situação das versões existentes

O ArnPlay 0.7.4 e as versões já publicadas sob **GNU GPLv3 ou posterior** continuam sob essa licença. A migração futura não revoga nem restringe retroativamente direitos já concedidos sobre essas versões.

## Dependências

O ArnPlay utiliza componentes independentes, incluindo **Qt 6**, **libmpv/mpv**, **FFmpeg** por meio da pilha de reprodução e **yt-dlp** como ferramenta externa opcional. Cada componente continua sujeito à sua própria licença.

Para uma edição do ArnPlay sob PolyForm Noncommercial, o binário distribuído não deverá ser combinado com uma compilação GPL-only de libmpv/mpv ou com uma compilação de FFmpeg cuja configuração force GPL ou contenha componentes incompatíveis com a distribuição pretendida.

A documentação oficial do mpv prevê uma modalidade de compilação sem arquivos GPL-only por meio de `-Dgpl=false`, destinada especialmente ao uso com libmpv, mas a própria documentação alerta que essa opção, isoladamente, não garante a situação de todas as bibliotecas vinculadas. Por isso, a cadeia completa de dependências precisa ser validada.

O Qt utilizado na nova edição deverá limitar-se a módulos disponíveis sob LGPL ou outra licença compatível e deverá ser distribuído em conformidade com as obrigações da LGPL aplicáveis.

## Regra de publicação durante a migração

Até a conclusão da validação da cadeia de dependências:

- o workflow automático da versão 0.7.4 permanece desativado para `push` e só pode ser executado manualmente;
- nenhuma versão nova deve ser apresentada como PolyForm Noncommercial enquanto ainda depender de uma cadeia de reprodução GPL não validada;
- o arquivo `LICENSE` atual permanece representando a versão legada até a criação da primeira versão efetivamente migrada;
- os cabeçalhos SPDX GPL existentes no código legado não devem ser simplesmente substituídos antes da nova versão estar juridicamente e tecnicamente separada.

## Critério para concluir a migração

A primeira versão não comercial somente deve ser publicada depois de:

1. usar uma cadeia de reprodução comprovadamente compatível com a licença planejada;
2. registrar as licenças dos componentes distribuídos em um arquivo de terceiros;
3. substituir, na nova versão, o `LICENSE` e os avisos de licença do código original do Studio Arn;
4. atualizar README, créditos, pacote DMG e tela Sobre;
5. preservar separadamente os avisos de Qt, mpv/libmpv, FFmpeg, yt-dlp e demais terceiros utilizados;
6. testar a compilação e o DMG em macOS Intel antes da publicação.

## Marca

A licença do código não concede automaticamente autorização para usar **ArnPlay**, **Studio Arn**, logotipos, ícones e identidade visual de modo a apresentar produto de terceiros como oficial. Consulte `BRANDING.md`.
