package main

import (
	"fmt"

	"github.com/shirou/gopsutil/cpu"
	"github.com/shirou/gopsutil/disk"
	"github.com/shirou/gopsutil/host"
	"github.com/shirou/gopsutil/mem"
)

func main() {
	infor,_ := cpu.Info()
	diskinfor,_ := disk.Usage("/")
	hostinfor,_ := host.Info()
	meminfor,_ := mem.SwapDevices();

	fmt.Print(infor[0].String()+"\n\n")
	fmt.Print(diskinfor.String()+"\n\n")
	fmt.Print(meminfor[0].String()+"\n\n")
	fmt.Print(hostinfor.String()+"\n\n")
}
