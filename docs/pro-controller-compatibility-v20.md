# v20 — Pro Controller e compatibilidade com controles completos

## Requisito

O Nintendo Switch Pro Controller deve conservar o comportamento esperado do
Moonlight-Switch original: leitura de A/B/X/Y, D-pad, dois analógicos, L/R,
ZL/ZR, +/−, cliques dos analógicos, rumble e movimento SixAxis quando disponível,
sem sofrer o mapeamento de botões aplicado a Joy-Cons horizontais.

## Implementação

- O Borealis preserva o caminho `FullKey` de `updateControllerStateInner`
  (mapeamento `SWITCH_BUTTONS_FULL_MAPPING`), `sendRumbleInternal` e a
  aquisição `hidGetSixAxisSensorHandles` com `HidNpadStyleTag_NpadFullKey`.
- Quando **somente** o Pro Controller está conectado ao iniciar a sessão,
  `stableVirtualGamepadCount` anuncia um controle, mesmo com a preferência
  Joy-Con Separados. Nenhuma porta Joy-Con extra é criada nessa condição.
- A configuração **Pro Controller (compatibilidade original)** não reserva
  quatro assentos, mesmo quando o stream inicia sem dispositivos. Também não
  chama as operações libnx de mudança de atribuição Single/Dual dos Joy-Cons.
- Os modos **Pares** e **Individual** continuam disponíveis sem modificar a
  função de botões, eixos ou rumble de um `FullKey`.
- No modo misto, a ordem de tipos que iniciou a sessão é preservada:
  **Pro primeiro:** Pro na primeira porta e quatro assentos Joy-Con após ele;
  **Joy-Cons primeiro:** quatro assentos Joy-Con e Pro na porta seguinte.
  A desconexão temporária mantém assentos neutros enquanto a sessão existir.
- Alterações na quantidade de assentos anunciam **somente as novas portas** ao
  Sunshine. O cliente envia a máscara atualizada nos eventos de estado dos
  controles restantes, evitando reanúncio dos dispositivos já existentes.
- Uma nova sessão redefine somente essa topologia temporária; os hashes de
  identidade dos Joy-Cons e a calibração dos sensores são mantidos.

## Matriz de testes a executar

| Caso | Expectativa |
| --- | --- |
| Pro conectado antes do streaming | Um gamepad FullKey, como no upstream |
| Pro-native: stream iniciado com 0 controles, Pro conecta depois | De 0 para 1 gamepad |
| Pro desconecta e reconecta durante sessão Pro-only | Retorno à mesma porta após a nova enumeração |
| Joy-Cons separados, stream com 0 Joy-Cons | Quatro posições neutras |
| Pro conectado primeiro, depois dois pares Joy-Con | Pro permanece porta 0, novos controles em outras portas |
| Joy-Cons conectados primeiro, depois Pro | Quatro posições Joy-Con preservadas; Pro aparece depois |
| Pro desconectado temporariamente em sessão mista | Assento virtual neutro, sem compactar Joy-Cons |
| Modo portátil com Pro conectado | Caminho handheld e Pro sem remapear botões FullKey |
| Pares Joy-Con | Um controle por JoyDual, preservado |
| Dois Pro Controllers | Cada controle recebe uma porta sem duplicação |

## Testes disponíveis

O teste C++ `tests/stable_virtual_gamepad_count_test.cpp` verifica a contagem
por topologia, incluindo Pro conectado antes/depois da sessão. O teste
`extern/borealis/tests/hybrid_controller_slots_test.cpp` verifica a ordenação
física, remoção, slots neutros e limites do vetor; os testes de origem
`tests/test_pro_controller_compatibility.py` verificam preservação dos caminhos
FullKey, rumble e declaração de capacidades de motion.

**Limitação inerente:** não é possível anunciar simultaneamente *zero* portas
para um futuro Pro Controller **e** quatro portas para futuros Joy-Cons quando
nenhum deles está conectado; o cliente ainda não sabe qual chegará depois.
Por isso existe o modo explícito Pro Controller. Para resultados reprodutíveis
em uma sessão com emulador, selecione o modo antes de iniciar o stream.

O teste de compatibilidade final exige hardware, Sunshine e emulador. Testes
isolados compilados no PC não comprovam funcionamento e tempos de resposta
do Switch real.

## Builds

Esta branch não deve gerar compilações em cada commit. O workflow Switch
é **somente manual** (`workflow_dispatch`), e a v20 só deve ser compilada
quando a mudança consolidada estiver pronta para validação.
