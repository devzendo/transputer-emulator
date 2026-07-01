# Run the IServer and Emulator to run a standalone binary.

param(
    [Parameter(ValueFromRemainingArguments)]
    [string[]]$Args
)

# Store command line arguments for future parsing
$storedArgs = $Args

# Launch the emulator in the background and capture its PID.
$tProc = Start-Process -FilePath "temulate.exe" -ArgumentList "-le -x" -PassThru -NoNewWindow
$tPid  = $tProc.Id

# Run IServer with the stored arguments, passing through all stdout/stderr.
& iserver.exe -le @storedArgs
$iExit = $LASTEXITCODE

# Retrieve the emulator's exit code.
if (-not $tProc.HasExited) {
    $tProc.Kill()
}
$tProc.WaitForExit()
$tExit = $tProc.ExitCode

# If the emulator exited with an error, notify and exit with its code.
if ($tExit -ne 0) {
    Write-Error "'temulate.exe' exited with code $tExit"
    exit $tExit
}

# Otherwise exit with the IServer's exit code.
exit $iExit
