package internal

import (
    "fmt"
    "strconv"
)

type LogicalAddress struct {
    addressUint64    uint64
    addressBinaryStr string
}

func (address *LogicalAddress) Show() {
    fmt.Printf("Logical Address { addressUint64: %8d, addressBinaryStr: %s }\n",
        address.addressUint64, address.addressBinaryStr)
}

func (address *LogicalAddress) GetRootTableIndex(level int) uint64 {
    switch level {
    case 0:
        index, _ := strconv.ParseUint(address.addressBinaryStr[16:25], 2, 64)
        fmt.Printf("Level 0 index:  %s (%d)\n", address.addressBinaryStr[16:25], index)
        return index
    case 1:
        index, _ := strconv.ParseUint(address.addressBinaryStr[25:34], 2, 64)
        fmt.Printf("Level 1 index:  %s (%d)\n", address.addressBinaryStr[25:34], index)
        return index
    case 2:
        index, _ := strconv.ParseUint(address.addressBinaryStr[34:43], 2, 64)
        fmt.Printf("Level 2 index:  %s (%d)\n", address.addressBinaryStr[34:43], index)
        return index
    case 3:
        index, _ := strconv.ParseUint(address.addressBinaryStr[43:52], 2, 64)
        fmt.Printf("Level 3 index:  %s (%d)\n", address.addressBinaryStr[43:52], index)
        return index
    default:
        return 1
    }
}

func NewLogicalAddress(addressUint64 uint64) *LogicalAddress {
    address := LogicalAddress{
        addressUint64:    addressUint64,
        addressBinaryStr: fmt.Sprintf("%064b", addressUint64),
    }
    return &address
}
