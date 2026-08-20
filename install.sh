#!/bin/bash

# Interrompe em caso de erro
set -e

APP_NAME="fydelisdisksuite"
VERSION="1.0.0"
PACKAGE_NAME="fydelis-disk-suite"
BUILD_DIR="build"
PKG_DIR="pkg_temp"

echo "=================================================="
echo " [1/5] Verificando e instalando dependências..."
echo "=================================================="
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    qt6-base-dev \
    qt6-tools-dev \
    dpkg-dev \
    parted \
    smartmontools \
    foremost \
    rsync

echo "=================================================="
echo " [2/5] Compilando o projeto (Release)..."
echo "=================================================="
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
cd ..

echo "=================================================="
echo " [3/5] Criando estrutura do pacote .deb..."
echo "=================================================="
rm -rf "$PKG_DIR"
mkdir -p "$PKG_DIR/DEBIAN"
mkdir -p "$PKG_DIR/usr/bin"
mkdir -p "$PKG_DIR/usr/share/applications"
mkdir -p "$PKG_DIR/usr/share/icons/hicolor/scalable/apps"

# Copia o binário compilado
cp "$BUILD_DIR/FydelisDiskSuite" "$PKG_DIR/usr/bin/$APP_NAME"
chmod +x "$PKG_DIR/usr/bin/$APP_NAME"

# Criação do arquivo de controle do pacote .deb
cat << EOF > "$PKG_DIR/DEBIAN/control"
Package: $PACKAGE_NAME
Version: $VERSION
Section: utils
Priority: optional
Architecture: amd64
Maintainer: FydelisTech <contato@fydelistech.com>
Depends: libc6, libgcc-s1, libstdc++6, qt6-base-dev, parted, smartmontools, foremost
Description: FydelisDisk Suite - Gerenciador avançado de discos e partições
 Utilitário completo para formatação, clonagem, recuperação de dados
 e criação de mídias USB para o FydelisTechOS.
EOF

# Criação de um ícone vetorial moderno e elegante (SVG)
cat << 'EOF' > "$PKG_DIR/usr/share/icons/hicolor/scalable/apps/fydelisdisksuite.svg"
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 128 128" width="128" height="128">
  <defs>
    <linearGradient id="bgGrad" x1="0%" y1="0%" x2="100%" y2="100%">
      <stop offset="0%" stop-color="#161623"/>
      <stop offset="100%" stop-color="#0f0f1a"/>
    </linearGradient>
    <linearGradient id="diskGrad" x1="0%" y1="0%" x2="0%" y2="100%">
      <stop offset="0%" stop-color="#00b4d8"/>
      <stop offset="100%" stop-color="#0077b6"/>
    </linearGradient>
    <linearGradient id="glowGrad" x1="0%" y1="0%" x2="100%" y2="0%">
      <stop offset="0%" stop-color="#06d6a0"/>
      <stop offset="100%" stop-color="#00b4d8"/>
    </linearGradient>
  </defs>
  <!-- Fundo com cantos arredondados -->
  <rect x="8" y="8" width="112" height="112" rx="24" fill="url(#bgGrad)" stroke="#2a2a3f" stroke-width="3"/>
  
  <!-- Disco Externo / Base -->
  <circle cx="64" cy="64" r="40" fill="none" stroke="#2a2a3f" stroke-width="8"/>
  
  <!-- Anel de Progresso / Atividade -->
  <circle cx="64" cy="64" r="40" fill="none" stroke="url(#glowGrad)" stroke-width="8" stroke-dasharray="180 70" stroke-linecap="round" transform="rotate(-90 64 64)"/>
  
  <!-- Disco Interno (Spindle/Capa) -->
  <circle cx="64" cy="64" r="16" fill="url(#diskGrad)"/>
  <circle cx="64" cy="64" r="6" fill="#0f0f1a"/>
  
  <!-- Indicador LED de status -->
  <circle cx="92" cy="36" r="5" fill="#06d6a0"/>
</svg>
EOF

# Atalho do Menu de Aplicativos (.desktop)
cat << EOF > "$PKG_DIR/usr/share/applications/$APP_NAME.desktop"
[Desktop Entry]
Name=FydelisDisk Suite
Comment=Gerenciador e Utilitários de Disco
Exec=/usr/bin/$APP_NAME
Icon=fydelisdisksuite
Terminal=false
Type=Application
Categories=System;DiskManagement;Utility;
Keywords=disk;partition;formatter;clone;usb;
EOF

echo "=================================================="
echo " [4/5] Gerando o pacote .deb..."
echo "=================================================="
dpkg-deb --build "$PKG_DIR" "${PACKAGE_NAME}_${VERSION}_amd64.deb"

echo "=================================================="
echo " [5/5] Instalando o pacote localmente..."
echo "=================================================="
sudo dpkg -i "${PACKAGE_NAME}_${VERSION}_amd64.deb"
sudo apt-get install -f -y

echo "=================================================="
echo " Concluído com sucesso! O FydelisDisk Suite está pronto."
echo "=================================================="
