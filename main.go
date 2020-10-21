package main

import (
	"bufio"
	"flag"
	"fmt"
	"github.com/radmirsafin/page-translator/internal"
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
	memoryMapSize, err := strconv.ParseInt(parameters[0], 10, 64)
	requestCount, err := strconv.ParseInt(parameters[1], 10, 64)
	rootTableAddress, err := strconv.ParseInt(parameters[2], 10, 64)
	if err != nil {
		fmt.Printf("Can't read parameters from file: %s", err)
		os.Exit(1)
	}

	fmt.Printf("Memory map size: %d, Requests to handle: %d, Root table address: %d\n",
		memoryMapSize, requestCount, rootTableAddress)

	memory := internal.NewMemory()

	for i := int64(0); i < memoryMapSize; i++ {
		scanner.Scan()
		rec := strings.Split(scanner.Text(), " ")
		address, _ := strconv.ParseUint(rec[0], 10, 64)
		value, _ := strconv.ParseUint(rec[1], 10, 64)
		memory.Write(address, value)
	}

	memory.Show()

	for i := int64(0); i < requestCount; i++ {
		scanner.Scan()
		addressUint64, _ := strconv.ParseUint(scanner.Text(), 10, 64)
		logicalAddress := internal.NewLogicalAddress(addressUint64)
		logicalAddress.Show()
		logicalAddress.GetRootTableIndex(0)
		logicalAddress.GetRootTableIndex(1)
		logicalAddress.GetRootTableIndex(2)
		logicalAddress.GetRootTableIndex(3)
	}

}
