# Coleta controlada de movimento dos Joy-Cons — diagnóstico v15

Esta versão experimental parte da v13 e **não introduz filtros novos nem muda os eixos**. O objetivo é identificar a origem das regressões de gyro da v14 sem prejudicar a referência de comparação.

## Ativar registros

1. No cartão SD, abra `sdmc:/switch/Moonlight-Switch/`.
2. Crie um arquivo vazio chamado `motion_trace.enable` (o nome completo, sem extensão adicional).
3. Encerre completamente o Moonlight e abra novamente com o arquivo presente.
4. Inicie uma transmissão com os Joy-Cons conectados e reproduza os sintomas.
5. Saia do Moonlight após a captura. Os arquivos são salvos no diretório acima.
6. Copie os três CSVs para o PC e remova `motion_trace.enable` para desativar a gravação nas próximas sessões.

**Atenção:** os CSVs são substituídos ao iniciar uma nova sessão de captura. Copie-os antes de reiniciar o Moonlight. O registro é opcional, possui buffering e encerra a gravação após um limite de linhas, mas pode afetar a latência em cartões SD lentos. Não utilize esse modo como medição definitiva de desempenho sem compará-lo com os logs desativados.

## Arquivos gerados

- `motion_hid.csv`: tempo monotônico local (microssegundos), controlador lógico, slot HID, lado físico, índice do sensor, número da amostra, delta do firmware, estado de repouso, aceitação, aceleração e giro brutos, ângulos e matriz de orientação fornecida pelo Switch, valores transformados e viés.
- `motion_send.csv`: tempo do enfileiramento local, controle lógico, amostra, tipo, valores de entrada, valores entregues à API Moonlight, ajuste de sensibilidade e código de retorno.
- `motion_pointer.csv`: tempo local, amostra e duração usados pelo mouse, velocidade angular, ganho, posição normalizada produzida e sinal de aceitação.

Esses arquivos registram apenas dados de movimento, sem teclas digitadas ou credenciais. O valor `t_us` usa o relógio monotônico do console para correlacionar eventos **dentro do Switch**.

## Roteiro recomendado

Execute os passos nesta ordem, anotando aproximadamente o horário de início de cada etapa:

1. **15 segundos:** Joy-Cons apoiados e completamente imóveis.
2. **15 segundos:** Joy-Cons nas mãos, tentando permanecer imóveis.
3. **15 segundos:** rotações esquerda/direita e retorno à pose inicial, primeiro no direito e depois no esquerdo.
4. **15 segundos:** inclinação para cima/baixo, seguida de torção no próprio eixo longitudinal.
5. **15 segundos:** translação da mão para esquerda/direita/cima/baixo, procurando manter a mesma orientação.
6. **30–60 segundos:** reproduza o erro dentro de um jogo no Lichii; se possível, grave em vídeo os indicadores de movimento do emulador.
7. **30 segundos:** ative o mouse por giroscópio, mantenha parado, mire em pontos pequenos e retorne ao centro.

Mantenha a sensibilidade de gyro em **100%** durante a coleta, sem alterar o modo de Joy-Cons nem calibrar durante o roteiro. Se a sensibilidade habitual é diferente, anote-a para uma segunda coleta separada.

## Gerar resumo opcional no PC

No repositório, execute:

```bash
python tools/analyze_motion_trace.py /caminho/para/os/csvs --output motion_summary.json
```

Compartilhe `motion_hid.csv`, `motion_send.csv`, `motion_pointer.csv` e, se gerado, `motion_summary.json`. Eles permitem detectar intervalos irregulares, lacunas de amostras, movimento em repouso, conversões de eixos/unidades, tempo entre leitura HID e enfileiramento no Moonlight e deslocamentos do ponteiro.

**Limite da instrumentação:** o envio pelo Moonlight pode ser coalescido. Um código de retorno bem-sucedido não comprova recebimento de todos os eventos no PC. Estes CSVs não medem o atraso total até o jogo no Lichii; isso exige telemetria adicional no PC ou captura de vídeo sincronizada.

## Recuperar a v13 se necessário

A captura usa o comportamento da v13 e não substitui uma build estável. Em caso de falha, retorne ao artefato da branch `joycon-gyro-mouse-stable-mapping-v13`, removendo o arquivo `motion_trace.enable`.
