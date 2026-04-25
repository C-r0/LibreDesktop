package main

import (
	"fmt"
	"os"
	"io"
	"strings"
)

func main() {
	if len(os.Args) > 1 {
		if os.Args[1] == "--help" {
			fmt.Print(`--help - Show this message
filename - Display the content of the filename
-n - Display the text with number of lines
file1 > file2 - Copy content of file1 to file2
file1 >> file2 - Append content of file1 to file2
`)
			return
		} else if os.Args[1] == "-n" {
			if len(os.Args) == 3 {
				data, err := os.ReadFile(os.Args[2])
				if err != nil {
					fmt.Println(err)
					return
				}
				linhas := strings.Split(string(data), "\n")
				for i, linha := range linhas {
					fmt.Printf("%d - %s\n", i+1, linha)
				}
			} else {
				fmt.Println("Invalid number of args libr -n file")
			}
		} else {
			data, err := os.ReadFile(os.Args[1])
			if err != nil {
				fmt.Println(err)
			}
			fmt.Print(string(data))
		}
	} else {
		_, err := io.Copy(os.Stdout, os.Stdin)
		if err != nil {
			fmt.Println(err)
		}
		return
	}
}
