# Validação de precisão do gyro — mouse e movimento nos jogos

## O que significa atingir 90%

O projeto não pode atribuir uma probabilidade de precisão sem um conjunto de testes reais e comparáveis. Para o mouse, o gate primário é **no mínimo 90 acertos em 100 tentativas reais**, usando posições-alvo e cursor normalizadas por largura e altura. O erro euclidiano máximo padrão é `0.01` (1% da diagonal do espaço normalizado). É uma métrica operacional proposta, não uma garantia estatística ou especificação da Nintendo.

Registre cada tentativa em CSV:

```csv
target_x,target_y,cursor_x,cursor_y
0.5,0.5,0.503,0.497
0.2,0.8,0.214,0.806
```

Depois execute:

```bash
python3 tools/evaluate_gyro_targets.py trials.csv --tolerance 0.01
```

O relatório contém tentativas, acertos, taxa de acerto e erro p95. Use **100 ou mais tentativas**, incluindo alvos pequenos, cantos e região central, em três velocidades de movimento. Repita com controles separados e com ambos os pares de Joy-Cons. Não confunda teste sintético automatizado com medição física.

## Testes independentes para jogos

No Fritchii/Lichii, valide os quatro controladores com botões e movimento, depois confirme qual porta SDL está associada a cada lado de cada jogador. Faça movimentos de yaw, pitch e roll separadamente com os Joy-Cons; para cada eixo compare sentido, amplitude e retorno à posição anterior. Confirme que o jogo recebe dados novos pela porta UDP 26760 e que não há descontinuidade entre amostras. No gyro de jogos não aplicar filtro de ponteiro, pois isso distorce a resposta angular e acrescenta latência.

## Diferencie classes de erro

- Drift quando imóvel: medir RMS e pico do giroscópio parado em `motion_hid.csv`, além de deslocamento acumulado em `motion_pointer.csv`.
- Movimentos não reconhecidos: comparar rotações lentas e rápidas e o número de amostras HID rejeitadas.
- Erro de orientação: validar separadamente eixos e sinais em cada lado físico, com referência externa filmada.
- Latência fim a fim: exige vídeo sincronizado ou medição no host; os registros locais não a demonstram.
- Docked versus portátil: executar também o cenário portátil com Joy-Cons desconectados para separar Bluetooth do dock.

## Estado da branch v17

- Mouse: retirado o gate temporal fixo de 20 ms; atenuação contínua de baixa velocidade; suavização menor em giros rápidos.
- Jogos: validação de pacote numérico sem transformação adicional; reuso de snapshot HID por ciclo em Borealis.
- Quatro Joy-Cons: preservada a correção de pares JoyDual adjacentes e testes para quatro e seis metades.
- **Aprovação ainda pendente:** suíte C++ completa, build Switch 1 e comparação no console e no emulador. Nenhuma porcentagem real de acerto foi medida nesta iteração.
