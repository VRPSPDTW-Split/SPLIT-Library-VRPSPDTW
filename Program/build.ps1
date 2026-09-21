# Set compilation flags
$CC = "g++"
$CCFLAGS = @("-O3", "-std=c++11")
$CPPFLAGS = "-I."
$TARGETDIR = "."

# All source files
$sourceFiles = @(
    "commandline.cpp",
    "main.cpp",
    "Pb_Data.cpp",
    "Split_Bellman_Soft_CVRP.cpp",
    "Split_Bellman_VRPSPDTW.cpp",
    "Split_Bellman_Soft_VRPSPD.cpp",
    "Split_Bellman_Soft_VRPTW.cpp",
    "Split_Bellman_Soft_VRPSPDTW.cpp",
    "Split_Linear.cpp",
    "Split_Linear_Soft_CVRP.cpp",
    "Split_Linear_VRPSPDTW.cpp",
    "Split_Linear_VRPSPD.cpp",
    "Split_Linear_VRPTW.cpp",
    "Split_Linear_Soft_VRPSPD.cpp",
    "Split_Linear_Soft_VRPSPDTW.cpp",
    "Split_Linear_Soft_VRPTW.cpp",
    "Split_Linear_CVRP.cpp"
)

# Create corresponding object files
$objFiles = $sourceFiles | ForEach-Object {
    $base = [System.IO.Path]::GetFileNameWithoutExtension($_)
    "$TARGETDIR/$base.o"
}

# Compile all source files to object files
foreach ($src in $sourceFiles) {
    $obj = "$TARGETDIR/" + [System.IO.Path]::GetFileNameWithoutExtension($src) + ".o"
    Write-Host "Compiling $src..."
    & $CC $CCFLAGS $CPPFLAGS -c $src -o $obj
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Compilation failed for $src"
        exit 1
    }
}

# Link object files into the final binary
Write-Host "Linking object files..."
& $CC $CCFLAGS -o "$TARGETDIR/split" $objFiles
if ($LASTEXITCODE -ne 0) {
    Write-Error "Linking failed"
    exit 1
}

Write-Host "Build successful: split"

# Build the separate correctness driver. It reuses the solver objects above,
# but has its own main and native benchmark reader.
$correctnessSourceFiles = @(
    "main_correctness.cpp",
    "../Benchmark-Instances/BenchmarkInstanceReader.cpp"
)

foreach ($src in $correctnessSourceFiles) {
    $obj = "$TARGETDIR/" + [System.IO.Path]::GetFileNameWithoutExtension($src) + ".o"
    Write-Host "Compiling $src..."
    & $CC $CCFLAGS $CPPFLAGS -c $src -o $obj
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Compilation failed for $src"
        exit 1
    }
}

$correctnessObjFiles = @(
    "$TARGETDIR/main_correctness.o",
    "$TARGETDIR/BenchmarkInstanceReader.o",
    "$TARGETDIR/Pb_Data.o",
    "$TARGETDIR/Split_Bellman_Soft_CVRP.o",
    "$TARGETDIR/Split_Bellman_VRPSPDTW.o",
    "$TARGETDIR/Split_Bellman_Soft_VRPSPD.o",
    "$TARGETDIR/Split_Bellman_Soft_VRPTW.o",
    "$TARGETDIR/Split_Bellman_Soft_VRPSPDTW.o",
    "$TARGETDIR/Split_Linear_CVRP.o",
    "$TARGETDIR/Split_Linear_Soft_CVRP.o",
    "$TARGETDIR/Split_Linear_VRPSPD.o",
    "$TARGETDIR/Split_Linear_VRPTW.o",
    "$TARGETDIR/Split_Linear_VRPSPDTW.o",
    "$TARGETDIR/Split_Linear_Soft_VRPSPD.o",
    "$TARGETDIR/Split_Linear_Soft_VRPSPDTW.o",
    "$TARGETDIR/Split_Linear_Soft_VRPTW.o"
)

Write-Host "Linking correctness checker..."
& $CC $CCFLAGS -o "$TARGETDIR/split_correctness" $correctnessObjFiles
if ($LASTEXITCODE -ne 0) {
    Write-Error "Linking split_correctness failed"
    exit 1
}

Write-Host "Build successful: split_correctness"
