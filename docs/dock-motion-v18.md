# v18 — robustez do gyro no modo docked

## Evidência no código

Na v17, `SwitchInputManager::handleControllerSensors()` era chamado pelo `runloopStart()` do Borealis, dentro de `Application::processInput()`. O sensor era consultado com `hidGetSixAxisSensorStates(..., 1)`, portanto apenas o estado mais recente era consumido a cada ciclo. O vídeo é renderizado no mesmo fluxo principal de apresentação; se houver menos ciclos de entrada, amostras intermediárias deixam de ser processadas.

No modo de resolução automática, `MoonlightSession::start()` seleciona `Application::windowWidth/windowHeight`, o que pode resultar em streaming maior no dock. Isso não modifica diretamente a medição dos sensores, mas pode alterar carga e cadência de apresentação. A v18 **não altera a resolução configurada pelo usuário**.

Além disso, o estado físico passa de `HidNpadIdType_Handheld` para controladores `NpadJoyDual/Left/Right` ao destacar Joy-Cons. O índice lógico do Joy-Con direito pode mudar: agora o ponteiro verifica o controle preferencial até uma vez por segundo: sensor handheld quando os Joy-Cons estão encaixados e Joy-Con direito quando estão destacados. A referência é reiniciada somente se houver alteração na seleção, em ambos os sentidos da transição.

## Prevenção implementada

- O Borealis consulta até **8 estados HID por sensor** por passagem do loop, em vez de apenas um. O buffer da libnx retorna o mais recente primeiro; os estados são processados do mais antigo para o mais novo.
- `SixAxisQuality` continua rejeitando amostras duplicadas ou antigas, com calibração e viés independentes por sensor. Até oito amostras podem recuperar o movimento ocorrido enquanto a renderização esteve ocupada.
- O `SensorEvent` transporta o `delta_time` do HID em segundos após validação de 1 a 50 ms. O ponteiro usa o intervalo real de amostragem para integrar velocidades angulares; se inválido, recorre ao tempo local.
- Os jogos recebem cada amostra válida em ordem cronológica pelo caminho existente de `LiSendControllerMotionEvent`. Nenhuma transformação de eixos foi introduzida.
- Não foi criada thread concorrente: o fluxo de callbacks permanece na thread anterior, evitando riscos de corrida em Borealis e Moonlight nesta iteração.
- O número máximo de oito amostras por passagem evita explosão ilimitada de mensagens após um travamento prolongado. Se o loop passar tempo demais sem rodar, ainda é possível perder amostras: **a correção é parcial, não uma garantia de latência**.

## Comparação em hardware

Sem trocar a versão durante a comparação, use um único par de Joy-Cons, mesma rede e mesmo jogo:

1. Switch portátil, Joy-Cons encaixados; gravar 30 s em repouso, 30 movimentos lentos e 30 rápidos.
2. Switch portátil, Joy-Cons destacados; repetir. Este passo isola o efeito da conexão sem fio.
3. Switch docked, Joy-Cons destacados: repetir. Posicionar console sem obstáculos, longe de equipamentos USB 3.0 e equipamentos que produzam interferência.
4. Repetir o teste docked com resolução explicitamente configurada em **720p e 60 FPS**, depois com resolução maior (se habilitada), mantendo os demais parâmetros iguais.
5. Testar a entrada no dock durante uma sessão com mouse gyro ativado, observando se o Joy-Con direito permanece selecionado.
6. Repetir com dois pares (quatro Joy-Cons) e verificar identificadores, movimentos e latência de ambos os jogadores.

### Arquivos e comparação

Para cada sessão, criar no microSD `sdmc:/switch/Moonlight-Switch/motion_trace.enable` e reiniciar o cliente. Os CSVs serão gerados em `sdmc:/switch/Moonlight-Switch/`:

- `motion_hid.csv`
- `motion_send.csv`
- `motion_pointer.csv`

Copiar os arquivos completos para duas pastas no PC, por exemplo `portable/` e `docked/`; não misturar sessões. Executar:

```bash
python3 tools/compare_dock_motion.py portable/ docked/ --output dock_comparison.json
```

O relatório inclui sequências HID não recebidas, amostras rejeitadas, intervalos de amostragem, erros de enfileiramento e dispersão de movimento do cursor. Comparar sempre períodos e trajetórias semelhantes. Ativar rastreamento CSV pode aumentar I/O e carga; repetir pelo menos um teste sem logs.

## Validação

- Teste local C++ da função pura de conversão de intervalos e reprodução cronológica (8 amostras): passou.
- A suíte do projeto, a compilação completa da v18 e os testes em Switch físico ainda não foram realizados.
- Uma diferença positiva nos dados locais não comprova latência fim a fim nem ausência de interferência Bluetooth.
- A v17 que teve bons resultados no portátil foi preservada em sua própria branch. A v18 é uma candidata separada e ainda não deve substituir a versão aprovada sem testes.
