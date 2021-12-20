package main

import (
	"./internal"
	"bufio"
	"flag"
	"fmt"
	"os"
	"strconv"
	"strings"
)

func main() {
	inputFile := flag.String("fpath", "pages.txt", "File path to read from")
	flag.Parse()

	file, err := os.Open(*inputFile)
	if err != nil {
		fmt.Printf("Can't read file: %s", err)
		os.Exit(1)
	}
	defer file.Close()

	scanner := bufio.NewScanner(file)

	scanner.Scan()
	parameters := strings.Split(scanner.Text(), " ")
	memoryMapSize, err := strconv.ParseUint(parameters[0], 10, 64)
	requestCount, err := strconv.ParseUint(parameters[1], 10, 64)
	rootTableAddress, err := strconv.ParseUint(parameters[2], 10, 64)
	if err != nil {
		fmt.Printf("Can't read parameters from file: %s", err)
		os.Exit(1)
	}

	fmt.Printf("Memory map size: %d\n", memoryMapSize)
	fmt.Printf("Requests to handle: %d\n", requestCount)
	fmt.Printf("Root table address: %d\n", rootTableAddress)

	memory := internal.NewMemory()
	for i := uint64(0); i < memoryMapSize; i++ {
		scanner.Scan()
		row := strings.Split(scanner.Text(), " ")
		address, _ := strconv.ParseUint(row[0], 10, 64)
		value, _ := strconv.ParseUint(row[1], 10, 64)
		memory.Write(address, value)
	}

	translator := internal.NewTranslator(memory, rootTableAddress)
	for i := uint64(0); i < requestCount; i++ {
		scanner.Scan()
		logicalAddress, _ := strconv.ParseUint(scanner.Text(), 10, 64)
		physicalAddress, err := translator.TranslateLogicalToPhysical(logicalAddress)
		if err != nil {
			fmt.Println("fault")
		} else {
			fmt.Println(physicalAddress)
		}
	}
}
