# Mapeamento persistente — Joy-Cons, Sunshine e Lichii (diagnóstico v16)

## Por que os controles invertiam?

O Moonlight v13 ordenava apenas os Joy-Cons presentes. Se o lado direito conectava sozinho, ele recebia a porta virtual 0; quando o esquerdo conectava depois, o esquerdo passava para 0 e o direito para 1. Se a desconexão removesse um controle virtual, os índices poderiam mudar outra vez.

O SDL do Lichii identifica dispositivos por GUID e `port`, e salva esses identificadores em cada botão do perfil. Dois dispositivos virtuais DS4 com o mesmo GUID precisam ter números de porta consistentes em todas as sessões.

## Correção no cliente

No modo **Joy-Cons separados**, o Borealis reserva esquerda/direita por grupo de dois slots HID. Se o direito conectar primeiro, a porta esquerda fica temporariamente neutra, sem deslocar o direito da sua porta lógica. Caso um grupo de slots HID anterior esteja ausente, suas portas são reservadas enquanto houver Joy-Cons de um grupo posterior.

Durante a mesma transmissão, o Moonlight mantém ativos os controles virtuais já anunciados, mesmo após uma desconexão temporária, e envia estado neutro. Isso impede que um dispositivo virtual desapareça e seja criado em outra posição quando o Joy-Con reconectar.

**Limite:** não existe identificação estável por número de série de cada Joy-Con. Se o próprio Switch reatribuir um Joy-Con de um grupo HID físico para outro grupo entre sessões, ou se o Sunshine mudar o GUID ou a numeração dos dispositivos, o mapeamento ainda poderá precisar de correção. Controles mistos (Pro Controller e Joy-Cons) mantêm a enumeração anterior em vez de perder o controle Pro.

## Configuração única recomendada no PC

1. No Moonlight, use **Joy-Cons separados**, não o modo pareado, para manter sensores independentes e mapear os dois lados no Lichii.
2. No Sunshine, em **Configuration → Input**, configure **Gamepad = DualShock 4 (ds4)** e mantenha a mesma opção em todas as sessões. Não deixe alternar entre Xbox e DS4; controles Xbox não fornecem giroscópio ao Lichii.
3. Se o Sunshine estiver usando **Virtual HID Broker/libvirtualhid**, desative `virtualhid_randomize_mac`, quando essa opção estiver disponível. Na configuração por arquivo, o valor correspondente é `virtualhid_randomize_mac = disabled`. No backend ViGEmBus, essa opção não se aplica.
4. Reinicie o Sunshine depois de mudar a configuração. Antes de abrir o Lichii, inicie a transmissão e confira pelo Windows (`joy.cpl`) que os controles virtuais estão conectados. As portas 0 e 1 devem corresponder, respectivamente, aos lados esquerdo e direito do primeiro par.
5. No Lichii, vá a **Emulation → Configure → Controls**. Escolha o tipo de controle emulado **Dual Joycons** para o jogador. Os dois DS4 virtuais não são detectados automaticamente como Joy-Cons físicos: configure os botões do lado esquerdo usando a primeira porta DS4, e os botões do lado direito usando a segunda. Faça isso apenas uma vez para gerar o perfil.
6. Clique em **New** e salve o perfil, por exemplo `Moonlight-Dual-P1`. Após qualquer alteração, clique em **Save** para atualizar o arquivo. Repita para os outros jogadores se necessário.
7. Para aplicar o perfil ao jogo: clique com o botão direito sobre o jogo, abra **Properties → Input Profiles** e associe `Moonlight-Dual-P1` ao jogador correspondente. Salve a configuração e confirme os botões antes de iniciar uma partida.
8. Confirme que o arquivo `config/input/Moonlight-Dual-P1.ini` realmente foi criado no **diretório de usuário do Lichii**. O caminho absoluto depende de a instalação ser portátil ou usar dados do usuário. Faça uma cópia de segurança dos perfis.

## Verificação de persistência

- Feche e reabra o Lichii **sem encerrar a transmissão**: o perfil deve funcionar sem remapeamento. Se falhar, verifique a existência do arquivo INI e a associação por jogo.
- Encerre a transmissão e inicie outra, **mantendo o Sunshine com a mesma configuração**. Verifique novamente os controles no `joy.cpl` e no Lichii.
- Inverta a ordem física de conexão dos Joy-Cons e confirme se cada lado continua controlando os mesmos botões.
- Desconecte apenas o esquerdo por alguns segundos, reconecte-o e confira se o direito continua na mesma porta.
- Se o perfil aparecer como salvo, mas apontar para outro controle, capture os dados do `config/input/*.ini` e o `log.txt` da sessão do Lichii, especialmente as entradas `SDL joystick ... connected on port`.

## Limitação importante

O SDL do Lichii só oferece a autodetecção de um dispositivo **Nintendo Dual Joy-Con** quando recebe duas identidades físicas Joy-Con reconhecidas como esquerda/direita. O Sunshine enviando dois DS4 não produz automaticamente esse tipo de dispositivo. Por isso, a solução apresentada estabiliza as portas e reaproveita um perfil salvo, mas não transforma dois DS4 em um par Nintendo com autodetecção universal.

A revisão pública consultada de `zortos293/Lichii` na linha `nx2` não permite afirmar que todos os detalhes sejam idênticos ao executável local `build-358`. Não altere arquivos INI manualmente sem cópia de segurança.
