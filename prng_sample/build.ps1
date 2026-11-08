param([switch]$Test, [switch]$TestSafe512)
$ErrorActionPreference = 'Stop'
Push-Location $PSScriptRoot
try {
    Get-Command g++ -ErrorAction Stop | Out-Null
    New-Item -ItemType Directory -Force bin | Out-Null
    $common = @('src/u512_sample.cpp', 'src/dh_sample.cpp', 'src/dh_exchange.cpp', 'src/private_key.cpp', 'lib/fips202/prng.cpp', 'lib/fips202/fips202.c')
    $targets = @(
        @{Name='prng'; Sources=@('src/main_prng.cpp', 'lib/fips202/prng.cpp', 'lib/fips202/fips202.c')},
        @{Name='dh'; Sources=@('src/main_dh.cpp', 'src/prime_sample.cpp') + $common},
        @{Name='test-u512'; Sources=@('src/test_u512.cpp') + $common},
        @{Name='test-private-key'; Sources=@('src/test_private_key.cpp') + $common},
        @{Name='test-prime'; Sources=@('src/test_prime.cpp', 'src/prime_sample.cpp') + $common},
        @{Name='test-private-key-extended'; Sources=@('src/test_private_key_extended.cpp') + $common},
        @{Name='test-prng'; Sources=@('tests/prng_test.cpp', 'lib/fips202/prng.cpp', 'lib/fips202/fips202.c')},
        @{Name='test-dh-exchange'; Sources=@('src/test_dh_exchange.cpp') + $common},
        @{Name='test-regressions'; Sources=@('src/test_regressions.cpp') + $common}
    )
    foreach ($target in $targets) {
        Write-Host "Building $($target.Name)"
        & g++ -std=c++14 -O2 -Wall -Wextra -Wpedantic @($target.Sources) -I src -I lib/fips202 -ladvapi32 -o "bin/$($target.Name).exe"
        if ($LASTEXITCODE -ne 0) { throw "Build failed: $($target.Name)" }
    }
    if ($Test) {
        foreach ($name in @('prng', 'test-u512', 'test-private-key', 'test-private-key-extended', 'test-prng', 'test-prime', 'test-dh-exchange', 'test-regressions')) {
            & "./bin/$name.exe"
            if ($LASTEXITCODE -ne 0) { throw "Test failed: $name" }
        }
    }
    if ($Test) {
        & ./bin/dh.exe --test
        if ($LASTEXITCODE -ne 0) { throw "DH smoke test failed." }
    }
    if ($TestSafe512) {
        & ./bin/test-prime.exe --safe512
        if ($LASTEXITCODE -ne 0) { throw 'Safe prime test failed.' }
    }
} finally { Pop-Location }
