package internal

type PageFaultError struct {
	msg string
}

func (e *PageFaultError) Error() string {
	return "Page fault"
}
