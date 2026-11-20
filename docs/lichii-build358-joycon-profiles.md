# Lichii build 358 — controles Joy-Con no Moonlight v13

O Moonlight v13 apresenta Joy-Cons separados na ordem esquerda, direita, esquerda, direita dentro dos grupos HID 0–1 e 2–3. Essa ordem não identifica um Joy-Con individual por número de série; reconexões parciais ainda podem alterar índices.

## Configuração para manter os mapeamentos

1. No Moonlight, selecione Joy-Cons separados para jogos que utilizem sensores individuais. Não alterne esse modo durante a execução do emulador.
2. No Sunshine, fixe um tipo de controle virtual compatível com movimento (por exemplo, DS4). Se estiver usando Virtual HID Broker, desative a aleatorização do MAC virtual. Essas opções dependem do backend instalado.
3. Conecte todos os Joy-Cons antes de iniciar o Lichii e aguarde o Sunshine apresentar os controles virtuais.
4. No Lichii, abra **Emulation → Configure → Controls**. Associe os botões dos Joy-Cons às portas virtuais corretas. Ao usar dois controles virtuais para um par Dual Joycons, mapeie o lado esquerdo a partir da porta à esquerda e o lado direito da porta seguinte.
5. Crie e salve perfis de entrada, identificando cada jogador e par. Em **Properties → Input Profiles** do jogo, associe os perfis aos jogadores e salve.
6. Reinicie a sessão e confirme as portas virtuais apresentadas no Lichii antes de jogar. Se o GUID ou a porta SDL mudar, um perfil salvo pode apontar para o dispositivo errado.

## Diagnóstico quando os lados aparecem invertidos

Compare o controle físico e seu slot no Moonlight, depois compare a porta SDL no Lichii. Um controle DS4 virtual não é reconhecido automaticamente como um Joy-Con físico esquerdo/direito; o tipo emulado **Dual Joycons** não transforma por si só os dois DS4 virtuais em Joy-Cons identificados por lado.

O código público da linha Lichii nx2 usa identificadores SDL baseados em GUID e porta. Não foi verificado que o código da instalação build 358 seja idêntico à revisão pública consultada.

A v13 estabiliza a ordem ao conectar os dois lados de um par, mas não fornece identidade persistente por hardware, reserva de slots de controles ausentes ou reatribuição automática de perfis dentro do Lichii. Essas melhorias dependem da enumeração dos controles virtuais no PC e de alterações adicionais no emulador ou no Sunshine.
