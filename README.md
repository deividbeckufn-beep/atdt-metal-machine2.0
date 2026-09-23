# ATDT METAL MACHINE

Instrumento virtual (VST3) para metal moderno, nu metal, metalcore, industrial,
trap metal e dubstep. Feito em C++ com JUCE. Plataforma principal: Windows x64 + REAPER.

> Versao atual: **0.2.0 - Fase 2** (presets, modulacao, LFOs, macros, voice/unison).

---

## Jeito mais facil: instalador pronto (.exe)

Siga **[COMO_GERAR_O_INSTALADOR.md](COMO_GERAR_O_INSTALADOR.md)**. O GitHub compila no Windows,
testa todos os presets e entrega um instalador `.exe`. Voce nao precisa instalar ferramentas de programacao.
Compilar no proprio PC (Visual Studio) continua possivel: veja **BUILD_WINDOWS.md**.

---

## O que ja funciona (Fases 1 e 2)

Tudo abaixo esta ligado ao motor de audio real. Nada e decorativo.

**Sintese (aba SYNTH)**
- **3 osciladores**: Sine, Triangle, Saw, Square, Pulse, Noise (anti-aliasing PolyBLEP).
  OCT, SEMI, FINE, PHASE, LEVEL, PAN, com display da forma de onda em tempo real.
- **SUB** (Sine/Triangle/Square) e **NOISE** (White/Pink/Digital).
- **FILTER**: LP12, LP24, HP12, HP24, BP, NOTCH. CUTOFF, RESO, DRIVE, KEY TRK, MIX, com curva real.
- **FILTER ENV** e **AMP ENV** com pontos arrastaveis no grafico.
- **VOICE**: Poly / Mono / Legato, 1 a 32 vozes, GLIDE, UNISON (ate 8), DETUNE, SPREAD.
- **MASTER**: ganho (-60 a +18 dB), faixa do pitch bend, **MONO <** (grave mono abaixo da frequencia escolhida).

**Modulacao (aba MOD)**
- **4 LFOs**: Sine, Triangle, Saw, Square, S&H, Random. **SYNC** com o BPM do REAPER
  (2/1 a 1/32 e tercinas) travado no compasso durante o play, ou RATE livre em Hz. PHASE, FADE, AMOUNT.
  O display mostra onde o LFO esta de verdade no audio.
- **Mod Matrix** com 8 slots: SOURCE, DESTINATION, AMOUNT, CURVE (Linear/Exp/Log), MODE (Normal/Invert).
- Fontes: LFO 1-4, Filter Env, Amp Env, Velocity, Mod Wheel, Aftertouch, Note, Macro 1-4.
- Destinos: Pitch, Pitch de cada OSC, Level de cada OSC, Sub, Noise, Cutoff, Resonance, Drive,
  Filter Mix, Amp, Pan, Unison Detune, Amount de cada LFO.
- **Arrastar e soltar**: a fileira **MOD SOURCES** (ao lado das abas) fica sempre visivel.
  Arraste um chip (ex.: LFO1) e solte sobre um knob (ex.: CUTOFF). O slot e criado na matriz
  e o knob ganha um **anel** mostrando a faixa de modulacao.
- **4 Macros** (AGGRESSION, MOVEMENT, DARKNESS, SPACE): duplo clique no nome para renomear.
  **EDIT MACRO** mostra na matriz so o que as macros controlam.

**Presets**
- **33 presets de fabrica** com volumes equilibrados: Bass (11), Trap (1), Dubstep (2), Keys (7),
  Pads (3), Leads (4), Plucks (2), Arps (1), FX (2).
- **Browser**: clique no nome do preset no topo. Categorias, busca, favoritos (estrela), autor.
  Clique carrega na hora; setas cima/baixo navegam; Enter/duplo clique fecha; Esc fecha.
