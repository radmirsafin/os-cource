package internal

type Translator struct {
	memory           *Memory
	rootTableAddress uint64
}

func NewTranslator(memory *Memory, rootTableAddress uint64) *Translator {
	translator := Translator{
		memory:           memory,
		rootTableAddress: rootTableAddress,
	}
	return &translator
}

func (translator *Translator) TranslateLogicalToPhysical(logicalAddress uint64) (uint64, error) {
	nextTablePointer := translator.rootTableAddress
	// we have 4 table levels (x86 Long Mode Paging)
	for i := 0; i < 4; i++ {
		currLevelIndex := logicalAddress

		// clean unused high-order bits
		currLevelIndex = currLevelIndex << (9*i + 16)
		currLevelIndex = currLevelIndex >> (9*i + 16)

		currLevelIndex = currLevelIndex >> (9*(3-i) + 12)

		tableEntryAddress := nextTablePointer + (currLevelIndex * 8)
		tableEntry := translator.memory.Read(tableEntryAddress)

		if tableEntry%2 == 0 { // check P bit
			return 0, &PageFaultError{}
		}

		// get physical address from range [51:12]
		nextTablePointer = tableEntry & 0xFFFFFFFFFF000
	}
	offset := logicalAddress & 0xFFF
	return nextTablePointer + offset, nil
}
