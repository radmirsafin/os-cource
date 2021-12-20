package internal

import "fmt"

type Memory struct {
	data map[uint64]uint64
}

func NewMemory() *Memory {
	memory := Memory{
		data: make(map[uint64]uint64),
	}
	return &memory
}

func (memory *Memory) Write(address uint64, value uint64) {
	memory.data[address] = value
}

func (memory *Memory) Read(address uint64) uint64 {
	if value, ok := memory.data[address]; ok {
		return value
	} else {
		return 0
	}
}

func (memory *Memory) Show() {
	for key, value := range memory.data {
		fmt.Printf("Address: %8d Value: %064b (%d) \n", key, value, value)
	}
}
