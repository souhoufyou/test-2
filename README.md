
# AI Vocal Assistant **EZ** (VST3)

Version **zéro réglage** pour voix : Denoiser → HPF → De-Esser → Compressor → Saturation → Limiter → Auto-Gain.
- Auto-analyse au démarrage, aucun paramètre requis (UI = Bypass).
- **Denoiser adaptatif** (type spectral gating) intégré, temps réel.

## Build rapide (Windows via GitHub Actions)
1. Uploadez ce dossier dans un repo GitHub vide.
2. Allez dans **Actions** → lancez **Build VST3**.
3. Téléchargez l’artefact `AIVocalAssistantEZ_vst3.zip` → contient `AI Vocal Assistant EZ.vst3`.

## Installation (FL Studio)
Copiez `AI Vocal Assistant EZ.vst3` dans `C:\Program Files\Common Files\VST3\`, puis **Find plugins** dans FL Studio.
