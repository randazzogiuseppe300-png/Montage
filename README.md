# MONTAGE - granulatore per Ableton Live

Plugin strumento (VST3 + AU) per macOS. Carichi un sample, lo suoni dal MIDI e Montage lo spezza in tanti piccoli grani, che ricompone cambiando posizione, durata, altezza e direzione.

Nell'interfaccia c'e' il nome **GIUSEPPE RANDAZZO** in grande, con **MONTAGE** sotto. I colori sono quelli del color grading cinematografico: arancio e verde acqua su nero.

## Come ottenere il DMG (senza installare nulla)

1. Crea un account su https://github.com (gratis).
2. Crea un nuovo repository, ad esempio `montage`.
3. Carica **tutti** i file di questa cartella, comprese le cartelle `Source` e `.github`.
   - Con il sito: "Add file" > "Upload files" e trascina tutto. Attenzione che la cartella nascosta `.github` venga caricata, altrimenti la compilazione non parte.
4. Vai nella scheda **Actions**: la compilazione parte da sola (circa 5-10 minuti).
   - Se non parte, apri "Build Montage (macOS)" e premi **Run workflow**.
5. Quando e' verde, apri l'esecuzione e scarica **Montage-macOS-DMG** in fondo alla pagina.
6. Decomprimi lo zip scaricato: dentro c'e' `Montage.dmg`.
7. Apri il DMG e segui `INSTALLA.txt`.

Se la compilazione fallisce (puo' succedere al primo giro, perche' il codice non e' stato provato su un compilatore): apri l'esecuzione rossa, copia il testo dell'errore e mandalo a Claude, che lo corregge.

## Come si usa

- **Sample**: trascinalo nella finestra o premi LOAD SAMPLE (WAV, AIFF, FLAC, MP3, OGG, max 60 secondi). Il sample resta associato al progetto Ableton.
- **MIDI**: ogni nota suona il sample a un'altezza diversa (Do centrale = altezza originale), fino a 8 note insieme.

### Controlli

| Controllo | Cosa fa |
|---|---|
| Position | punto del sample da cui pescano i grani (linea arancione) |
| Scatter | quanto i grani si sparpagliano intorno alla posizione (banda arancione) |
| Grain Size | durata di ogni grano |
| Density | quanti grani al secondo |
| Pitch / Pitch Spread | intonazione e variazione casuale dell'intonazione |
| Reverse | probabilita' che un grano suoni al contrario |
| Stereo Spread | quanto i grani si distribuiscono tra sinistra e destra |
| Attack / Release | inviluppo di ogni nota |
| Filter / Resonance | filtro passa-basso |
| Volume (LEVEL) | volume generale |
| Rate | velocita' degli stacchi di montaggio (OFF = spento) |
| Steps | in quanti punti viene divisa la zona degli stacchi |
| Chaos | probabilita' che lo stacco vada a un punto a caso invece che al successivo |
| Span | quanto in la' nel sample arrivano gli stacchi |

### CUTS: gli stacchi di montaggio

E' la funzione che rende Montage diverso da un granulatore normale. Con **Rate** acceso, il punto da cui pescano i grani non resta fermo: salta come in un montaggio cinematografico, da uno stacco al successivo. I punti si vedono sulla pellicola come linee verdi con un triangolino, e quello attivo si illumina. In alto a destra legge `CUT 03/08`.

- **Rate** decide quanti stacchi al secondo.
- **Steps** dice quanti punti ci sono, distribuiti dalla Position in avanti.
- **Span** decide quanto sono distanti tra loro.
- **Chaos** a 0% fa andare gli stacchi in ordine; piu' lo alzi, piu' saltano a caso.

I preset delle famiglie Glitch, Rhythmic ed Experimental hanno gli stacchi gia' accesi.

### Interfaccia

- La forma d'onda e' una **pellicola** con perforazioni e fotogrammi.
- Ogni grano che parte compare come una **luce** sopra l'onda: arancione se suona in avanti, verde se suona al contrario. L'altezza della luce indica la posizione stereo.
- Clicca o trascina sulla pellicola per spostare la Position.
- Il pallino **LIVE** si accende quando suoni una nota.

### Varianti del suono

- **MUTATE**: ricalcola in modo drastico posizione, dimensione, densita', altezza, reverse e stereo. Ogni clic e' una nuova versione del suono.
- **VARIATION**: spostamento leggero dei valori attuali, per esplorare dintorni del suono che ti piace.

### Preset

Menu in alto a destra, con frecce per scorrere: 100 preset di fabbrica numerati, divisi in 10 famiglie da 10 (Clouds & Pads, Glitch & Stutter, Reverse & Dreams, Dust & Texture, Slow Motion & Freeze, Rhythmic, Lo-Fi & Tape, Bass & Low, Bright & Shimmer, Experimental). L'elenco completo e' in `PRESETS.md`. Compaiono anche come programmi nella lista preset di Ableton.

## Struttura del progetto

```
CMakeLists.txt              configurazione (scarica JUCE da sola)
Source/PluginProcessor.*    motore granulare, parametri, preset, MIDI
Source/PluginEditor.*       interfaccia grafica
.github/workflows/build.yml compilazione e DMG su GitHub
INSTALLA.txt                istruzioni incluse nel DMG
```

## Nota sulla firma

Il DMG e' firmato solo "ad-hoc", sufficiente per usarlo sul tuo Mac con il passaggio `xattr -cr` descritto in INSTALLA.txt. Per distribuirlo ad altri senza avvisi serve un Apple Developer ID (99 $/anno) e la notarizzazione.
