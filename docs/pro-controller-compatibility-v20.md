# v20 — Pro Controller e compatibilidade com controles completos

## Requisito

O Nintendo Switch Pro Controller deve conservar o comportamento esperado do
Moonlight-Switch original: leitura de A/B/X/Y, D-pad, dois analógicos, L/R,
ZL/ZR, +/−, cliques dos analógicos, rumble e movimento SixAxis quando disponível,
sem sofrer o mapeamento de botões aplicado a Joy-Cons horizontais.

## Comportamento

- Uma transmissão vazia envia máscara zero ao host. A conexão de um Pro
  Controller no dock cria uma porta, inclusive com a configuração de Joy-Cons
  separados selecionada.
- Botões, analógicos, rumble e movimento usam o caminho HID FullKey. O Moonlight
  solicita emulação Xbox para FullKey e mantém o tipo Playstation para Joy-Cons.
  Em modo Auto, o Sunshine no Windows pode oferecer XInput para o Pro. Perfis
  manuais de emulação no Sunshine prevalecem sobre esse pedido.
- O Pro Controller já conectado no início da sessão fica na primeira porta
  também no modo portátil, seguido pelos controles integrados. Se o portátil
  já abriu a sessão sozinho, o Pro conectado depois recebe a próxima porta.
- As quatro portas dos Joy-Cons separados são criadas após detectar o primeiro
  Joy-Con destacado. Em uma sessão mista, o tipo de controle que entrou
  primeiro mantém sua posição. Uma falha de consulta ao número de série dos
  Joy-Cons preserva as posições conhecidas e ainda permite detectar FullKey.
- Uma nova transmissão redefine a ordem temporária e preserva a identidade
  salva dos Joy-Cons e a calibração dos sensores. O cliente tem até cinco
  portas lógicas por sessão, incluindo a entrada integrada no modo portátil.

## Matriz de validação em hardware

| Caso | Expectativa |
| --- | --- |
| Dock sem controles ao iniciar | Zero controles virtuais; conectar um Pro cria uma porta |
| Pro conectado antes de iniciar no dock | Um controle virtual, aceito em jogos XInput com Sunshine em Auto |
| Pro conectado antes de iniciar no portátil | Pro na primeira porta e entrada integrada na seguinte |
| Joy-Cons separados e depois Pro no dock | Mesmo mouse por gyro, calibração e perfis nas quatro portas Joy-Con; Pro na seguinte |
| Pro primeiro e depois Joy-Cons no dock | Pro mantém primeira porta; primeiro Joy-Con direito continua sendo a fonte do mouse por gyro |
| Desconexão temporária de um Joy-Con | Porta mantida durante a mesma sessão |
| Próxima transmissão sem controles | Volta a zero no dock |
| Joy-Cons pareados | Controles JoyDual mantêm o mapeamento do modo pareado |

A suíte C++ verifica máscaras e ordenação. O teste
`tests/test_pro_controller_compatibility.py` executa a lógica de topologia do
Borealis com HID simulado, incluindo conexão de FullKey durante uma falha de
identificação de Joy-Cons. O teste final de detecção em jogos depende de
hardware, Sunshine e do jogo ou emulador.
