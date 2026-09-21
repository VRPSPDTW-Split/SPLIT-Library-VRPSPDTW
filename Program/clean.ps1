$TARGETDIR = "."
$files = @(
    "commandline.o",
    "main.o",
    "main_correctness.o",
    "BenchmarkInstanceReader.o",
    "Pb_Data.o",
    "Split_Bellman_Soft_CVRP.o",
    "Split_Bellman_VRPSPDTW.o",
    "Split_Bellman_Soft_VRPSPD.o",
    "Split_Bellman_Soft_VRPTW.o",
	"Split_Bellman_Soft_VRPSPDTW.o",
	"Split_Linear.o",
	"Split_Linear_CVRP.o",
	"Split_Linear_VRPSPD.o",
	"Split_Linear_VRPTW.o",
    "Split_Linear_VRPSPDTW.o",
    "Split_Linear_Soft_VRPSPD.o",
    "Split_Linear_Soft_VRPSPDTW.o",
    "Split_Linear_Soft_CVRP.o",
    "Split_Linear_Soft_VRPTW.o",
    "split.exe",
    "split_correctness.exe"
)

foreach ($f in $files) {
    $path = Join-Path $TARGETDIR $f
    if (Test-Path $path) {
        Remove-Item $path -Force
        Write-Host "Removed $path"
    }
}
