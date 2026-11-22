# v19 — conexão de Joy-Cons e calibração do emulador

## O que foi confirmado no código

- O Borealis original reconstrói os controles pela posição `HidNpadIdType`; a ordem pode mudar ao conectar outro par e reutilizar índices.
- `LiSendControllerArrivalEvent` anuncia os controles virtuais ao Sunshine. A reserva de posições evita recriá-los a cada reconexão, mas **não garante**, sozinha, a identidade física e o mapeamento dentro do Lichii.
- O Lichii build 386, em `src/hid_core/frontend/motion_input.cpp`, usa `MotionInput::Calibrate()` para reajustar o viés e `StopCalibration()` para executar `ResetQuaternion()` e `ResetRotations()`. O protocolo de motion do Moonlight/Sunshine não contém comando para chamar diretamente essas rotinas no emulador.
- A v18 enviava os primeiros dados de giroscópio antes da conclusão da correção automática de viés. O estado de orientação do emulador pode integrar essa rotação inicial indevida.

## Mudanças na v19 em desenvolvimento

1. **Associação física:** Borealis consulta `hidsysGetUniquePadsFromNpad`, `hidsysGetUniquePadType` e `hidsysGetUniquePadSerialNumber`. Um hash do serial identifica fisicamente o Joy-Con e mantém as posições L/R do jogador 1 e do jogador 2 quando o sistema muda os índices Npad. Caso falhe a identificação, preserva o mapeamento anterior ou utiliza a ordenação compatível de fallback.
2. **Persistência:** armazena apenas os hashes de serial (não os seriais em texto puro) em `sdmc:/switch/Moonlight-Switch/joycon_seats.bin`. IDs válidos apenas durante a conexão não são restaurados após reiniciar.
3. **Calibração automática dos sensores:** cada nova aquisição SixAxis inicia uma janela de 40 amostras estacionárias por sensor, limitadas a 5 s. Enquanto o sensor está em repouso e a calibração pendente, rotações residuais não calibradas são enviadas como zero; o acelerômetro continua atualizado. Movimentos intencionais não ficam bloqueados aguardando a calibração.
4. **Reaproveitamento do mesmo Npad:** se um Joy-Con físico diferente assumir o índice HID de outro, o viés e a sequência de amostras anteriores são descartados e a calibração inicial é reiniciada.
5. **Mouse por gyro:** a compensação de offset agora substitui, em vez de acumular, o deslocamento do repouso. A suavização de baixa velocidade reage de imediato quando o usuário inverte o sentido do movimento.

## Limite da correção

A calibração do **viés físico** do Joy-Con e a **recentralização do quaternion interno do Lichii** não são a mesma operação. A v19 melhora a primeira e evita enviar deriva inicial ao emulador, mas não pode invocar automaticamente `MotionInput::Calibrate()` sem modificação explícita do host/emulador ou mecanismo documentado de comando. A mera inicialização dos sensores no Switch **não é prova** de que o emulador mudou seu referencial.

Não alterar artificialmente a orientação de cada sensor com matrizes adivinhadas: a conversão espacial tem de ser medida nos Joy-Cons esquerdos e direitos, com seus posicionamentos reais. Em especial, não usar uma tabela genérica de orientação do Joy-Con direito para o esquerdo.

## Cenários a validar no console e no PC

- Conectar primeiro o par 2, depois o par 1; observar perfis e portas do Sunshine/Lichii
- Desconectar e conectar cada metade em Npad diferente, mantendo o mesmo perfil por identidade física
- Conectar com os Joy-Cons **imóveis por aproximadamente 1 s**; confirmar viés estável e ausência de salto no gyro
- Repetir a conexão com movimento imediato e confirmar que a entrada não trava
- Testar giroscópio L/R individualmente e ambos os pares, começando com controlador de movimento em posição neutra
- No Lichii, verificar se a orientação inicial após conectar corresponde à orientação após usar o comando de calibração do emulador. Se for diferente, há estado interno do emulador não corrigido pelo cliente
- Comparar reversões pequenas para a esquerda e direita, e medir offset em repouso após longa sessão
- Testar o botão HOME sem fio e o mapeamento de botões no jogo, não apenas no overlay do Moonlight

## Validação automatizada

Testes C++ independentes: `sixaxis_quality_test.cpp`, `stable_joycon_identity_test.cpp`, `gyro_aim_pointer_test.cpp` e `gyro_aim_accuracy_synthetic_test.cpp`.

**Nenhum NRO v19 foi autorizado ou criado após estas mudanças.** O workflow de Switch deve ser acionado somente manualmente, quando solicitado. Os testes não representam precisão ou configuração comprovada no hardware até serem executados e cruzados com os logs físicos.
