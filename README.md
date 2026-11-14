# Telephone Beep – VST simple (tons DTMF)

Petit plugin pour faire les sons de touches de téléphone à l’ancienne + reverb.

## Utilisation
- Clique sur les touches dans l’interface
- Ou envoie des notes MIDI C4 → B4 (notes 60 à 71)
- Règle la reverb et la longueur du bip avec les sliders

## Compiler

### Avec CMake (recommandé)
```bash
git clone https://github.com/juce-framework/JUCE.git
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Le plugin se trouve dans `build/TelephoneBeep_artefacts/Release/`.

### Avec Projucer
Ouvre `TelephoneBeep.jucer` dans Projucer et exporte vers Visual Studio / Xcode.

---

Créé pour faire des sons de touches de téléphone bipants à l’ancienne.
