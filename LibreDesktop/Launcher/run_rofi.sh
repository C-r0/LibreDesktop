#!/bin/bash
# Força o locale para evitar o erro que vimos antes
export LANG=pt_BR.UTF-8
export LC_ALL=pt_BR.UTF-8

# Roda o rofi em background e desvincula totalmente
setsid rofi -show drun &
disown
