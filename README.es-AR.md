# Analizador de Espectro en Tiempo Real con LPC1769 (Cortex-M3)

[![en documentation](https://img.shields.io/badge/lang-en-orange.svg)](README.md)

# Quickstart

## Requisitos

- [MCUXpresso IDE](https://www.nxp.com/design/design-center/software/development-software/mcuxpresso-software-and-tools-/mcuxpresso-integrated-development-environment-ide:MCUXpresso-IDE)
  - [Instalación en Arch Linux](https://gist.github.com/b-Tomas/0020459896914a7bc4183d71dc9441dd)
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

Click en *File* → *Import* → *General* → *Existing Projects into Workspace* → seleccionar la raiz del repositorio → Marcar la casilla *Search for nested projects* y verificar que tanto `firmware/` como `lib/CMSISv2p00_LPC17xx/CMSISv2p00_LPC17xx/` estén marcados → *Finish*

Deberían aparecer dos proyectos en el workspace: `spectrum-analyzer` y `CMSISv2p00_LPC17xx`.

## Configurar referencias del proyecto

1. Click derecho en `spectrum-analyzer` → *Properties* → *Project References*
2. Asegurar que esté marcada la casilla `CMSISv2p00_LPC17xx`
3. Aplicar y cerrar

Esto asegura que la biblioteca se compile antes que el firmware.

## Compilar

Si la referencia del proyecto está configurada correctamente, compilar `spectrum-analyzer` debería ser suficiente.

Caso contrario, puede ser necesario compilar `CMSISv2p00_LPC17xx` primero, y luego `spectrum-analyzer`:
1. Click derecho en `CMSISv2p00_LPC17xx` → Build Project
2. Click derecho en `spectrum-analyzer` → Build Project
