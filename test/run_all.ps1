param(
    [ValidateSet("lexer", "parser", "semantic", "execute", "negative", "dump", "m4", "regression", "all")]
    [string]$Suite = "all",
    [switch]$SkipBuild
)

$ErrorActionPreference = "Continue"
$Root = Split-Path -Parent $PSScriptRoot
$Arion = Join-Path $Root "build\arion.exe"

if (-not $SkipBuild) {
    Push-Location $Root
    cmake --build build | Out-Host
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    Pop-Location
}

if (-not (Test-Path $Arion)) {
    Write-Error "arion executable not found at $Arion"
}

$passed = 0
$failed = 0

function Normalize-Text([string]$Text) {
    return ($Text -replace "`r`n", "`n").Trim()
}

function Extract-ExecutionOutput([string]$Stdout) {
    $marker = "=== EXECUTION OUTPUT ==="
    $idx = $Stdout.IndexOf($marker)
    if ($idx -lt 0) { return "" }
    return $Stdout.Substring($idx + $marker.Length).Trim()
}

function Run-TestCase {
    param(
        [string]$Name,
        [string[]]$ArionArgs,
        [string]$ExpectedFile = "",
        [ValidateSet("stdout", "stderr", "exit", "no_execution")]
        [string]$Mode
    )

    if ($null -eq $ArionArgs -or $ArionArgs.Count -eq 0) {
        Write-Host "FAIL $Name (empty args)"
        $script:failed++
        return
    }
    $stdout = & $Arion @ArionArgs 2>&1 | ForEach-Object { "$_" } | Out-String
    $exit = $LASTEXITCODE

    $ok = $false
    switch ($Mode) {
        "stdout" {
            $actual = Normalize-Text (Extract-ExecutionOutput $stdout)
            $expected = Normalize-Text (Get-Content $ExpectedFile -Raw)
            $ok = ($exit -eq 0) -and ($actual -eq $expected)
        }
        "stderr" {
            $expected = Normalize-Text (Get-Content $ExpectedFile -Raw)
            $ok = ($exit -ne 0) -and ($stdout -match [regex]::Escape($expected.Trim()))
        }
        "exit" {
            $ok = ($exit -eq 0) -and ($stdout -notmatch "semantic error")
        }
        "no_execution" {
            $ok = ($exit -eq 0) -and ($stdout -notmatch "=== EXECUTION OUTPUT ===")
        }
    }

    if ($ok) {
        Write-Host "PASS $Name"
        $script:passed++
    } else {
        Write-Host "FAIL $Name (exit=$exit)"
        $script:failed++
    }
}

function Run-LexerSuite {
    $dir = Join-Path $Root "test\milestone-1\lexer-test-data"
    Get-ChildItem $dir -Filter "*.in.txt" | Sort-Object Name | ForEach-Object {
        $base = $_.FullName -replace '\.in\.txt$', ''
        $expected = "$base.expected.txt"
        if (-not (Test-Path $expected)) { return }
        $tokenOut = Join-Path $env:TEMP ("arion_lex_" + $_.BaseName + ".token")
        if (Test-Path $tokenOut) { Remove-Item $tokenOut -Force }
        & $Arion --lexer $_.FullName | Out-Null
        $generated = $_.FullName + ".token"
        if (Test-Path $generated) {
            $a = Normalize-Text (Get-Content $generated -Raw)
            $e = Normalize-Text (Get-Content $expected -Raw)
            if ($a -eq $e) {
                Write-Host "PASS lexer $($_.Name)"
                $script:passed++
            } else {
                Write-Host "FAIL lexer $($_.Name)"
                $script:failed++
            }
        } else {
            Write-Host "FAIL lexer $($_.Name) (no token output)"
            $script:failed++
        }
    }
}

function Run-ParserSuite {
    $dir = Join-Path $Root "test\milestone-2\parser-test-data"
    Get-ChildItem $dir -Filter "*.in.txt" | Sort-Object Name | ForEach-Object {
        $base = $_.FullName -replace '\.in\.txt$', ''
        $expected = "$base.expected.txt"
        if (-not (Test-Path $expected)) { return }
        $stdout = & $Arion --parser --dump $_.FullName 2>&1 | Out-String
        $start = $stdout.IndexOf("parse tree:")
        if ($start -lt 0) { $start = 0 }
        $actual = Normalize-Text $stdout.Substring($start)
        $expectedText = Normalize-Text (Get-Content $expected -Raw)
        if (($LASTEXITCODE -eq 0) -and ($actual -eq $expectedText)) {
            Write-Host "PASS parser $($_.Name)"
            $script:passed++
        } else {
            Write-Host "FAIL parser $($_.Name)"
            $script:failed++
        }
    }
}

function Run-SemanticSuite {
    $dir = Join-Path $Root "test\milestone-3\semantic-test"
    Get-ChildItem $dir -Filter "*.in.txt" | Sort-Object Name | ForEach-Object {
        Run-TestCase -Name "semantic $($_.Name)" -ArionArgs @("--dump", $_.FullName) -Mode "exit"
    }
}

function Source-From-Expected([string]$ExpectedPath, [string]$Suffix) {
    if (-not $ExpectedPath.EndsWith($Suffix)) { return $null }
    return $ExpectedPath.Substring(0, $ExpectedPath.Length - $Suffix.Length)
}

function Run-ExecuteSuite {
    $dir = Join-Path $Root "test\milestone-4"
    Get-ChildItem $dir -Filter "*.expected.out" | Sort-Object Name | ForEach-Object {
        $source = Source-From-Expected $_.FullName ".expected.out"
        if ($null -eq $source -or -not (Test-Path $source)) { return }
        Run-TestCase -Name "execute $(Split-Path $source -Leaf)" -ArionArgs @($source) -ExpectedFile $_.FullName -Mode "stdout"
    }
}

function Run-NegativeSuite {
    $dir = Join-Path $Root "test\milestone-4"
    Get-ChildItem $dir -Filter "*.expected.err" | Sort-Object Name | ForEach-Object {
        $source = Source-From-Expected $_.FullName ".expected.err"
        if ($null -eq $source -or -not (Test-Path $source)) { return }
        Run-TestCase -Name "negative $(Split-Path $source -Leaf)" -ArionArgs @($source) -ExpectedFile $_.FullName -Mode "stderr"
    }
}

function Run-DumpSuite {
    $sample = Join-Path $Root "test\milestone-4\text_keyword.txt"
    Run-TestCase -Name "dump text_keyword" -ArionArgs @("--dump", $sample) -Mode "no_execution"
}

switch ($Suite) {
    "lexer" { Run-LexerSuite }
    "parser" { Run-ParserSuite }
    "semantic" { Run-SemanticSuite }
    "execute" { Run-ExecuteSuite }
    "negative" { Run-NegativeSuite }
    "dump" { Run-DumpSuite }
    "m4" {
        Run-SemanticSuite
        Run-ExecuteSuite
        Run-NegativeSuite
        Run-DumpSuite
    }
    "regression" {
        Run-LexerSuite
        Run-ParserSuite
    }
    "all" {
        Run-SemanticSuite
        Run-ExecuteSuite
        Run-NegativeSuite
        Run-DumpSuite
    }
}

Write-Host ""
Write-Host "Summary: $passed passed, $failed failed"
if ($failed -gt 0) { exit 1 }
exit 0
