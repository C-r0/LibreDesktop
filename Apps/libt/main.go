package main

import (
	"fmt"
	"os"
	"golang.org/x/term"
	"strings"
)

func main() {
	if len(os.Args) > 1 {
		// Modo RAW		
		fd := int(os.Stdin.Fd())
		oldState, _ := term.MakeRaw(fd)
		defer term.Restore(fd, oldState)

		// Abre Arquivo do arg[1]
		data, err := os.ReadFile(os.Args[1])
		if err != nil {
			fmt.Print(err)
		}
		fmt.Print("\033[2J\033[H")
		fmt.Printf("\033[7m LibreText v0.1 - Editando: %s \033[m\r\n", os.Args[1])
		fmt.Print(strings.ReplaceAll(string(data), "\n", "\r\n"))

		// Parser
		b := make([]byte, 3) // Max 3 bytes para setas
		for {
			n, _ := os.Stdin.Read(b)

			if n == 1 {
				if b[0] == 3 { break } // ctrl c
				fmt.Printf("Tecla: %s ASCII: %d\r\n", string(b[0]), b[0])
			} else if n == 3 && b[0] == 27 && b[1] == 91 {
				switch b[2] {
					case 65: fmt.Print("Seta Cima \r\n")
					case 66: fmt.Print("Seta Baixo \r\n")
					case 67: fmt.Print("Seta Direita \r\n")
					case 68: fmt.Print("Seta Esquerda\r\n")
				}
			}
		}

	} else {
		fmt.Println("Uso: libt filename")
	}
}
