# Moonlight-Switch — Joy-Con Motion Fork

Este repositório é um fork do [Moonlight-Switch original](https://github.com/XITRIX/Moonlight-Switch), uma versão do [Moonlight Game Streaming](https://github.com/moonlight-stream) para Nintendo Switch e outras plataformas. Ele transmite jogos e aplicativos de um computador para o console, usando um host compatível com o protocolo GameStream, como o [Sunshine](https://github.com/LizardByte/Sunshine).

**Objetivo deste fork:** tornar o uso dos Joy-Cons mais previsível como controles independentes, como pares e como fontes de movimento (giroscópio e acelerômetro), tanto no mouse quanto nos jogos executados no computador.

> **Estado:** desenvolvimento experimental, especialmente nas mudanças de motion e na transição portátil ↔ dock. Testes automatizados de funções isoladas não substituem testes no console. Os recursos de calibragem local **não** redefinem automaticamente a orientação interna do emulador.

## Funcionalidades principais

### Mantidas do projeto original

- Streaming de vídeo, áudio e entrada de jogos/aplicativos, com suporte a Sunshine e controles.
- Interface navegável por controles, sobreposição durante a sessão, configurações de conexão, gamepad e mouse.
- Entrada por tela sensível ao toque, teclado e mouse compatíveis.
- Suporte a outras plataformas e fluxos de compilação presentes no projeto upstream, sem garantia de validação dessas plataformas neste fork.

### Adaptações para Joy-Con

- **Controles separados e pareados:** atribuição lógica dos Joy-Cons esquerdo e direito, com suporte a dois pares (quatro metades) e recuperação de situações específicas de enumeração HID `JoyDual`.
- **Giroscópio no mouse:** ponteiro baseado em integração angular, suavização adaptativa para movimentos pequenos e bloqueio de deriva em repouso confirmado pelo sensor.
- **Motion para jogos:** encaminhamento de giroscópio e acelerômetro ao host por eventos de controle, permitindo o uso com controles virtuais que exponham motion.
- **Calibração de sensores:** correção de viés angular independente por sensor, incluindo calibração automática conservadora durante repouso.
- **Atalho L3 + R3:** mantenha pressionado por **2 segundos** para solicitar calibração local do jogador e recentralizar o mouse por gyro associado. Mantenha os controles imóveis para a coleta de amostras de repouso. **O atalho não transmite um comando de recentralização do referencial ao emulador.**
- **Robustez no dock (v18):** reprodução de até oito amostras HID por passagem, integração do mouse pelo intervalo do sensor e nova seleção de fonte ao mudar a configuração física dos controles.

A estabilidade dessas alterações depende do firmware, da comunicação dos Joy-Cons, da configuração do Sunshine e do programa receptor. Não há garantia de precisão de 90% em hardware sem medições específicas.

## Instalação no Nintendo Switch 1

1. Acesse [Releases](https://github.com/GutoMitzy/Moonlight-Switch/releases) e baixe o `Moonlight-Switch.nro` da versão mais recente para Switch 1.
2. Copie o arquivo para `sdmc:/switch/Moonlight-Switch/Moonlight-Switch.nro`, criando a pasta se necessário.
3. Abra o Homebrew Menu via **Title Redirection** para disponibilizar a memória necessária ao streaming. Inicie o Moonlight-Switch.
4. Configure o [Sunshine](https://github.com/LizardByte/Sunshine) no PC, pare o computador no Moonlight e inicie o jogo ou aplicativo desejado.

A instalação requer um Switch 1 com ambiente de homebrew.

### Configuração do gyro nos jogos

No Sunshine, selecione um tipo de controle virtual com suporte a sensores (por exemplo, **DualShock 4**, se disponível na sua instalação). No emulador, associe o jogador à porta virtual correta e configure seu provedor de movimento. O Motion DSU/UDP só funcionará se a ponte entre Sunshine, controle virtual e emulador estiver realmente configurada; o Moonlight não envia automaticamente pacotes DSU diretamente do console para o emulador.

No modo separado, confira a numeração dos dispositivos após conectar ou desconectar Joy-Cons. O comportamento de porta do emulador depende também de seus perfis salvos e da enumeração feita no host.

### Comandos durante o streaming

| Ação | Comando |
| --- | --- |
| Overlay do jogo | `-` + `+` por padrão (configurável) |
| Calibrar gyro do jogador | Segurar `L3` + `R3` por 2 segundos |
| Recentrar o mouse por gyro | Comando do overlay de mouse, quando disponível |
| Mouse por gyro | Ativar no overlay de entrada/mouse |

O atalho de calibração exige que o sensor permaneça imóvel para estimar seu viés. Ele **não** substitui a opção de recentralização/calibração de orientação do emulador, se existente.

## Logs e diagnóstico

Para ativar os registros de gyro, crie o arquivo vazio:

```
sdmc:/switch/Moonlight-Switch/motion_trace.enable
```

Reinicie o aplicativo. Os registros disponíveis incluem `motion_hid.csv`, `motion_send.csv` e `motion_pointer.csv`. Para análise:

```bash
python3 tools/analyze_motion_trace.py caminho/dos/logs/ --output motion_summary.json
python3 tools/compare_dock_motion.py portable/ docked/ --output dock_comparison.json
```

O registro acrescenta I/O e mede principalmente o comportamento **no Switch**. Não demonstra, sozinho, a recepção pelo emulador nem a latência fim a fim. Remova o arquivo `motion_trace.enable` quando não precisar coletar novos dados.

## Compilação para Switch 1

Use a branch `joycon-fidelity-v20` com o submódulo Borealis fixado na revisão do projeto:

```bash
git clone --recurse-submodules -b joycon-fidelity-v20 https://github.com/GutoMitzy/Moonlight-Switch.git
cd Moonlight-Switch
```

Se você já tem o checkout da branch, sincronize os submódulos com `git submodule update --init --recursive`.

Use o [devkitPro](https://devkitpro.org/wiki/Getting_Started), com devkitA64, libnx e as ferramentas para Switch, incluindo `uam` para os shaders Deko3D. O workflow usa a imagem `devkitpro/devkita64`. Nesse ambiente, remova `switch-enet` se estiver instalado para que a compilação use a cópia do ENet fornecida pelo submódulo Moonlight Common C.

```bash
cmake -S . -B build/switch -DCMAKE_BUILD_TYPE=Release -DPLATFORM_SWITCH=ON -DUSE_DEKO3D=ON
cmake --build build/switch --target Moonlight.nro --parallel
```

O CMake gera `build/switch/Moonlight.nro`. O [workflow de Switch](https://github.com/GutoMitzy/Moonlight-Switch/blob/joycon-fidelity-v20/.github/workflows/switch.yml) roda os testes antes da compilação e publica o NRO como `Moonlight-Switch.nro`. Para outras plataformas, consulte o [projeto original](https://github.com/XITRIX/Moonlight-Switch).

## Origem, licenças e créditos

Este trabalho é derivado do [Moonlight-Switch de XITRIX](https://github.com/XITRIX/Moonlight-Switch) e usa o protocolo do [Moonlight Game Streaming](https://github.com/moonlight-stream).

Créditos preservados do projeto original:

- [Rock88](https://github.com/rock88) e [Moonlight-NX](https://github.com/rock88/moonlight-nx): partes fundamentais do código de streaming.
- [Xfangfang](https://github.com/xfangfang) e [Borealis](https://github.com/xfangfang/borealis): biblioteca de interface e suporte de plataformas.
- [Averne](https://github.com/averne) e [FFmpeg](https://github.com/averne/FFmpeg): implementação NVDEC e orientações de decodificação.
- [Azkali](https://gitlab.com/Azkali) e [Moonlight-NX2](https://gitlab.com/Azkali/moonlight-nx2): contribuições relacionadas ao Switch 2.
- [Cooler3D](https://github.com/Cooler3Ds): implementação Deko3D e melhorias de desempenho.
- [AMD GPUOpen FidelityFX-FSR](https://github.com/GPUOpen-Effects/FidelityFX-FSR): implementação MIT de FSR 1.0 (EASU/RCAS) usada no caminho de upscaling Deko3D, © Advanced Micro Devices, Inc.

As licenças, avisos de direitos autorais e atribuições dos componentes upstream continuam aplicáveis. Este fork não é uma distribuição oficial dos mantenedores originais.
