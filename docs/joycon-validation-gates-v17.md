# Joy-Con v17 — bloqueios de lançamento e critérios de validação

Base: `joycon-mapping-evidence-v16`. Esta branch não está nos gatilhos de `push` do workflow Switch; não gerar builds de distribuição antes de concluir a validação.

## Achados verificáveis no código

1. `GyroAimPointer::update()` usa limiar de partida de 0,060 rad/s, limiar de parada de 0,032 rad/s e exige pelo menos 20 ms acima do limiar para iniciar movimento. Pequenos movimentos deliberados podem ser ignorados e há atraso adicional introduzido pelo filtro. Não afirmar que estes valores são adequados sem traces de movimentos lentos.
2. A orientação em `GyroAimPointer` é integrada a partir da velocidade angular; a aceleração é usada para correção de gravidade apenas no repouso. A gravidade não recupera yaw absoluto: deriva no heading não pode ser corrigida exclusivamente por acelerômetro. Não prometer mouse tipo apontamento absoluto de longo prazo sem referência externa. Avaliar modo relativo de velocidade angular, com botão de recentralização para modo absoluto.
3. `motion_hid.csv`, `motion_send.csv` e `motion_pointer.csv` medem eventos locais no console, não entrega pelo Sunshine, leitura UDP DSU ou tempo até o emulador. Código de retorno zero de `LiSendControllerMotionEvent` não confirma que o Lichii/Fritchii recebeu o sensor.
4. O relatório `analyze_motion_trace.py` usa `(controller,sample)` como chave de correspondência HID/envio. Em cenários de múltiplos sensores físicos para o mesmo controller, a chave pode colidir; a análise de latência HID->fila fica potencialmente ambígua. Revisar a identidade da amostra ponta-a-ponta antes de publicar números como prova de latência.
5. `stableVirtualGamepadCount()` impede redução da contagem de pads virtuais durante uma mesma sessão no modo separado. Isso reduz alterações de porta, mas não garante identidade persistente no host após reiniciar o stream, o Sunshine ou mudar a enumeração física. O DS4 e as associações por GUID/port no emulador precisam ser testados.
6. O workflow existente compila e publica artefatos no push da branch v16. Evitar commits nela durante a fase de análise. Os testes automatizados de contagem de pads verificam apenas valores retornados, não IDs reais do SDL nem persistência de perfis de jogos.

## Matriz de validação obrigatória antes da build

### Movimento e latência
- Joy-Con imóvel em superfície: identificar RMS, pico, frequência de falsos movimentos e deslocamento acumulado de cursor durante pelo menos 30 s.
- Movimento lento controlado em ambos os sentidos e nos dois eixos: medir movimentos perdidos, histerese, erro de sentido, início de movimento e retorno ao ponto de referência.
- Movimentos rápidos e pausas: detectar saturação, clipping de ponteiro, overshoot e recuperação depois de lacunas de amostras.
- Alterar orientação de apoio (vertical, horizontal, Joy-Con direito/esquerdo): validar transformação dos eixos por lado, sem aplicar uma mesma transformação assumida para todos os sensores.
- Separar latências: HID->fila no Switch; fila->host; host->emulador; efeito visual. Medir o total somente com captura externa ou timestamps correlacionáveis entre dispositivos.
- Comparar v13, v14 e candidato com o MESMO roteiro, sem atribuir melhorias a filtros sem evidência.

### DSU e emulador
- Capturar pacotes UDP 26760 no Windows (recebimento efetivo), distinguindo ausência de transmissão, dados constantes e erro de mapeamento de eixos.
- Garantir que ativar/desativar mouse por gyro não interrompa envio de gyro/aceleração do controle ao host.
- Testar remapeamento de usuário em sessão ativa, reinício do emulador, reinício do stream, inversão da ordem de conexão e reconexão individual.
- Registrar GUID, porta SDL, tipo de controle virtual Sunshine, perfil INI carregado e mensagens de log do emulador.
- Confirmar que a sessão continua configurável pelo menu in-game sem interromper a transmissão.

### Gate
Não publicar uma build como 'corrigida' enquanto não houver testes de host executados, resultado de compilação do alvo correspondente e uma rodada real em Switch + Sunshine + emulador. Indicar separadamente itens não testados. Não disparar builds em branches intermediárias.

## Teste A/B específico: docked vs portátil

**Não há validação física concluída.** Comparar com a mesma build, mesmo Joy-Con direito, mesma sensibilidade, mesmo jogo, mesmos FPS configurados no Moonlight e o mesmo Sunshine.

1. Portátil com Joy-Cons acoplados: registrar 30 s imóveis, 20 movimentos lentos horizontais, 20 verticais e 10 retornos à pose inicial.
2. Portátil com Joy-Cons destacados e conectados por Bluetooth, sem dock: repetir exatamente o protocolo. Essa etapa isola a diferença de transporte físico dos sensores.
3. Docked com Joy-Cons sem fio, Switch e controle próximos, preferencialmente com as mesmas opções de resolução/FPS: repetir.
4. Docked com vídeo reduzido a 720p e, depois, à resolução original: repetir. Essa etapa investiga influência de carga de decodificação/renderização.
5. Para minimizar variações, alternar a ordem A/B/A e não modificar sensibilidade/calibração durante a comparação.

Comparar p50/p95 de `sensor_dt_ms`, lacunas de sequência, RMS/pico em repouso, `local_hid_to_enqueue_ms`, rejeições de amostras, deslocamento de cursor parado e atraso visível. Os CSVs atuais não medem latência fim a fim ou recepção pelo emulador. É necessário vídeo externo sincronizado ou telemetria do PC para tais conclusões.

Interpretação condicional: se portátil acoplado difere do portátil desacoplado, investigar transporte dos Joy-Cons e indexação; se ambos portáteis são equivalentes e docked piora, investigar carga de processamento, frequência de polling, radiofrequência/posicionamento do dock e pipeline de apresentação. Não atribuir automaticamente a diferença ao dock sem o controle portátil sem fio.
