# Recalibrar giroscópio durante o jogo: L3 + R3 (v17)

## Atalho

Mantenha **L3 + R3** pressionados simultaneamente por **2 segundos**. No modo separado, pressione L3 no Joy-Con esquerdo e R3 no direito **do mesmo jogador**. O jogador 1 usa os controles virtuais 0/1 e o jogador 2, 2/3, desde que estes sejam Joy-Cons separados na ordem lógica esperada. No modo pareado, os dois botões são lidos no mesmo gamepad virtual.

- O atalho é detectado por jogador e dispara **uma vez por pressão contínua**; solte os dois botões antes de repetir.
- Durante a combinação simultânea, os eventos L3/R3 não são transmitidos ao jogo para evitar ações acidentais.
- Ao completar os 2 segundos, o Moonlight solicita calibração dos sensores referentes ao par e registra a operação no log.
- O mouse por giroscópio é recentralizado quando o controle do ponteiro pertence ao par selecionado.
- **Mantenha os Joy-Cons parados após o disparo**, permitindo coletar 40 amostras HID consecutivas em repouso (até 5 segundos para a calibração manual). O mecanismo existente verifica o estado real de repouso e não aceita amostras enquanto há movimento.
- Em caso de falha de identificação do sensor, será registrada mensagem de aviso sem interferir nos demais jogadores.

## Limite do protocolo e do emulador

A versão implementada **não envia uma mensagem especial de 'Joy-Cons verticais' ao Sunshine nem ao Fritchii**. O caminho Moonlight → Sunshine encaminha velocidade angular e aceleração do controle virtual DS4. Não há comando de recenter da orientação interna do Fritchii definido no evento de movimento Moonlight. Fabricar tal evento ou transmitir um estado falso de aceleração pode causar saltos e não representa uma calibração válida.

Para redefinir a orientação **dentro do Fritchii**, será preciso identificar uma função/hotkey existente que redefina o referencial de movimento e mapeá-la a um evento de teclado reconhecido no host, **ou** implementar uma integração explícita no emulador. Essa integração não está concluída nesta branch e não deve ser anunciada como funcional.

A postura vertical serve de referência física para o jogador, mas o presente atalho executa calibração de **bias angular em repouso**, não determina um heading absoluto. Sem referência externa, acelerômetro e giroscópio não tornam yaw globalmente observável.

## Checklist de testes em hardware

1. Um par: L3 isolado, R3 isolado, combo por 1,9 s, combo por 2,1 s, pressão prolongada e repetição após soltar.
2. Dois pares: calibrar jogador 1 enquanto jogador 2 se movimenta; repetir com jogador 2 e confirmar que a calibração é independente.
3. Calibração enquanto os Joy-Cons se movimentam: nenhuma amostra desse período deve contribuir para estimar o bias.
4. Controlar mouse gyro do par selecionado e verificar recentralização sem desativar DSU.
5. Confirmar a ausência de eventos L3 e R3 enviados ao Sunshine durante o combo e testar o jogo após a liberação.
6. Repetir em portátil e docked e comparar os logs de sensores.
7. Verificar separadamente se o Fritchii mantém ou não sua orientação acumulada depois do atalho; não confundir o comportamento do cliente com a orientação do emulador.

Ainda falta validar o fluxo completo em Switch + Sunshine + Fritchii. Não publicar build final com alegação de calibração de orientação do emulador.