- Setas **< >** no topo trocam de preset (respeitando a categoria/busca do browser).
- **SAVE** / **SAVE AS** / **DELETE** (preset do usuario vai para a lixeira).
- Presets do usuario: `Documentos\ATDT Metal Machine\Presets\<CATEGORIA>\`.
  Sao arquivos `.atdtpreset` (XML): da para copiar e compartilhar.
- Os presets de fabrica tambem aparecem no **menu de presets do proprio REAPER** (programs do VST3).

**Geral**
- MIDI: notas, velocity, sustain (CC64), pitch bend, mod wheel (CC1), aftertouch.
- **MIDI Learn** no clique direito de qualquer knob. Salvo no projeto.
- Automacao de todos os parametros no REAPER; estado completo salvo no projeto.
- INIT, escala da interface (80-150%), temas METAL e CYBER (menu **MENU**).

As abas **FX, SEQ, DRUMS e PERFORM** mostram apenas o que vira em cada fase (sem controles falsos).

## Como usar os knobs

| Acao | Resultado |
|---|---|
| Clicar e arrastar | ajusta |
| SHIFT + arrastar | ajuste fino |
| Duplo clique | volta ao valor padrao |
| Roda do mouse | ajusta |
| Clique direito | MIDI Learn / remover CC / remover modulacoes / valor padrao |
| Soltar um chip de MOD SOURCES | cria modulacao nesse knob |

---

## Requisitos

- Windows 10 ou 11, 64 bits
- Visual Studio 2022 Community (gratis), com "Desenvolvimento para desktop com C++"
- CMake 3.22 ou superior
- Git
- REAPER (64 bits)

O passo a passo completo, do zero, esta em **[BUILD_WINDOWS.md](BUILD_WINDOWS.md)**.

## Compilacao rapida

1. Dois cliques em `build_windows.bat`
2. Clique direito em `install_vst3.bat` -> **Executar como administrador**
3. No REAPER: *Options > Preferences > Plug-ins > VST > Re-scan*

## Instalacao no REAPER

1. O arquivo `ATDT METAL MACHINE.vst3` deve estar em:
   `C:\Program Files\Common Files\VST3\`
2. Abra o REAPER.
3. Va em **Options > Preferences > Plug-ins > VST**.
4. Confira se `C:\Program Files\Common Files\VST3` esta na lista de caminhos.
5. Clique em **Re-scan** (ou *Clear cache/re-scan*).
6. Feche as preferencias.
7. Menu **Track > Insert virtual instrument on new track...**
8. Procure por **ATDT METAL MACHINE** (em *VST3i*) e confirme.

## Configurando o teclado MIDI no REAPER

1. Conecte o teclado **antes** de abrir o REAPER.
2. **Options > Preferences > Audio > MIDI Inputs**: clique no seu teclado e marque
   **Enable input**. Clique em OK.
3. Na trilha do plugin, clique no botao vermelho **Record arm**.
4. Clique com o botao direito no botao Record arm -> **Input: MIDI > All MIDI inputs > All channels**.
5. Ative o **Record monitoring** (icone de alto-falante ao lado do Record arm).
6. Toque. O LED **MIDI** no topo do plugin acende a cada nota.

## Solucao de problemas

**O plugin nao aparece no REAPER**
- Confira se o arquivo esta mesmo em `C:\Program Files\Common Files\VST3\`.
- Faca *Clear cache/re-scan* nas preferencias de VST.
- Veja se o REAPER e 64 bits (o plugin e 64 bits).

**O plugin aparece mas nao faz som**
- A trilha precisa estar com **Record arm** + **Record monitoring** ligados e com a entrada MIDI certa.
- Clique no teclado virtual do plugin: se sair som ali, o problema esta na configuracao do MIDI.
- Veja se o **GAIN** do MASTER nao esta em -inf.

**Chiados ou falhas no audio**
- Em *Options > Preferences > Audio > Device*, use o driver **ASIO** da sua placa (ou ASIO4ALL)
  e aumente o buffer (256 ou 512 amostras).

**Mudei de preset e o volume ficou diferente do esperado**
- Os presets de fabrica foram nivelados entre -10 e -12 dBFS (nota unica para baixos, acorde para teclas/pads).
  O botao GAIN do MASTER faz parte do preset.

**O preset que salvei nao aparece**
- MENU > "Recarregar lista de presets". Confira a pasta em MENU > "Abrir pasta de presets do usuario".

**MIDI Learn nao responde**
- Mexa o knob/fader fisico **depois** de escolher "MIDI Learn" (o knob pisca enquanto espera).
- O pedal de sustain (CC64) nao pode ser mapeado: ele sempre controla o sustain.

---

## Estrutura do projeto

```
ATDT-METAL-MACHINE/
├── CMakeLists.txt          configuracao de build (baixa o JUCE sozinho)
├── build_windows.bat       compila no Windows
├── install_vst3.bat        instala o VST3 (executar como administrador)
├── COMO_GERAR_O_INSTALADOR.md   instalador .exe pelo GitHub (sem instalar ferramentas)
├── .github/workflows/      compilacao automatica no Windows + testes + instalador
├── installer/              script do instalador (Inno Setup)
├── README.md               este arquivo
├── BUILD_WINDOWS.md        passo a passo de compilacao
├── Docs/ARCHITECTURE.md    arquitetura, fluxo de sinal e plano das fases
├── Tests/PluginTest.cpp    teste automatico: toca todos os presets e verifica o audio
├── Samples/                samples WAV licenciados (vazio por enquanto)
└── Source/
    ├── PluginProcessor.*   ponto central: MIDI -> synth -> saida, estado
    ├── PluginEditor.*      janela do plugin (escala e tema)
    ├── Parameters/         todos os parametros automatizaveis
    ├── DSP/                oscilador, noise, filtro
    ├── Synth/              motor de vozes (SynthEngine, SynthVoice)
    ├── MIDI/               atividade MIDI, MIDI Learn
    ├── Sequencer/          sincronia de tempo com o REAPER (sequencer: Fase 3)
    ├── UI/                 tema, knobs, displays, paginas
    ├── Drums/              Fase 4
    ├── Presets/            PresetManager + 33 presets de fabrica
    ├── Effects/            Fase 6
    └── Utilities/
```

## Teste automatico (opcional)

```
cmake -B build -DMM_BUILD_TESTS=ON
cmake --build build --config Release --target PluginTest
build\PluginTest_artefacts\Release\PluginTest.exe "build\MetalMachine_artefacts\Release\VST3\ATDT METAL MACHINE.vst3"
```
Carrega o VST3 como um host, toca todos os presets e confere: gera audio, sem NaN/Inf,
nenhuma nota presa depois do release, estado salvo/restaurado.

## Dependencias

- **JUCE 8.0.9** (baixado automaticamente pelo CMake). Licenca: AGPLv3 ou licenca comercial JUCE.
  Para vender o plugin com codigo fechado, e preciso uma licenca comercial do JUCE.
- **VST3 SDK**: ja incluso no JUCE.
- Nenhum sample, preset ou codigo de terceiros proprietario esta incluido.
