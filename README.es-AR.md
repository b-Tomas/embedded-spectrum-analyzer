# Analizador de Espectro en Tiempo Real con LPC1769 (Cortex-M3)

[![en documentation](https://img.shields.io/badge/lang-en-orange.svg)](README.md)

# Quickstart

## Requisitos

- [MCUXpresso IDE](https://www.nxp.com/design/design-center/software/development-software/mcuxpresso-software-and-tools-/mcuxpresso-integrated-development-environment-ide:MCUXpresso-IDE)
- Git

## Clonar el repositorio

```bash
git clone --recursive https://github.com/b-Tomas/embedded-spectrum-analyzer.git
cd embedded-spectrum-analyzer
```

O si ya se clonó sin `--recursive`, inicializar los submódulos manualmente:

```bash
git submodule update --init --recursive
```

## Workaround para Linux: symlink de headers

La biblioteca CMSIS usa `#include "LPC17xx.h"` pero algunos drivers incluyen `"lpc17xx.h"` (minúsculas). En sistemas case-sensitive como Linux, esto falla. Crear un symlink:

```bash
ln -s LPC17xx.h lib/CMSISv2p00_LPC17xx/CMSISv2p00_LPC17xx/inc/lpc17xx.h
```

## Importar proyectos en MCUXpresso

1. **Importar el proyecto:** File → Import → General → Existing Projects into Workspace → seleccionar el directorio `firmware/` → Finish
2. **Importar la biblioteca CMSIS:** File → Import → General → Existing Projects into Workspace → seleccionar `lib/CMSISv2p00_LPC17xx/CMSISv2p00_LPC17xx/` → Finish

Deberían aparecer dos proyectos en el workspace: `spectrum-analyzer` y `CMSISv2p00_LPC17xx`.

## Configurar referencias del proyecto

1. Click derecho en `spectrum-analyzer` → Properties → Project References
2. Marcar la casilla `CMSISv2p00_LPC17xx`
3. Aplicar y cerrar

Esto asegura que la biblioteca se compile antes que el firmware.

## Compilar

1. Click derecho en `CMSISv2p00_LPC17xx` → Build Project
2. Click derecho en `spectrum-analyzer` → Build Project

Si la referencia del proyecto está configurada correctamente, compilar `spectrum-analyzer` debería ser suficiente, compilando la biblioteca automáticamente.
